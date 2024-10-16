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

void WriteJob::work()
{
    if (!write()) {
        return;
    }

    if (!check()) {
        return;
    }

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
    HANDLE drive = CreateFile(m_wmiDiskDrive->deviceID().toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);
    if (drive == INVALID_HANDLE_VALUE) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        m_err << tr("Couldn't open the drive for partition removal for %1:").arg(m_wmiDiskDrive->deviceID()) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        m_err.flush();
        qApp->exit(1);
        return false;
    }
    auto driveloseGuard = qScopeGuard([&drive] {
        CloseHandle(drive);
    });

    QList<HANDLE> volumeHandles;
    auto volumeCloseGuard = qScopeGuard([&volumeHandles] {
        for (HANDLE volume : volumeHandles) {
            CloseHandle(volume);
        }
    });

    const QStringList partitions = m_wmi->getDevicePartitions(m_wmiDiskDrive->index());
    for (const QString &partition : partitions) {
        QStringList volumes = m_wmi->getLogicalDisks(partition);
        for (const QString &volumeID : volumes) {
            const QString volumePath = QString("\\\\.\\%1").arg(volumeID);
            m_err << "Volume path " << volumePath << "\n";
            m_out << "Unmounting: " << volumePath;
            m_out.flush();

            DWORD ret;
            HANDLE volume = CreateFile(volumePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (volume == INVALID_HANDLE_VALUE) {
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Couldn't open the volume %1:").arg(volumeID) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }
            volumeHandles << volume;

            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &ret, NULL)) {
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Couldn't unmount the volume %1:").arg(volumeID) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            if (!DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &ret, NULL)) {
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Couldn't lock the volume %1:").arg(volumeID) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
            }
        }
    }

    if (!lockDrive(drive)) {
        m_out << "Failed to lock the drive";
        m_out.flush();
        qApp->exit(1);
        return false;
    }

    // Remove partitions
    DWORD ret;
    DRIVE_LAYOUT_INFORMATION_EX *driveLayout = (DRIVE_LAYOUT_INFORMATION_EX *)malloc(sizeof(DRIVE_LAYOUT_INFORMATION_EX));
    ZeroMemory(driveLayout, sizeof(DRIVE_LAYOUT_INFORMATION_EX));
    driveLayout->PartitionCount = 0;
    driveLayout->PartitionStyle = PARTITION_STYLE_RAW;
    driveLayout->Mbr.Signature = 0;

    if (!DeviceIoControl(drive, IOCTL_DISK_SET_DRIVE_LAYOUT_EX, driveLayout, sizeof(DRIVE_LAYOUT_INFORMATION_EX), NULL, 0, &ret, NULL)) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        m_err << tr("Couldn't remove the partition table for %1:").arg(m_wmiDiskDrive->deviceID()) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        m_err.flush();
        qApp->exit(1);
        free(driveLayout);
        return false;
    }
    free(driveLayout);

    // FIXME: not sure if this is needed and it has been working
    // also without it, but previously we did that for diskpart
    // so it might maybe help in some situations and 5 seconds
    // delay is not a big deal.
    QThread::sleep(5);

    bool result;
    if (m_image.endsWith(".xz")) {
        result = writeCompressed(drive);
    } else {
        result = writePlain(drive);
    }

    if (!result) {
        qApp->exit(1);
        return false;
    }

    return true;
}

