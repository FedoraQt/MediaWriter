#include "windrivemanager.h"

#include <QTimer>
#include <QDebug>

#include <windows.h>

WinDriveProvider::WinDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    QTimer::singleShot(0, this, &WinDriveProvider::checkDrives);
}

void WinDriveProvider::checkDrives() {
    for (int i = 0; i < 64; i++) {
        WinDrive *drive = describeDrive(i);
        if (drive) {
            emit driveConnected(drive); // no disconnecting is implemented yet
        }
    }
}

WinDrive *WinDriveProvider::describeDrive(int nDriveNumber) {
    BOOL removable;
    QString productVendor;
    QString productId;
    uint64_t deviceBytes;


    BOOL bResult   = FALSE;                 // results flag
    //DWORD dwRet = NO_ERROR;

    // Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
    QString strDrivePath = QString("\\\\.\\PhysicalDrive%0").arg(nDriveNumber);

    // Get a handle to physical drive
    HANDLE hDevice = ::CreateFile(strDrivePath.toStdWString().c_str(), 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, 0, NULL);

    if(INVALID_HANDLE_VALUE == hDevice)
        return nullptr; //::GetLastError();

    // Set the input data structure
    STORAGE_PROPERTY_QUERY storagePropertyQuery;
    ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
    storagePropertyQuery.PropertyId = StorageDeviceProperty;
    storagePropertyQuery.QueryType = PropertyStandardQuery;

    // Get the necessary output buffer size
    STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader;
    ZeroMemory(&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER));
    DWORD dwBytesReturned = 0;
    if(! ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
        &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
        &storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
        &dwBytesReturned, NULL))
    {
        //dwRet = ::GetLastError();
        ::CloseHandle(hDevice);
        return nullptr; // dwRet;
    }

    // Alloc the output buffer
    const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
    BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
    ZeroMemory(pOutBuffer, dwOutBufferSize);

    // Get the storage device descriptor
    if(!(bResult = ::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
            &storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
            pOutBuffer, dwOutBufferSize,
            &dwBytesReturned, NULL)))
    {
        //dwRet = ::GetLastError();
        delete []pOutBuffer;
        ::CloseHandle(hDevice);
        return nullptr; // dwRet;
    }

    // Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
    // followed by additional info like vendor ID, product ID, serial number, and so on.
    STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
    removable = pDeviceDescriptor->RemovableMedia;
    if (pDeviceDescriptor->ProductIdOffset != 0)
        productId = QString((char*) pOutBuffer + pDeviceDescriptor->ProductIdOffset).trimmed();
    if (pDeviceDescriptor->VendorIdOffset != 0)
        productVendor = QString((char*) pOutBuffer + pDeviceDescriptor->VendorIdOffset).trimmed();

    qDebug() << "Found a drive:";
    qDebug() << "\tProduct vendor:" << productVendor;
    qDebug() << "\tProduct ID:" << productId;
    qDebug() << "\tRemovable:" << removable;

    if (!removable)
        return nullptr;

    DISK_GEOMETRY pdg;
    DWORD junk     = 0;                     // discard results

    bResult = DeviceIoControl(hDevice,                       // device to be queried
                              IOCTL_DISK_GET_DRIVE_GEOMETRY, // operation to perform
                              NULL, 0,                       // no input buffer
                              &pdg, sizeof(pdg),            // output buffer
                              &junk,                         // # bytes returned
                              (LPOVERLAPPED) NULL);          // synchronous I/O

    deviceBytes = pdg.Cylinders.QuadPart * pdg.TracksPerCylinder * pdg.SectorsPerTrack * pdg.BytesPerSector;
    qDebug() << "\tSize:" << deviceBytes;

    // Do cleanup and return
    delete []pOutBuffer;
    ::CloseHandle(hDevice);
    return new WinDrive(this, productVendor + " " + productId, deviceBytes, false); // dwret
}

WinDrive::WinDrive(WinDriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, name, size, containsLive)
{

}

void WinDrive::write(ReleaseVariant *data) {
    Drive::write(data);
}

void WinDrive::restore() {

}
