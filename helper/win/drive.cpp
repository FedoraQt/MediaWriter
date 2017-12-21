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

Drive::Drive(const QString &identifier) : m_handle(nullptr) {
    m_driveNumber = identifier.toInt();
}

Drive::~Drive() {
    unlock();
    ::CloseHandle(m_handle);
    m_handle = nullptr;
}

void Drive::throwError(Error error, const QString &drive) {
    constexpr std::size_t MESSAGE_SIZE = 256;
    TCHAR message[MESSAGE_SIZE];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, MESSAGE_SIZE - 1, nullptr);
    auto detail = QString::fromWCharArray(message).trimmed();
    throw HelperError(error, drive, detail);
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
            throwError(Error::DRIVE_USE, drive());
            break;
        }
        Sleep(2000);
    }
}

void Drive::unlock() {
    // Error is ignored because we're done here.
    deviceIoControlCode(FSCTL_UNLOCK_VOLUME);
}

void Drive::init() {
    if (m_handle != nullptr)
        return;
    QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(m_driveNumber);
    m_handle = openBlockDevice(drivePath);
    if (m_handle == INVALID_HANDLE_VALUE) {
        m_handle = nullptr;
        throwError(Error::DRIVE_WRITE, drive());
    }

    lock();
    m_geometry = deviceIoControl<decltype(m_geometry)>();
}

/**
 * Write buffer directly to drive.
 */
void Drive::write(const void *buffer, std::size_t size) {
    int fd = getDescriptor();
    if (static_cast<std::size_t>(::write(fd, buffer, size)) != size) {
        throw HelperError(Error::DRIVE_WRITE, drive());
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

QString Drive::drive() const {
    return QString("\\\\.\\PhysicalDrive%0").arg(m_driveNumber);
}

void Drive::wipe() {
    CREATE_DISK disk;
    disk.PartitionStyle = PARTITION_STYLE_MBR;
    deviceIoControl(&disk);
    if (!deviceIoControlCode(IOCTL_DISK_UPDATE_PROPERTIES)) {
        throwError(Error::DRIVE_USE, drive());
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
        throw HelperError(Error::DRIVE_USE, drive());
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
    } catch (HelperError &) {
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
                throwError(Error::DRIVE_USE, drive());
            }
        }
    }
}

/**
 * If you'd like to use a new control code you'll look up which one you want
 * and what the input and output buffer types are.
 * Control codes: https://msdn.microsoft.com/en-us/library/windows/desktop/aa363216(v=vs.85).aspx
 * Then you add it in the following form to make use of the new control code
 * through the Drive::deviceIoControl interface:
 * ```
 * template <>
 * DWORD Drive::controlCodeOf<OUTPUT_TYPE, INPUT_TYPE>() {
 *     return CONTROL_CODE;
 * }
 * ```
 * If either input or output buffer is not specified you just use
 * std::nullptr_t in the place of them.
 * If both of them are not specified you want to use something like
 * Drive::deviceIoControlCode instead instead of adding the control code here.
 *
 * The benefit of this is that you don't have to fill in the gaps or edit
 * multiple values when calling DeviceIoControl through the deviceIoControl
 * interface.
 * Which means you don't have to specify the control code anywhere else nor do
 * you have to fill in null pointers.
 */
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
