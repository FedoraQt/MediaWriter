/*
 * Fedora Media Writer
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
#include <QTimer>
#include <QTextStream>
#include <QProcess>
#include <QFile>
#include <QThread>

#include <QDebug>

#include <io.h>
#include <windows.h>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"


WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what)
{
    bool ok = false;
    this->where = where.toInt(&ok);

    if (what.endsWith(".part")) {
        connect(&watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
        watcher.addPath(what);
    }
    else {
        QTimer::singleShot(0, this, &WriteJob::work);
    }
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob*)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    Q_UNUSED(total);
    out << offset << "\n";
    out.flush();
    return 0;
}

HANDLE WriteJob::openDrive(int physicalDriveNumber) {
    HANDLE hVol;
    QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(physicalDriveNumber);

    hVol = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

    if( hVol == INVALID_HANDLE_VALUE ) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        err << tr("Couldn't open the drive for writing") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        err.flush();
        return hVol;
    }

    return hVol;
}

bool WriteJob::lockDrive(HANDLE drive) {
    int attempts = 0;
    DWORD status;

    while (true) {
        if (!DeviceIoControl(drive, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
            attempts++;
        }
        else {
            return true;
        }

        if (attempts == 10) {
            TCHAR message[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);

            err << tr("Couldn't lock the drive") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
            err.flush();
            break;
        }

        QThread::sleep(2);
    }

    return false;
}

bool WriteJob::removeMountPoints(uint diskNumber) {
    DWORD drives = ::GetLogicalDrives();

    for (char i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            char currentDrive = 'A' + i;
            QString drivePath = QString("\\\\.\\%1:").arg(currentDrive);

            HANDLE hDevice = ::CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

            DWORD bytesReturned;
            VOLUME_DISK_EXTENTS vde; // TODO FIXME: handle ERROR_MORE_DATA (this is an extending structure)
            BOOL bResult = DeviceIoControl(hDevice, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &vde, sizeof(vde), &bytesReturned, NULL);

            if (bResult) {
                for (uint j = 0; j < vde.NumberOfDiskExtents; j++) {
                    if (vde.Extents[j].DiskNumber == diskNumber) {
                        QString volumePath = QString("%1:\\").arg(currentDrive);

                        CloseHandle(hDevice);
                        hDevice = nullptr;

                        if (!DeleteVolumeMountPointA(volumePath.toStdString().c_str())) {
                            TCHAR message[256];
                            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                            err << tr("Couldn't remove the drive %1:").arg(currentDrive) << " (" << QString::fromWCharArray(message).trimmed() << "\n";
                            err.flush();
                            return false;
                        }

                        break;
                    }
                }
            }
            if (hDevice)
                CloseHandle(hDevice);
        }
    }

    return true;
}

bool WriteJob::cleanDrive(uint driveNumber) {
    QProcess diskpart;
    diskpart.setProgram("diskpart.exe");
    diskpart.setProcessChannelMode(QProcess::ForwardedChannels);

    diskpart.start(QIODevice::ReadWrite);

    diskpart.write(qPrintable(QString("select disk %0\r\n").arg(driveNumber)));
    diskpart.write("clean\r\n");
    // for some reason this works (tm)
    diskpart.write("create part pri\r\n");
    diskpart.write("clean\r\n");
    diskpart.write("exit\r\n");

    diskpart.waitForFinished();

    if (diskpart.exitCode() == 0) {
        // as advised in the diskpart documentation
        QThread::sleep(15);

        return true;
    }

    return false;
}

bool WriteJob::writeBlock(HANDLE drive, OVERLAPPED *overlap, char *data, uint size) {
    DWORD bytesWritten;

    if (!WriteFile(drive, data, size, &bytesWritten, overlap)) {
        DWORD Errorcode = GetLastError();
        if (Errorcode == ERROR_IO_PENDING) {
            WaitForSingleObject(overlap->hEvent, INFINITE);
        }
        else {
            TCHAR message[256];
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
            err << tr("Destination drive is not writable") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
            err.flush();
            return false;
        }
    }

    if (bytesWritten != size) {
        err << tr("Destination drive is not writable") << "\n";
        err.flush();
        return false;
    }

    return true;
}


void WriteJob::unlockDrive(HANDLE drive) {
    DWORD status;
    if (!DeviceIoControl(drive, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        err << tr("Couldn't unlock the drive") << " (" << QString::fromWCharArray(message).trimmed() << ")\n";
        err.flush();
    }
}

void WriteJob::work() {
    if (!write()) {
        out << "0\n";
        out.flush();
        QThread::sleep(5);
        if (!write())
            return;
    }

    if (!check())
        return;

    qApp->exit(0);
}

void WriteJob::onFileChanged(const QString &path) {
    if (QFile::exists(path))
        return;

    what = what.replace(QRegExp("[.]part$"), "");

    out << "WRITE\n";
    out.flush();

    work();
}

bool WriteJob::write() {
    removeMountPoints(where);
    cleanDrive(where);

    HANDLE drive = openDrive(where);
    if (!lockDrive(drive)) {
        qApp->exit(1);
        return false;
    }

    if (what.endsWith(".xz"))
        return writeCompressed(drive);
    else
        return writePlain(drive);
}

bool WriteJob::writeCompressed(HANDLE drive) {
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    uint8_t *inBuffer = new uint8_t[BLOCK_SIZE];
    uint8_t *outBuffer = new uint8_t[BLOCK_SIZE];

    QFile file(what);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = inBuffer;
    strm.avail_in = 0;
    strm.next_out = outBuffer;
    strm.avail_out = BLOCK_SIZE;

    OVERLAPPED osWrite;
    memset(&osWrite, 0, sizeof(osWrite));
    osWrite.hEvent = 0;

    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = file.read((char*) inBuffer, BLOCK_SIZE);
            totalRead += len;

            strm.next_in = inBuffer;
            strm.avail_in = len;

            out << totalRead << "\n";
            out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            if (!writeBlock(drive, &osWrite, (char *) outBuffer, BLOCK_SIZE - strm.avail_out)) {
                qApp->exit(1);
                CloseHandle(drive);
                return false;
            }

            if (osWrite.Offset + BLOCK_SIZE < osWrite.Offset)
                osWrite.OffsetHigh++;
            osWrite.Offset += BLOCK_SIZE;

            CloseHandle(drive);

            return true;
        }
        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                err << tr("There is not enough memory to decompress the file.");
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                err << tr("The downloaded compressed file is corrupted.");
                break;
            case LZMA_OPTIONS_ERROR:
                err << tr("Unsupported compression options.");
                break;
            default:
                err << tr("Unknown decompression error.");
                break;
            }
            qApp->exit(4);
            CloseHandle(drive);
            return false;
        }

        if (strm.avail_out == 0) {
            if (!writeBlock(drive, &osWrite, (char *) outBuffer, BLOCK_SIZE - strm.avail_out)) {
                qApp->exit(1);
                CloseHandle(drive);
                return false;
            }

            if (osWrite.Offset + BLOCK_SIZE < osWrite.Offset)
                osWrite.OffsetHigh++;
            osWrite.Offset += BLOCK_SIZE;

            strm.next_out = outBuffer;
            strm.avail_out = BLOCK_SIZE;
        }
    }
}

bool WriteJob::writePlain(HANDLE drive) {
    OVERLAPPED osWrite;
    memset(&osWrite, 0, sizeof(osWrite));
    osWrite.hEvent = 0;

    uint64_t cnt = 0;
    QByteArray buffer;
    QFile isoFile(what);
    isoFile.open(QIODevice::ReadOnly);
    if (!isoFile.isOpen()) {
        err << tr("Source image is not readable");
        err.flush();
        qApp->exit(1);
        return false;
    }

    while (true) {
        buffer = isoFile.read(BLOCK_SIZE);
        if (!writeBlock(drive, &osWrite, buffer.data(), buffer.size())) {
            qApp->exit(1);
            return false;
        }

        if (osWrite.Offset + BLOCK_SIZE < osWrite.Offset)
            osWrite.OffsetHigh++;
        osWrite.Offset += BLOCK_SIZE;
        cnt += buffer.size();
        out << cnt << "\n";
        out.flush();

        if (buffer.size() != BLOCK_SIZE || isoFile.atEnd())
            break;
    }

    CloseHandle(drive);

    return true;
}

bool WriteJob::check() {
    out << "CHECK\n";
    out.flush();

    HANDLE drive = openDrive(where);

    switch (mediaCheckFD(_open_osfhandle(reinterpret_cast<intptr_t>(drive), 0), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "DONE\n";
        out.flush();
        err << "OK\n";
        err.flush();
        qApp->exit(0);
        break;
    case ISOMD5SUM_CHECK_FAILED:
        err << tr("Your drive is probably damaged.") << "\n";
        err.flush();
        qApp->exit(1);
        return false;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        return false;
    }

    return true;
}
