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

#include <QDebug>

#include <io.h>
#include <windows.h>

#include "isomd5/libcheckisomd5.h"

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), dd(new QProcess(this))
{
    bool ok = false;
    this->where = where.toInt(&ok);

    QTimer::singleShot(0, this, &WriteJob::work);
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob*)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    out << offset << "\n";
    out.flush();
    return 0;
}

HANDLE WriteJob::openDrive(int physicalDriveNumber) {
    HANDLE hVol;
    QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(physicalDriveNumber);

    hVol = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);

    if( hVol == INVALID_HANDLE_VALUE ) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        err << tr("Couldn't open the drive for writing") << " (" << message << ")\n";
        err.flush();
        return hVol;
    }

    return hVol;
}

bool WriteJob::lockDrive(HANDLE drive) {
    DWORD status;
    if (!DeviceIoControl(drive, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        err << tr("Couldn't lock the drive") << " (" << message << ")\n";
        err.flush();
        return false;
    }
    return true;
}

bool WriteJob::dismountDrive(HANDLE drive, uint diskNumber) {
    DWORD status;
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
                        BOOL b = DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &status, NULL);
                        if (!b) {
                            TCHAR message[256];
                            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
                            err << message << "\n";
                            err.flush();
                        }
                    }
                }
            }
        }
    }

    if (!DeviceIoControl(drive, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
        TCHAR message[256];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
        err << tr("Couldn't dismount the drive") << " ("<< message <<")\n";
        err.flush();
        return false;
    }
    return true;
}

bool WriteJob::cleanDrive(HANDLE drive) {
    OVERLAPPED overlap;
    memset(&overlap, 0, sizeof(overlap));
    overlap.hEvent = 0;

    char buf[BLOCK_SIZE] = { 0 };

    DISK_GEOMETRY pdg;

    if (!DeviceIoControl(drive,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         NULL, 0,
                         &pdg, sizeof(pdg),
                         nullptr,
                         &overlap))
        return false;

    memset(&overlap, 0, sizeof(overlap));
    uint64_t deviceBytes = pdg.Cylinders.QuadPart * pdg.TracksPerCylinder * pdg.SectorsPerTrack * pdg.BytesPerSector;


    // erase first and last 1MB on the drive
    for (uint64_t i = 0; i < 1024 * 1024; i += BLOCK_SIZE) {
        if (overlap.Offset + BLOCK_SIZE < overlap.Offset)
            overlap.OffsetHigh++;
        overlap.Offset += BLOCK_SIZE;
        if (!writeBlock(drive, &overlap, buf, BLOCK_SIZE))
            return false;
    }
    for (uint64_t i = deviceBytes - 1024 * 1024; i < deviceBytes; i += BLOCK_SIZE) {
        if (overlap.Offset + BLOCK_SIZE < overlap.Offset)
            overlap.OffsetHigh++;
        overlap.Offset += BLOCK_SIZE;
        if (!writeBlock(drive, &overlap, buf, BLOCK_SIZE))
            return false;
    }

    return true;
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
            err << tr("Destination drive is not writable") << " (" << message << ")\n";
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
        err << tr("Couldn't unlock the drive") << " (" << message << ")\n";
        err.flush();
        return;
    }
    return;
}

void WriteJob::work() {
    out << -1 << "\n";
    out.flush();

    HANDLE drive = openDrive(where);
    if (!lockDrive(drive)) {
        qApp->exit(1);
        return;
    }
    if (!dismountDrive(drive, where)) {
        qApp->exit(1);
        return;
    }
    if (!cleanDrive(drive)) {
        qApp->exit(1);
        return;
    }

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
        return;
    }

    while (true) {
        buffer = isoFile.read(BLOCK_SIZE);
        if (!writeBlock(drive, &osWrite, buffer.data(), buffer.size())) {
            qApp->exit(1);
            return;
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

    out << "CHECK\n";
    out.flush();

    drive = openDrive(where);

    switch (mediaCheckFD(_open_osfhandle(reinterpret_cast<intptr_t>(drive), 0), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        err << "OK\n";
        err.flush();
        qApp->exit(0);
        break;
    case ISOMD5SUM_CHECK_FAILED:
        err << tr("Your drive is probably damaged.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    }

    unlockDrive(drive);

    qApp->exit(0);
}
