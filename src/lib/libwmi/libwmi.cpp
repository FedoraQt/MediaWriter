/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

#include "libwmi.h"

#include <QDebug>

#include <comdef.h>
#include <dbt.h>

#pragma comment(lib, "wbemuuid.lib")

LibWMI::LibWMI(QObject *parent)
    : QObject(parent)
{
    HRESULT res = S_OK;
    // This needs to be initialized here before any RPC communication occurs
    res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "Failed to initialize security. Error = " << err.ErrorMessage();
        return;
    }

    res = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void **>(&m_IWbemLocator));
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "Failed to create IWbemLocator object. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return;
    }

    res = m_IWbemLocator->ConnectServer(_bstr_t(L"root\\cimv2"), NULL, NULL, NULL, 0, NULL, NULL, &m_IWbemServices);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "Could not connect to WMI. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return;
    }

    initialized = true;
}

LibWMI::~LibWMI()
{
    if (m_IWbemLocator) {
        m_IWbemLocator->Release();
    }
    if (m_IWbemServices) {
        m_IWbemServices->Release();
    }
    CoUninitialize();
}

QMap<quint32, QString> LibWMI::getUSBDeviceList()
{
    QMap<quint32, QString> result;
    if (!initialized) {
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pEnumDiskObjects = NULL;

    res = m_IWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_DiskDrive WHERE InterfaceType = 'USB'"), WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskObjects);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "WMI query failed. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return result;
    }

    while (true) {
        IWbemClassObject *pDiskObject = NULL;
        ULONG uReturn = 0;
        pEnumDiskObjects->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturn);
        if (uReturn == 0) {
            break;
        }

        // Fetch disk information
        quint32 index;
        QString deviceID;
        VARIANT var;

        if ((pDiskObject->Get(_bstr_t(L"Index"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_I4) {
                index = var.intVal;
                qDebug() << "Disk Index: " << index;
            } else if (var.vt == VT_UI4) {
                index = var.uintVal;
                qDebug() << "Disk Index: " << index;
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"DeviceID"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                deviceID = QString::fromWCharArray(var.bstrVal);
                qDebug() << "Device ID" << deviceID;
            }
            VariantClear(&var);
        }
        pDiskObject->Release();

        result.insert(index, deviceID);
    }
    pEnumDiskObjects->Release();

    return result;
}

QStringList LibWMI::getDevicePartitions(quint32 index)
{
    QStringList result;
    if (!initialized) {
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pPartitionObjects = NULL;
    std::wstring partitionQuery = L"SELECT * FROM Win32_DiskPartition WHERE DiskIndex = " + std::to_wstring(index);

    res = m_IWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(partitionQuery.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pPartitionObjects);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "Query for disk partitions failed. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return result;
    }

    while (true) {
        IWbemClassObject *pPartitionObject = NULL;
        ULONG uReturnPartition = 0;
        pPartitionObjects->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturnPartition);
        if (uReturnPartition == 0) {
            break;
        }

        QString deviceID;
        VARIANT partitionDeviceID;
        if ((pPartitionObject->Get(_bstr_t(L"DeviceID"), 0, &partitionDeviceID, 0, 0)) == WBEM_S_NO_ERROR) {
            if (partitionDeviceID.vt == VT_BSTR) {
                deviceID = QString::fromWCharArray(partitionDeviceID.bstrVal);
                qDebug() << "Partition device ID " << deviceID;
            }
            VariantClear(&partitionDeviceID);
        }
        pPartitionObject->Release();
        if (!deviceID.isEmpty()) {
            result << deviceID;
        }
    }
    pPartitionObjects->Release();

    return result;
}

QStringList LibWMI::getLogicalDisks(const QString &partitionID)
{
    QStringList result;
    if (!initialized) {
        return result;
    }

    HRESULT res = S_OK;
    std::wstring partitionToLogicalQuery = L"ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" + partitionID.toStdWString() + L"'} WHERE AssocClass = Win32_LogicalDiskToPartition";
    IEnumWbemClassObject *pLogicalDiskObjects = NULL;
    res = m_IWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(partitionToLogicalQuery.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pLogicalDiskObjects);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "Query for logical disks failed. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return result;
    }

    while (true) {
        IWbemClassObject *pLogicalDiskObject = NULL;
        ULONG uReturnLogicalDisk = 0;
        pLogicalDiskObjects->Next(WBEM_INFINITE, 1, &pLogicalDiskObject, &uReturnLogicalDisk);
        if (uReturnLogicalDisk == 0) {
            break;
        }

        QString deviceID;
        VARIANT logicalDiskDeviceID;
        if ((pLogicalDiskObject->Get(_bstr_t(L"DeviceID"), 0, &logicalDiskDeviceID, 0, 0)) == WBEM_S_NO_ERROR) {
            if (logicalDiskDeviceID.vt == VT_BSTR) {
                deviceID = QString::fromWCharArray(logicalDiskDeviceID.bstrVal);
                qDebug() << "Logical disk device ID " << deviceID;
            }
            VariantClear(&logicalDiskDeviceID);
        }
        pLogicalDiskObject->Release();
        if (!deviceID.isEmpty()) {
            result << deviceID;
        }
    }
    pLogicalDiskObjects->Release();

    return result;
}

