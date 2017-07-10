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

Drive::Drive(const QString &identifier) : QObject(nullptr), m_handle(nullptr) {
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
    DWORD bytesWritten;
    OVERLAPPED overlap{};

    if (!WriteFile(m_handle, buffer, size, &bytesWritten, &overlap)) {
        if (GetLastError() == ERROR_IO_PENDING) {
            WaitForSingleObject(overlap.hEvent, INFINITE);
        }
        else {
            throwError("Destination drive is not writable.");
        }
    }

    if (bytesWritten != size) {
        throw std::runtime_error("Destination drive is not writable.");
    }
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() const {
    return _open_osfhandle(reinterpret_cast<intptr_t>(m_handle), 0);
}

void Drive::wipe() {
    const QString message = "Couldn't wipe partition.";
    CREATE_DISK disk;
    disk.PartitionStyle = PARTITION_STYLE_MBR;
    deviceIoControl(message, &disk);
    if (!deviceIoControlCode(IOCTL_DISK_UPDATE_PROPERTIES)) {
        throwError(message);
    }
}

/**
 * Fill the rest of the drive with a primary partition that uses the fat
 * filesystem.
 */
QPair<QString, qint64> Drive::addPartition(quint64 offset, const QString &label) {
    const QString message = "Couldn't add partition.";
    LayoutInfo layout = deviceIoControl<decltype(layout)>(message);
    std::size_t partition_idx = 0;
    for (; partition_idx < layout.PartitionCount; ++partition_idx) {
        if (layout.PartitionEntry[partition_idx].PartitionNumber == 0) {
            break;
        }
    }
    if (partition_idx == layout.PartitionCount) {
        ++layout.PartitionCount;
    }
    const std::size_t partition_num = partition_idx + 1;
    if (layout.PartitionStyle != PARTITION_STYLE_MBR || layout.PartitionCount > MAX_PARTITIONS) {
        throw std::runtime_error(message.toStdString());
    }
    PARTITION_INFORMATION_EX &partition = layout.PartitionEntry[partition_idx];
    partition.PartitionStyle = PARTITION_STYLE_MBR;
    partition.StartingOffset.QuadPart = offset;
    partition.PartitionLength.QuadPart = m_geometry.DiskSize.QuadPart - offset;
    partition.PartitionNumber = partition_num;
    partition.RewritePartition = TRUE;
    partition.Mbr.PartitionType = PARTITION_FAT32;
    partition.Mbr.BootIndicator = FALSE;
    partition.Mbr.RecognizedPartition = FALSE;
    partition.Mbr.HiddenSectors = offset / m_geometry.Geometry.BytesPerSector;

    PARTITION_INFORMATION_EX empty{};
    empty.RewritePartition = TRUE;
    for (std::size_t i = partition_num; i < layout.PartitionCount; ++i) {
        layout.PartitionEntry[i] = empty;
    }

    deviceIoControl(message, &layout);
    if (!deviceIoControlCode(IOCTL_DISK_UPDATE_PROPERTIES)) {
        throwError(message);
    }
    /*
     * Same as IOCTL_DISK_VERIFY with the difference that I know how to use it.
     * This is important because sometimes there's simply no change.
     */
    layout = deviceIoControl<decltype(layout)>(message);
    const PARTITION_INFORMATION_EX &target = layout.PartitionEntry[partition_idx];
    if (target.PartitionNumber != partition_num ||
            target.StartingOffset.QuadPart != partition.StartingOffset.QuadPart ||
            target.PartitionLength.QuadPart != partition.PartitionLength.QuadPart) {
        throw std::runtime_error(message.toStdString());
    }
    /*
     * Shared pointer owned by parent chain. Don't die. You know that already
     * since this is Qt.
     */
    QProcess *process = new QProcess(this);
    /**
     * Would love to use the Virtual Disk Service via COM but MinGW doesn't
     * have the ability to do so out of the box and to do so requires linking
     * against a static library which can't be included because of licensing
     * issues and guidelines.
     *
     * Note that running diskpart is still slow but since it's only used for
     * formating there does not seem to be a need to wait 15 seconds also it
     * doesn't seem to get in the way of locking.
     */
    QProcess &diskpart = *process;
    /* QProcess diskpart; */
    diskpart.setProgram("diskpart.exe");
    diskpart.setProcessChannelMode(QProcess::ForwardedChannels);
    diskpart.start(QIODevice::ReadWrite);
    diskpart.write(qPrintable(QString("select disk %0\r\n").arg(m_driveNumber)));
    diskpart.write(qPrintable(QString("select partition %0\r\n").arg(partition.PartitionNumber)));
    diskpart.write(qPrintable(QString("format fs=fat32 label=\"%0\" quick\r\n").arg(label)));
    diskpart.write("exit\r\n");
    diskpart.waitForFinished();
    if (diskpart.exitCode() != 0)
        throw std::runtime_error(message.toStdString());

    return qMakePair(guidOfPartition(partition.PartitionNumber), partition.PartitionLength.QuadPart);
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

QString Drive::guidOfPartition(int partitionNumber) const {
    const QString message = "Couldn't get volume GUID path of partition.";
    /*
     * A volume GUID path looks like this:
     * \\?\Volume{00000000-0000-0000-0000-000000000000}\
     */
    constexpr std::size_t GUID_SIZE = 50;
    WCHAR guid[GUID_SIZE] = L"";
    HANDLE volume = FindFirstVolume(guid, GUID_SIZE);
    if (volume == INVALID_HANDLE_VALUE)
        throwError(message);
    do {
        QString identifier = QString::fromStdWString(guid);
        identifier.chop(1);
        HANDLE handle = openBlockDevice(identifier);
        if (handle == INVALID_HANDLE_VALUE) {
            CloseHandle(handle);
            continue;
        }

        STORAGE_DEVICE_NUMBER device;
        try {
            device = deviceIoControl<decltype(device)>(handle);
        } catch (std::runtime_error &) {
            CloseHandle(handle);
            continue;
        }
        CloseHandle(handle);
        if (device.DeviceType == FILE_DEVICE_DISK && device.DeviceNumber == m_driveNumber && device.PartitionNumber == static_cast<decltype(device.PartitionNumber)>(partitionNumber)) {
            FindVolumeClose(volume);
            return identifier + "\\";
        }
    } while (FindNextVolume(volume, guid, GUID_SIZE));
    FindVolumeClose(volume);
    throw std::runtime_error(message.toStdString());
}

/**
 * Mount specified partition.
 */
QString Drive::mount(const QString &partitionIdentifier) {
    QDir dir = QDir::temp();
    dir.mkdir("fmwmountpoint");
    QString mountpoint = dir.absolutePath() + "/fmwmountpoint/";
    if (!SetVolumeMountPointA(qPrintable(QDir::toNativeSeparators(mountpoint)),
                qPrintable(partitionIdentifier))) {
        throwError("Couldn't mount partititon.");
    }
    return mountpoint;
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
DWORD Drive::controlCodeOf<Drive::LayoutInfo>() {
    return IOCTL_DISK_GET_DRIVE_LAYOUT_EX;
}
template <>
DWORD Drive::controlCodeOf<std::nullptr_t, Drive::LayoutInfo>() {
    return IOCTL_DISK_SET_DRIVE_LAYOUT_EX;
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
template <>
DWORD Drive::controlCodeOf<STORAGE_DEVICE_NUMBER>() {
    return IOCTL_STORAGE_GET_DEVICE_NUMBER;
}
