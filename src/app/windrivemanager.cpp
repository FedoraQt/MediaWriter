/*
 * Fedora Media Writer
 * Copyright (C) 2022 Jan Grulich <jgrulich@redhat.com>
 * Copyright (C) 2011-2022 Pete Batard <pete@akeo.ie>
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

#include "windrivemanager.h"
#include "notifications.h"

#include <QDebug>
#include <QStringList>
#include <QTimer>

#include <windows.h>
#define INITGUID
#include <guiddef.h>

#include <cmath>
#include <cstring>

const int maxPartitionCount = 16;

DEFINE_GUID(PARTITION_MICROSOFT_DATA, 0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, 0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7);

WinDriveProvider::WinDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    mDebug() << this->metaObject()->className() << "construction";
    QTimer::singleShot(0, this, &WinDriveProvider::checkDrives);
}

void WinDriveProvider::checkDrives()
{
    static bool firstRun = true;

    if (firstRun)
        mDebug() << this->metaObject()->className() << "Looking for the drives for the first time";

    for (int i = 0; i < 64; i++) {
        bool present = describeDrive(i, firstRun);
        if (!present && m_drives.contains(i)) {
            emit driveRemoved(m_drives[i]);
            m_drives[i]->deleteLater();
            m_drives.remove(i);
        }
    }

    if (firstRun)
        mDebug() << this->metaObject()->className() << "Finished looking for the drives for the first time";
    firstRun = false;
    QTimer::singleShot(2500, this, &WinDriveProvider::checkDrives);
}

QString getPhysicalName(int driveNumber)
{
    return QString("\\\\.\\PhysicalDrive%0").arg(driveNumber);
}

HANDLE getPhysicalHandle(int driveNumber)
{
    HANDLE physicalHandle = INVALID_HANDLE_VALUE;
    QString physicalPath = getPhysicalName(driveNumber);
    physicalHandle = CreateFileA(physicalPath.toStdString().c_str(), GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    return physicalHandle;
}

bool WinDriveProvider::isMountable(int driveNumber)
{
    mDebug() << this->metaObject()->className() << "Checking whether " << getPhysicalName(driveNumber) << " is mountable";

    HANDLE physicalHandle = getPhysicalHandle(driveNumber);
    if (physicalHandle == INVALID_HANDLE_VALUE) {
        mDebug() << this->metaObject()->className() << "Could not get physical handle for drive " << getPhysicalName(driveNumber);
        return false;
    }

    DWORD size;
    BYTE geometry[256];
    bool ret = DeviceIoControl(physicalHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, geometry, sizeof(geometry), &size, NULL);
    if (!ret || size <= 0) {
        mDebug() << this->metaObject()->className() << "Could not get geometry for drive " << getPhysicalName(driveNumber);
        CloseHandle(physicalHandle);
        return false;
    }

    PDISK_GEOMETRY_EX diskGeometry = (PDISK_GEOMETRY_EX)(void *)geometry;
    // Drive info
    LONGLONG diskSize;
    DWORD sectorSize;
    DWORD sectorsPerTrack;
    DWORD firstDataSector;
    MEDIA_TYPE mediaType;

    diskSize = diskGeometry->DiskSize.QuadPart;
    sectorSize = diskGeometry->Geometry.BytesPerSector;
    firstDataSector = MAXDWORD;
    if (sectorSize < 512) {
        mDebug() << this->metaObject()->className() << "Warning: Drive " << getPhysicalName(driveNumber) << " reports a sector size of " << sectorSize << " - Correcting to 512 bytes.";
        sectorSize = 512;
    }
    sectorsPerTrack = diskGeometry->Geometry.SectorsPerTrack;
    mediaType = diskGeometry->Geometry.MediaType;

    BYTE layout[4096] = {0};
    ret = DeviceIoControl(physicalHandle, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, layout, sizeof(layout), &size, NULL);
    if (!ret || size <= 0) {
        mDebug() << this->metaObject()->className() << "Could not get layout for drive " << getPhysicalName(driveNumber);
        CloseHandle(physicalHandle);
        return false;
    }

    PDRIVE_LAYOUT_INFORMATION_EX driveLayout = (PDRIVE_LAYOUT_INFORMATION_EX)(void *)layout;

    switch (driveLayout->PartitionStyle) {
    case PARTITION_STYLE_MBR:
        mDebug() << this->metaObject()->className() << "MBR partition style";
        for (int i = 0; i < driveLayout->PartitionCount; i++) {
            if (driveLayout->PartitionEntry[i].Mbr.PartitionType != PARTITION_ENTRY_UNUSED) {
                QVector<uint8_t> mbrMountable = {0x01, 0x04, 0x06, 0x07, 0x0b, 0x0c, 0x0e};
                BYTE partType = driveLayout->PartitionEntry[i].Mbr.PartitionType;
                mDebug() << this->metaObject()->className() << "Partition type: " << partType;
                if (!mbrMountable.contains(partType)) {
                    CloseHandle(physicalHandle);
                    mDebug() << this->metaObject()->className() << getPhysicalName(driveNumber) << " is not mountable";
                    return false;
                }
            }
        }
        break;
    case PARTITION_STYLE_GPT:
        mDebug() << this->metaObject()->className() << "GPT partition style";
        for (int i = 0; i < driveLayout->PartitionCount; i++) {
            if (memcmp(&driveLayout->PartitionEntry[i].Gpt.PartitionType, &PARTITION_MICROSOFT_DATA, sizeof(GUID)) != 0) {
                CloseHandle(physicalHandle);
                mDebug() << this->metaObject()->className() << getPhysicalName(driveNumber) << " is not mountable";
                return false;
            }
        }
        break;
    default:
        mDebug() << this->metaObject()->className() << "Partition type: RAW";
        break;
    }

    mDebug() << this->metaObject()->className() << getPhysicalName(driveNumber) << " is mountable";

    CloseHandle(physicalHandle);
    return true;
}

bool WinDriveProvider::describeDrive(int nDriveNumber, bool verbose)
{
    BOOL removable;
    QString productVendor;
    QString productId;
    QString serialNumber;
    uint64_t deviceBytes;
    STORAGE_BUS_TYPE storageBus;

    BOOL bResult = FALSE; // results flag
    // DWORD dwRet = NO_ERROR;

    // Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
    QString strDrivePath = getPhysicalName(nDriveNumber);

    // Get a handle to physical drive
    HANDLE hDevice = ::CreateFile(strDrivePath.toStdWString().c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE)
        return false; //::GetLastError();

    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "is present";

    // Set the input data structure
    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
    storagePropertyQuery.PropertyId = StorageDeviceProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;

    // Get the necessary output buffer size
    STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
    ZeroMemory(&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER));
    DWORD dwBytesReturned = 0;
    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "IOCTL_STORAGE_QUERY_PROPERTY";
    if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY), &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER), &dwBytesReturned, NULL)) {
        // dwRet = ::GetLastError();
        ::CloseHandle(hDevice);
        return false; // dwRet;
    }

    // Alloc the output buffer
    const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
    BYTE *pOutBuffer = new BYTE[dwOutBufferSize];
    ZeroMemory(pOutBuffer, dwOutBufferSize);

    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "IOCTL_STORAGE_QUERY_PROPERTY with a bigger buffer";
    // Get the storage device descriptor
    if (!(bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY), pOutBuffer, dwOutBufferSize, &dwBytesReturned, NULL))) {
        // dwRet = ::GetLastError();
        delete[] pOutBuffer;
        ::CloseHandle(hDevice);
        return false; // dwRet;
    }

    // Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
    // followed by additional info like vendor ID, product ID, serial number, and so on.
    STORAGE_DEVICE_DESCRIPTOR *pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR *)pOutBuffer;
    removable = pDeviceDescriptor->RemovableMedia;
    if (pDeviceDescriptor->ProductIdOffset != 0)
        productId = QString((char *)pOutBuffer + pDeviceDescriptor->ProductIdOffset).trimmed();
    if (pDeviceDescriptor->VendorIdOffset != 0)
        productVendor = QString((char *)pOutBuffer + pDeviceDescriptor->VendorIdOffset).trimmed();
    if (pDeviceDescriptor->SerialNumberOffset != 0)
        serialNumber = QString((char *)pOutBuffer + pDeviceDescriptor->SerialNumberOffset).trimmed();
    storageBus = pDeviceDescriptor->BusType;

    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "detected:" << productVendor << productId << (removable ? ", removable" : ", nonremovable") << (storageBus == BusTypeUsb ? "USB" : "notUSB");

    if (!removable && storageBus != BusTypeUsb)
        return false;

    DISK_GEOMETRY pdg;
    DWORD junk = 0; // discard results

    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "IOCTL_DISK_GET_DRIVE_GEOMETRY";
    bResult = DeviceIoControl(hDevice, // device to be queried
                              IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
                              NULL,
                              0, // no input buffer
                              &pdg,
                              sizeof(pdg), // output buffer
                              &junk, // # bytes returned
                              (LPOVERLAPPED)NULL); // synchronous I/O

    if (!bResult || pdg.MediaType == Unknown)
        return false;

    deviceBytes = pdg.Cylinders.QuadPart * pdg.TracksPerCylinder * pdg.SectorsPerTrack * pdg.BytesPerSector;

    // Do cleanup and return
    if (verbose)
        mDebug() << this->metaObject()->className() << strDrivePath << "cleanup, adding to the list";
    delete[] pOutBuffer;
    ::CloseHandle(hDevice);

    WinDrive *currentDrive = new WinDrive(this, productVendor + " " + productId, deviceBytes, !isMountable(nDriveNumber), nDriveNumber, serialNumber);
    if (m_drives.contains(nDriveNumber) && *m_drives[nDriveNumber] == *currentDrive) {
        currentDrive->deleteLater();
        return true;
    }

    if (m_drives.contains(nDriveNumber)) {
        emit driveRemoved(m_drives[nDriveNumber]);
        m_drives[nDriveNumber]->deleteLater();
    }

    m_drives[nDriveNumber] = currentDrive;
    emit driveConnected(currentDrive);

    return true;
}

WinDrive::WinDrive(WinDriveProvider *parent, const QString &name, uint64_t size, bool containsLive, int device, const QString &serialNumber)
    : Drive(parent, name, size, containsLive)
    , m_device(device)
    , m_serialNo(serialNumber)
{
}

WinDrive::~WinDrive()
{
    if (m_child)
        m_child->kill();
}

bool WinDrive::write(ReleaseVariant *data)
{
    mDebug() << this->metaObject()->className() << "Preparing to write" << data->fullName() << "to drive" << m_device;
    if (!Drive::write(data))
        return false;

    if (m_child) {
        // TODO some handling of an already present process
        m_child->deleteLater();
    }
    m_child = new QProcess(this);
    connect(m_child, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &WinDrive::onFinished);
    connect(m_child, &QProcess::readyRead, this, &WinDrive::onReadyRead);
    connect(qApp, &QCoreApplication::aboutToQuit, m_child, &QProcess::terminate);

    if (data->status() != ReleaseVariant::DOWNLOADING)
        m_image->setStatus(ReleaseVariant::WRITING);

    if (QFile::exists(qApp->applicationDirPath() + "/helper.exe")) {
        m_child->setProgram(qApp->applicationDirPath() + "/helper.exe");
    } else if (QFile::exists(qApp->applicationDirPath() + "/../helper.exe")) {
        m_child->setProgram(qApp->applicationDirPath() + "/../helper.exe");
    } else {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        setDelayedWrite(false);
        return false;
    }

    QStringList args;
    args << "write";
    if (data->status() == ReleaseVariant::WRITING)
        args << data->iso();
    else
        args << data->temporaryPath();
    args << QString("%1").arg(m_device);
    m_child->setArguments(args);

    mDebug() << this->metaObject()->className() << "Starting" << m_child->program() << args;
    m_child->start();
    return true;
}

void WinDrive::cancel()
{
    Drive::cancel();
    if (m_child) {
        m_child->kill();
        m_child->deleteLater();
        m_child = nullptr;
    }
}

void WinDrive::restore()
{
    mDebug() << this->metaObject()->className() << "Preparing to restore disk" << m_device;
    if (m_child)
        m_child->deleteLater();

    m_child = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    if (QFile::exists(qApp->applicationDirPath() + "/helper.exe")) {
        m_child->setProgram(qApp->applicationDirPath() + "/helper.exe");
    } else if (QFile::exists(qApp->applicationDirPath() + "/../helper.exe")) {
        m_child->setProgram(qApp->applicationDirPath() + "/../helper.exe");
    } else {
        m_restoreStatus = RESTORE_ERROR;
        return;
    }

    QStringList args;
    args << "restore";
    args << QString("%1").arg(m_device);
    m_child->setArguments(args);

    // connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_child, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int, QProcess::ExitStatus)));
    connect(qApp, &QCoreApplication::aboutToQuit, m_child, &QProcess::terminate);

    mDebug() << this->metaObject()->className() << "Starting" << m_child->program() << args;

    m_child->start(QIODevice::ReadOnly);
}

QString WinDrive::serialNumber() const
{
    return m_serialNo;
}

bool WinDrive::operator==(const WinDrive &o) const
{
    return (o.serialNumber() == serialNumber()) && Drive::operator==(o);
}

void WinDrive::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    setDelayedWrite(false);

    if (!m_child)
        return;

    mDebug() << "Child finished" << exitCode << exitStatus;
    mDebug() << m_child->errorString();

    if (exitCode == 0) {
        m_image->setStatus(ReleaseVariant::FINISHED);
        Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
    } else {
        m_image->setErrorString(m_child->readAllStandardError().trimmed());
        m_image->setStatus(ReleaseVariant::FAILED);
    }

    m_child->deleteLater();
    m_child = nullptr;
}

void WinDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_child)
        return;

    mCritical() << "Process finished" << exitCode << exitStatus;
    mCritical() << m_child->readAllStandardError();

    if (exitCode == 0)
        m_restoreStatus = RESTORED;
    else
        m_restoreStatus = RESTORE_ERROR;
    emit restoreStatusChanged();

    m_child->deleteLater();
    m_child = nullptr;
}

void WinDrive::onReadyRead()
{
    if (!m_child)
        return;

    m_progress->setTo(m_image->size());
    m_progress->setValue(NAN);

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING)
        m_image->setStatus(ReleaseVariant::WRITING);

    while (m_child->bytesAvailable() > 0) {
        QString line = m_child->readLine().trimmed();
        if (line == "CHECK") {
            mDebug() << this->metaObject()->className() << "Written media check starting";
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITE_VERIFYING);
        } else if (line == "WRITE") {
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITING);
        } else if (line == "DONE") {
            m_progress->setValue(m_image->size());
            m_image->setStatus(ReleaseVariant::FINISHED);
            Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
        } else {
            bool ok;
            qreal bytes = line.toLongLong(&ok);
            if (ok) {
                if (bytes < 0)
                    m_progress->setValue(NAN);
                else
                    m_progress->setValue(bytes);
            }
        }
    }
}
