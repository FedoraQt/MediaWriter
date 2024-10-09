/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "writejob.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

#include <io.h>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"

static constexpr qint64 BLOCK_SIZE{512 * 128};

WriteJob::WriteJob(const QString &image, const QString &driveNumber, QObject *parent)
    : QObject(parent)
    , m_image(image)
{
    const int wmiDriveNumber = driveNumber.toInt();

    m_wmi = std::make_unique<LibWMI>(this);
    m_wmiDiskDrive = m_wmi->getDiskDriveInformation(wmiDriveNumber);

    if (m_image.endsWith(".part")) {
        connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
        m_watcher.addPath(m_image);
    } else {
        QTimer::singleShot(0, this, &WriteJob::work);
    }
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total)
{
    return ((WriteJob *)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total)
{
    Q_UNUSED(total);
    m_out << offset << "\n";
    m_out.flush();
    return 0;
}

void WriteJob::cleanDrive(HANDLE driveHandle)
{
    QFile drive;
    drive.open(_open_osfhandle(reinterpret_cast<intptr_t>(driveHandle), 0), QIODevice::WriteOnly | QIODevice::Unbuffered, QFile::AutoCloseHandle);
    if (!drive.isOpen()) {
        m_err << tr("Failed to open the device for writing");
        m_err.flush();
        qApp->exit(1);
        return;
    }

    void *buffer = NULL;
    buffer = VirtualAlloc(NULL, BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer == NULL) {
        m_err << tr("Failed to allocate the buffer");
        m_err.flush();
        qApp->exit(1);
        return;
    }

    memset(buffer, 0, BLOCK_SIZE);
    drive.write(static_cast<char *>(buffer), BLOCK_SIZE);
    drive.close();
}

bool WriteJob::lockDrive(HANDLE driveHandle)
{
    int attempts = 0;
    DWORD status;

    while (true) {
        if (!DeviceIoControl(driveHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
            attempts++;
        } else {
            return true;
        }

        if (attempts == 10) {
            TCHAR message[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
            m_err << tr("Couldn't lock the drive") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
            m_err.flush();
            break;
        }

        QThread::sleep(2);
    }

    return false;
}

HANDLE WriteJob::openDrive()
{
    HANDLE hVol;
    hVol = CreateFile(m_wmiDiskDrive->deviceID().toStdWString().c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);

    if (hVol == INVALID_HANDLE_VALUE) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        m_err << tr("Couldn't open the drive for writing") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        m_err.flush();
        return hVol;
    }

    return hVol;
}

void WriteJob::unmountVolumes()
{
    m_out << "Unmounting volumes";
    m_out.flush();

    QStringList partitions = m_wmi->getDevicePartitions(m_wmiDiskDrive->index());
    for (const QString &partition : partitions) {
        QStringList volumes = m_wmi->getLogicalDisks(partition);
        for (const QString &volumeID : volumes) {
            const QString volumePath = QString("\\\\.\\%1").arg(volumeID);
            m_out << "Unmounting volume: " << volumePath;
            m_out.flush();

            DWORD ret;
            HANDLE volume = CreateFile(volumePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (volume == INVALID_HANDLE_VALUE) {
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Failed to unmount the drive %1:").arg(volumeID) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
                return;
            }
            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &ret, NULL)) {
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Failed to unmount the drive %1:").arg(volumeID) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
            }
            CloseHandle(volume);
        }
    }
}

void WriteJob::work()
{
    if (!write()) {
        // m_out << "0\n";
        // m_out.flush();
        // QThread::sleep(5);
        // if (!write())
        return;
    }

    if (!check())
        return;

    qApp->exit(0);
}

void WriteJob::onFileChanged(const QString &path)
{
    if (QFile::exists(path))
        return;
    QRegularExpression reg("[.]part$");
    m_image = m_image.replace(reg, "");

    m_out << "WRITE\n";
    m_out.flush();

    work();
}

bool WriteJob::write()
{
    unmountVolumes();

    HANDLE drive = openDrive();
    if (!lockDrive(drive)) {
        qApp->exit(1);
        return false;
    }
    // cleanDrive(drive);

    // drive = openDrive();
    // if (!lockDrive(drive)) {
    //     qApp->exit(1);
    //     return false;
    // }
    // if (m_image.endsWith(".xz"))
    //     return writeCompressed(drive);
    // else
    bool ret = writePlain(drive);
    if (!ret) {
        qApp->exit(1);
    }
    CloseHandle(drive);

    return ret;
}

bool WriteJob::writeCompressed(HANDLE drive)
{
    // qint64 totalRead = 0;

    // lzma_stream strm = LZMA_STREAM_INIT;
    // lzma_ret ret;

    // uint8_t *inBuffer = new uint8_t[BLOCK_SIZE];
    // uint8_t *outBuffer = new uint8_t[BLOCK_SIZE];

    // QFile file(m_image);
    // file.open(QIODevice::ReadOnly);

    // ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    // if (ret != LZMA_OK) {
    //     m_err << tr("Failed to start decompressing.");
    //     return false;
    // }

    // strm.next_in = inBuffer;
    // strm.avail_in = 0;
    // strm.next_out = outBuffer;
    // strm.avail_out = BLOCK_SIZE;

    // OVERLAPPED osWrite;
    // memset(&osWrite, 0, sizeof(osWrite));
    // osWrite.hEvent = 0;

    // while (true) {
    //     if (strm.avail_in == 0) {
    //         qint64 len = file.read((char*) inBuffer, BLOCK_SIZE);
    //         totalRead += len;

    //         strm.next_in = inBuffer;
    //         strm.avail_in = len;

    //         m_out << totalRead << "\n";
    //         m_out.flush();
    //     }

    //     ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
    //     if (ret == LZMA_STREAM_END) {
    //         if (!writeBlock(drive, &osWrite, (char *) outBuffer, BLOCK_SIZE - strm.avail_out)) {
    //             qApp->exit(1);
    //             CloseHandle(drive);
    //             return false;
    //         }

    //         if (osWrite.Offset + BLOCK_SIZE < osWrite.Offset)
    //             osWrite.OffsetHigh++;
    //         osWrite.Offset += BLOCK_SIZE;

    //         CloseHandle(drive);

    //         return true;
    //     }
    //     if (ret != LZMA_OK) {
    //         switch (ret) {
    //         case LZMA_MEM_ERROR:
    //             m_err << tr("There is not enough memory to decompress the file.");
    //             break;
    //         case LZMA_FORMAT_ERROR:
    //         case LZMA_DATA_ERROR:
    //         case LZMA_BUF_ERROR:
    //             m_err << tr("The downloaded compressed file is corrupted.");
    //             break;
    //         case LZMA_OPTIONS_ERROR:
    //             m_err << tr("Unsupported compression options.");
    //             break;
    //         default:
    //             m_err << tr("Unknown decompression error.");
    //             break;
    //         }
    //         qApp->exit(4);
    //         CloseHandle(drive);
    //         return false;
    //     }

    //     if (strm.avail_out == 0) {
    //         if (!writeBlock(drive, &osWrite, (char *) outBuffer, BLOCK_SIZE - strm.avail_out)) {
    //             qApp->exit(1);
    //             CloseHandle(drive);
    //             return false;
    //         }

    //         if (osWrite.Offset + BLOCK_SIZE < osWrite.Offset)
    //             osWrite.OffsetHigh++;
    //         osWrite.Offset += BLOCK_SIZE;

    //         strm.next_out = outBuffer;
    //         strm.avail_out = BLOCK_SIZE;
    //     }
    // }
    return false;
}

bool WriteJob::writePlain(HANDLE driveHandle)
{
    const qint64 blockSize = m_wmiDiskDrive->sectorSize() * 128;

    QFile isoFile(m_image);
    isoFile.open(QIODevice::ReadOnly);
    if (!isoFile.isOpen()) {
        m_err << tr("Source image is not readable");
        m_err.flush();
        return false;
    }
    auto isoCleanup = qScopeGuard([&isoFile] {
        isoFile.close();
    });

    QFile drive;
    drive.open(_open_osfhandle(reinterpret_cast<intptr_t>(driveHandle), 0), QIODevice::WriteOnly | QIODevice::Unbuffered, QFile::AutoCloseHandle);
    if (!drive.isOpen()) {
        m_err << tr("Failed to open the device for writing");
        m_err.flush();
        qApp->exit(1);
        return false;
    }
    auto driveCleanup = qScopeGuard([&drive] {
        drive.close();
    });

    void *buffer = NULL;
    buffer = VirtualAlloc(NULL, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (buffer == NULL) {
        m_err << tr("Failed to allocate the buffer");
        m_err.flush();
        return false;
    }
    auto bufferCleanup = qScopeGuard([buffer, blockSize] {
        VirtualFree(buffer, blockSize, MEM_DECOMMIT | MEM_RELEASE);
    });

    qint64 sectorSize = m_wmiDiskDrive->sectorSize();
    qint64 totalBytes = 0;
    qint64 readBytes;
    qint64 writtenBytes;
    // DWORD writtenBytes = 0;
    while (true) {
        if ((readBytes = isoFile.read(static_cast<char *>(buffer), blockSize)) <= 0) {
            break;
        }

        readBytes = ((readBytes + sectorSize - 1) / sectorSize) * sectorSize;
        writtenBytes = drive.write(static_cast<char *>(buffer), readBytes);
        // BOOL result = ::WriteFile(driveHandle, buffer, readBytes, &writtenBytes, nullptr);
        if (writtenBytes < 0) {
            // if (!result) {
            m_err << tr("Failed to write to the device: ") << drive.errorString();
            // TCHAR message[256];
            // FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
            // m_err << tr("Failed to write to the device:") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
            m_err.flush();
            return false;
        }

        if (writtenBytes != readBytes) {
            m_err << tr("The last block was not fully written");
            m_err.flush();
            return false;
        }

        totalBytes += readBytes;
        m_out << totalBytes << "\n";
        m_out.flush();

        if (readBytes != blockSize || isoFile.atEnd()) {
            break;
        }
    }

    if (readBytes < 0) {
        m_err << tr("Failed to read the image file: ") << isoFile.errorString();
        m_err.flush();
        return false;
    }

    return true;
}

bool WriteJob::check()
{
    // m_out << "CHECK\n";
    // m_out.flush();

    // HANDLE drive = openDrive(m_drive);

    // switch (mediaCheckFD(_open_osfhandle(reinterpret_cast<intptr_t>(drive), 0), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    // case ISOMD5SUM_CHECK_NOT_FOUND:
    // case ISOMD5SUM_CHECK_PASSED:
    //     m_out << "DONE\n";
    //     m_out.flush();
    //     m_err << "OK\n";
    //     m_err.flush();
    //     qApp->exit(0);
    //     break;
    // case ISOMD5SUM_CHECK_FAILED:
    //     m_err << tr("Your drive is probably damaged.") << "\n";
    //     m_err.flush();
    //     qApp->exit(1);
    //     return false;
    // default:
    //     m_err << tr("Unexpected error occurred during media check.") << "\n";
    //     m_err.flush();
    //     qApp->exit(1);
    //     return false;
    // }

    return true;
}