std::unique_ptr<LibWMIDiskDrive> LibWMI::getDiskDriveInformation(quint32 index, const QString &deviceID)
{
    std::unique_ptr<LibWMIDiskDrive> result = std::make_unique<LibWMIDiskDrive>(index, deviceID);
    if (!initialized) {
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pEnumDiskObjects = NULL;

    std::wstring deviceQuery = L"SELECT * FROM Win32_DiskDrive WHERE Index = " + std::to_wstring(index);
    res = m_IWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(deviceQuery.c_str()), WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskObjects);
    if (FAILED(res)) {
        _com_error err(res);
        qWarning() << "WMI query failed. Error = " << QString::fromWCharArray(err.ErrorMessage());
        return result;
    }

    while (true) {
        IWbemClassObject *pDiskObject = NULL;
        ULONG uReturn = 0;
        pEnumDiskObjects->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturn);
        if (uReturn == 0) {
            break;
        }

        VARIANT var;
        if (result->deviceID().isEmpty()) {
            if ((pDiskObject->Get(_bstr_t(L"DeviceID"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
                if (var.vt == VT_BSTR) {
                    result->setDeviceID(QString::fromWCharArray(var.bstrVal));
                    qDebug() << "DeviceID " << result->deviceID();
                }
                VariantClear(&var);
            }
        }

        if ((pDiskObject->Get(_bstr_t(L"Model"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setModel(QString::fromWCharArray(var.bstrVal));
                qDebug() << "Disk model: " << result->model();
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"Size"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSize(QString::fromWCharArray(var.bstrVal).toULongLong());
                qDebug() << "Size " << result->size();
            } else if (var.vt == VT_I4) {
                result->setSize(var.intVal);
                qDebug() << "Size " << result->size();
            } else if (var.vt == VT_UI4) {
                result->setSize(var.uintVal);
                qDebug() << "Size " << result->size();
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"BytesPerSector"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSectorSize(QString::fromWCharArray(var.bstrVal).toULongLong());
                qDebug() << "Sector size " << result->sectorSize();
            } else if (var.vt == VT_I4) {
                result->setSectorSize(var.intVal);
                qDebug() << "Sector size " << result->sectorSize();
            } else if (var.vt == VT_UI4) {
                result->setSectorSize(var.uintVal);
                qDebug() << "Sector size " << result->sectorSize();
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"SerialNumber"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSerialNumber(QString::fromWCharArray(var.bstrVal));
                qDebug() << "Serial number " << result->serialNumber();
            }
            VariantClear(&var);
        }
        pDiskObject->Release();
    }
    pEnumDiskObjects->Release();

    return result;
}

LibWMIDiskDrive::LibWMIDiskDrive(quint32 index, const QString &deviceID)
    : m_index(index)
    , m_deviceID(deviceID)
{
}

quint32 LibWMIDiskDrive::index() const
{
    return m_index;
}

QString LibWMIDiskDrive::deviceID() const
{
    return m_deviceID;
}

void LibWMIDiskDrive::setDeviceID(const QString &deviceID)
{
    m_deviceID = deviceID;
}

QString LibWMIDiskDrive::model() const
{
    return m_model;
}

void LibWMIDiskDrive::setModel(const QString &model)
{
    m_model = model;
}

quint64 LibWMIDiskDrive::size() const
{
    return m_size;
}

void LibWMIDiskDrive::setSize(quint64 size)
{
    m_size = size;
}

QString LibWMIDiskDrive::serialNumber() const
{
    return m_serialNumber;
}

void LibWMIDiskDrive::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

quint32 LibWMIDiskDrive::sectorSize()
{
    return m_sectorSize;
}

void LibWMIDiskDrive::setSectorSize(quint32 sectorSize)
{
    m_sectorSize = sectorSize;
}