bool WriteJob::writeCompressed(HANDLE driveHandle)
{
    const qint64 blockSize = m_wmiDiskDrive->sectorSize() * 512;

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

    void *outBuffer = NULL;
    outBuffer = VirtualAlloc(NULL, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (outBuffer == NULL) {
        m_err << tr("Failed to allocate the buffer");
        m_err.flush();
        return false;
    }
    auto outBufferCleanup = qScopeGuard([outBuffer, blockSize] {
        VirtualFree(outBuffer, blockSize, MEM_DECOMMIT | MEM_RELEASE);
    });

    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    uint8_t *inBuffer = new uint8_t[blockSize];
    auto inBufferCleanup = qScopeGuard([inBuffer] {
        delete[] inBuffer;
    });

    QFile file(m_image);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        m_err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = inBuffer;
    strm.avail_in = 0;
    strm.next_out = static_cast<uint8_t *>(outBuffer);
    strm.avail_out = blockSize;

    const qint64 sectorSize = m_wmiDiskDrive->sectorSize();
    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = file.read((char *)inBuffer, blockSize);
            totalRead += len;

            strm.next_in = inBuffer;
            strm.avail_in = len;

            m_out << totalRead << "\n";
            m_out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            qint64 writtenBytes = 0;
            qint64 readBytes = blockSize - strm.avail_out;
            readBytes = ((readBytes + sectorSize - 1) / sectorSize) * sectorSize;
            writtenBytes = drive.write(static_cast<char *>(outBuffer), readBytes);
            if (writtenBytes < 0) {
                m_err << tr("Failed to write to the device: ") << drive.errorString() << "\n";
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Failed to write to the device:") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            if (writtenBytes != readBytes) {
                m_err << tr("The last block was not fully written");
                m_err.flush();
                return false;
            }

            return true;
        }

        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                m_err << tr("There is not enough memory to decompress the file.");
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                m_err << tr("The downloaded compressed file is corrupted.");
                break;
            case LZMA_OPTIONS_ERROR:
                m_err << tr("Unsupported compression options.");
                break;
            default:
                m_err << tr("Unknown decompression error.");
                break;
            }
            qApp->exit(4);
            return false;
        }

        if (strm.avail_out == 0) {
            qint64 writtenBytes = 0;
            writtenBytes = drive.write(static_cast<char *>(outBuffer), sectorSize);
            if (writtenBytes < 0) {
                m_err << tr("Failed to write to the device: ") << drive.errorString() << "\n";
                TCHAR message[256];
                FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                m_err << tr("Failed to write to the device:") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            if (writtenBytes != sectorSize) {
                m_err << tr("The last block was not fully written");
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            strm.next_out = static_cast<uint8_t *>(outBuffer);
            strm.avail_out = blockSize;
        }
    }
    return false;
}

bool WriteJob::writePlain(HANDLE driveHandle)
{
    const qint64 blockSize = m_wmiDiskDrive->sectorSize() * 512;

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
    while (true) {
        if ((readBytes = isoFile.read(static_cast<char *>(buffer), blockSize)) <= 0) {
            break;
        }

        readBytes = ((readBytes + sectorSize - 1) / sectorSize) * sectorSize;
        writtenBytes = drive.write(static_cast<char *>(buffer), readBytes);
        if (writtenBytes < 0) {
            m_err << tr("Failed to write to the device: ") << drive.errorString() << "\n";
            TCHAR message[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
            m_err << tr("Failed to write to the device:") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
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
    m_out << "CHECK\n";
    m_out.flush();

    HANDLE drive = CreateFile(m_wmiDiskDrive->deviceID().toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (drive == INVALID_HANDLE_VALUE) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        m_err << tr("Couldn't open the drive for data verification for %1:").arg(m_wmiDiskDrive->deviceID()) << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        m_err.flush();
        qApp->exit(1);
        return false;
    }
    auto driveloseGuard = qScopeGuard([&drive] {
        CloseHandle(drive);
    });

    switch (mediaCheckFD(_open_osfhandle(reinterpret_cast<intptr_t>(drive), 0), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        m_out << "DONE\n";
        m_out.flush();
        m_err << "OK\n";
        m_err.flush();
        qApp->exit(0);
        break;
    case ISOMD5SUM_CHECK_FAILED:
        m_err << tr("Your drive is probably damaged.") << "\n";
        m_err.flush();
        qApp->exit(1);
        return false;
    default:
        m_err << tr("Unexpected error occurred during media check.") << "\n";
        m_err.flush();
        qApp->exit(1);
        return false;
    }

    return true;
}
