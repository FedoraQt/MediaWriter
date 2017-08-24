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

#include "drive.h"

#include <unistd.h>

#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <QDir>
#include <QObject>
#include <QPair>
#include <QTextStream>
#include <QThread>
#include <QtGlobal>

#include <io.h>
#include <windows.h>

#include "blockdevice.h"

Drive::Drive(const QString &identifier) : m_handle(nullptr) {
    m_driveNumber = identifier.toInt();
}

Drive::~Drive() {
    try {
        unlock();
    } catch (std::runtime_error &e) {
        QTextStream err(stderr);
        err << e.what() << '\n';
        err.flush();
    }
    ::CloseHandle(m_handle);
    m_handle = nullptr;
}

void Drive::throwError(const QString &error) {
    constexpr std::size_t MESSAGE_SIZE = 256;
    TCHAR message[MESSAGE_SIZE];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, MESSAGE_SIZE - 1, nullptr);
    auto error_message = error + " (" + QString::fromWCharArray(message).trimmed() + ")";
    throw std::runtime_error(error_message.toStdString());
}

HANDLE Drive::openBlockDevice(const QString &device) {
    return ::CreateFileA(qPrintable(device), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
}

bool Drive::deviceIoControlCode(DWORD controlCode) const {
    DWORD bytesReturned;
    return ::DeviceIoControl(m_handle, controlCode, nullptr, 0, nullptr, 0, &bytesReturned, nullptr);
}

void Drive::lock() {
    constexpr int MAX_ATTEMPTS = 10;
    for (int attempts = 0; !deviceIoControlCode(FSCTL_LOCK_VOLUME); ++attempts) {
        if (attempts == MAX_ATTEMPTS) {
            throwError("Couldn't lock the drive.");
            break;
        }
        Sleep(2000);
    }
}

void Drive::unlock() {
    if (deviceIoControlCode(FSCTL_UNLOCK_VOLUME)) {
        throwError("Couldn't unlock the drive.");
    }
}

void Drive::init() {
    if (m_handle != nullptr)
        return;
    QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(m_driveNumber);
    m_handle = openBlockDevice(drivePath);
    if (m_handle == INVALID_HANDLE_VALUE) {
        m_handle = nullptr;
        throwError("Couldn't open the drive for writing.");
    }

    lock();
    m_geometry = deviceIoControl<decltype(m_geometry)>("Couldn't get disk info.");
}

/**
 * Write buffer directly to drive.
 */
void Drive::write(const void *buffer, std::size_t size) {
    int fd = getDescriptor();
    if (static_cast<std::size_t>(::write(fd, buffer, size)) != size) {
        throw std::runtime_error("Destination drive is not writable.");
    }
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() const {
    static int fd = -1;
    if (fd == -1) {
        fd = _open_osfhandle(reinterpret_cast<intptr_t>(m_handle), 0);
    }
    return fd;
}

void Drive::wipe() {
    const QString message = "Couldn't wipe partition.";
    CREATE_DISK disk;
    disk.PartitionStyle = PARTITION_STYLE_MBR;
    deviceIoControl(message, &disk);
    if (!deviceIoControlCode(IOCTL_DISK_UPDATE_PROPERTIES)) {
        throwError(message);
    }
    QProcess diskpart;
    diskpart.setProgram("diskpart.exe");
    diskpart.setProcessChannelMode(QProcess::ForwardedChannels);
    diskpart.start(QIODevice::ReadWrite);
    diskpart.write(qPrintable(QString("select disk %0\r\n").arg(m_driveNumber)));
    diskpart.write("create partition primary\r\n");
    diskpart.write("format fs=fat32 quick\r\n");
    diskpart.write("exit\r\n");
    diskpart.waitForFinished();
    if (diskpart.exitCode() != 0)
        throw std::runtime_error(message.toStdString());
}

void Drive::addOverlayPartition(quint64 offset) {
    addOverlay(offset, m_geometry.DiskSize.QuadPart - offset);
}

/**
 * Currently unused because it doesn't work correctly.
 */
char Drive::unusedDriveLetter() {
    constexpr char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    // This does not include mounted network drives, etc.
    DWORD drives = ::GetLogicalDrives();
    /*
     * It's safe to start at D because people hard-code C in their source code
     * so Windows will never switch away from that and A and B are legacy drive
     * letters.
     */
    for (std::size_t i = 3; i < std::extent<decltype(ALPHABET)>::value; ++i) {
        if (drives & (1 << i))
            continue;
        const char driveLetter = ALPHABET[i];
        /*
         * DRIVE_NO_ROOT_DIR should be returned if the root directory is
         * invalid or if the slot is unused.
         *
         * FIXME: For some reason the expression below evaluates to true even
         * though a network drive is mounted at the specified path.
         *
         * Note: What's crazy is that mounting to it a drive letter at which a
         * network drive is already monted works without any error but both
         * drives are then mounted on the same drive letter and the network
         * drive shadows the actual disk in the explorer visually but
         * essentially the network drive cannot be used from that mountpoint
         * anymore.
         */
        if (::GetDriveTypeA(qPrintable(QString("%0:\\").arg(driveLetter))) == DRIVE_NO_ROOT_DIR) {
            return driveLetter;
        }
    }
    throw std::runtime_error("All drive letters have been used.");
}

/**
 * Check whether the drive letter belongs to this drive.
 */
bool Drive::hasDriveLetter(const char driveLetter) const {
    HANDLE handle = openBlockDevice(QString("\\\\.\\%1:").arg(driveLetter));
    try {
        VOLUME_DISK_EXTENTS vde = deviceIoControl<decltype(vde)>(handle);
        for (std::size_t i = 0; i < vde.NumberOfDiskExtents; ++i) {
            if (vde.Extents[i].DiskNumber == m_driveNumber) {
                CloseHandle(handle);
                handle = nullptr;
                return true;
            }
        }
    } catch (std::runtime_error &) {
    }
    if (handle)
        CloseHandle(handle);
    return false;
}

/**
 * Unmount all partitions of a disk from drive letters.
 * TODO: Unmount more reliably.
 */
void Drive::umount() {
    constexpr char ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    DWORD drives = ::GetLogicalDrives();

    for (std::size_t i = 0; i < std::extent<decltype(ALPHABET)>::value; ++i) {
        const bool hasDrive = drives & (1 << i);
        if (!hasDrive)
            continue;
        const char driveLetter = ALPHABET[i];
        if (hasDriveLetter(driveLetter)) {
            QString volumePath = QString("%1:\\").arg(driveLetter);
            if (!DeleteVolumeMountPointA(qPrintable(volumePath))) {
                throwError(QString("Couldn't remove the drive %1:").arg(driveLetter));
            }
        }
    }
}

template <>
DWORD Drive::controlCodeOf<std::nullptr_t, CREATE_DISK>() {
    return IOCTL_DISK_CREATE_DISK;
}
template <>
DWORD Drive::controlCodeOf<VOLUME_DISK_EXTENTS>() {
    return IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS;
}
template <>
DWORD Drive::controlCodeOf<DISK_GEOMETRY_EX>() {
    return IOCTL_DISK_GET_DRIVE_GEOMETRY_EX;
}
