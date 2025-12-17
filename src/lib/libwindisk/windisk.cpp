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

#include "windisk.h"

#include <QDebug>

#include <comdef.h>
#include <dbt.h>

#include <QStandardPaths>
#include <QThread>

#define INITGUID
#include <guiddef.h>
// Define the partition GUID for a basic data partition (for GPT)
DEFINE_GUID(PARTITION_BASIC_DATA_GUID, 0xebd0a0a2, 0xb9e5, 0x4433, 0x87, 0xc0, 0x68, 0xb6, 0xb7, 0x26, 0x99, 0xc7);

#pragma comment(lib, "wbemuuid.lib")

WinDiskManagement::WinDiskManagement(QObject *parent, bool isHelper)
    : QObject(parent)
{
    HRESULT res = S_OK;

    if (isHelper) {
        QString debugFileName = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BazziteMediaWriter-helper.log";
        m_debugFile = _fsopen(debugFileName.toStdString().c_str(), "w", _SH_DENYWR);

        // Looks that for the app itself CoInitializeEx will be called by the platform plugin, but not
        // for the helper. This should be same arguments to CoInitializeEx used for the platform plugin.
        // Call CoInitializeEx will not fail in case it was already called, but it will fail if it was
        // called with different flags, therefore try to use same flags, but in case it changes for some
        // reason check whether the result is not RPC_E_CHANGED_MODE, in which was we should not fail.
        res = CoInitializeEx(NULL, COINIT(COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE));
        if (FAILED(res) && res != RPC_E_CHANGED_MODE) {
            _com_error err(res);
            logMessage(QtWarningMsg, QStringLiteral("Failed to initialize COM library. Error = %1").arg(err.ErrorMessage()));
            return;
        }
    }

    // This needs to be initialized before any RPC communication occurs
    // Currently when used in WinDriveManager we are good.
    res = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, 0);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtWarningMsg, QStringLiteral("Failed to initialize security. Error = %1").arg(err.ErrorMessage()));
        return;
    }

    res = CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void **>(&m_IWbemLocator));
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtWarningMsg, QStringLiteral("Failed to create IWbemLocator object. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return;
    }

    res = m_IWbemLocator->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, NULL, 0, NULL, NULL, &m_IWbemServices);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtWarningMsg, QStringLiteral("Could not connect to WMI. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return;
    }

    res = m_IWbemLocator->ConnectServer(_bstr_t(L"ROOT\\Microsoft\\Windows\\Storage"), NULL, NULL, NULL, 0, NULL, NULL, &m_IWbemStorageServices);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtWarningMsg, QStringLiteral("Could not connect to Window Storage. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return;
    }

    m_wmiInitialized = true;
}

WinDiskManagement::~WinDiskManagement()
{
    if (m_IWbemLocator) {
        m_IWbemLocator->Release();
    }
    if (m_IWbemServices) {
        m_IWbemServices->Release();
    }
    if (m_IWbemStorageServices) {
        m_IWbemStorageServices->Release();
    }
    CoUninitialize();

    if (m_debugFile) {
        fclose(m_debugFile);
    }
}

void WinDiskManagement::logMessage(QtMsgType type, const QString &msg)
{
    if (m_debugFile) {
        QString txt;
        switch (type) {
        case QtDebugMsg:
            txt = QString("WinDiskManagement[D]: %1").arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("WinDiskManagement[I]: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("WinDiskManagement[W]: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("WinDiskManagement[C]: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("WinDiskManagement[F]: %1").arg(msg);
            break;
        }
        fprintf(m_debugFile, "%s\n", txt.toStdString().c_str());
        fflush(m_debugFile);
        return;
    }

    switch (type) {
    case QtDebugMsg:
        qDebug() << "WinDiskManagement[D]: " << msg;
        break;
    case QtInfoMsg:
        qInfo() << "WinDiskManagement[I]: " << msg;
        break;
    case QtWarningMsg:
        qWarning() << "WinDiskManagement[W]: " << msg;
        break;
    case QtCriticalMsg:
        qCritical() << "WinDiskManagement[C]: " << msg;
        break;
    case QtFatalMsg:
        qFatal() << "WinDiskManagement[F]: " << msg;
        break;
    }
}

QVector<quint32> WinDiskManagement::getUSBDeviceList()
{
    logMessage(QtDebugMsg, QStringLiteral("Enumerating USB devices"));

    QVector<quint32> result;
    if (!m_wmiInitialized) {
        logMessage(QtCriticalMsg, QStringLiteral("WMI interface is not initialized"));
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pEnumDiskObjects = NULL;

    res = m_IWbemServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(L"SELECT * FROM Win32_DiskDrive"), WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskObjects);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("WMI query failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return result;
    }

    while (true) {
        IWbemClassObject *pDiskObject = NULL;
        ULONG uReturn = 0;
        pEnumDiskObjects->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturn);
        if (uReturn == 0) {
            break;
        }

        quint32 index = 0;
        QString mediaType;
        VARIANT var;

        if ((pDiskObject->Get(_bstr_t(L"Index"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_I4) {
                index = var.intVal;
            } else if (var.vt == VT_UI4) {
                index = var.uintVal;
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"MediaType"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                mediaType = QString::fromWCharArray(var.bstrVal);
            }
            VariantClear(&var);
        }
        pDiskObject->Release();

        if (mediaType != QStringLiteral("Removable Media") && mediaType != QStringLiteral("External hard disk media")) {
            logMessage(QtDebugMsg, QStringLiteral("Device with index %1 is not removable").arg(index));
            continue;
        }

        if (index > 0) {
            logMessage(QtDebugMsg, QStringLiteral("Found removable device with index %1").arg(index));
            result << index;
        }
    }
    pEnumDiskObjects->Release();

    return result;
}

QMap<quint32, bool> WinDiskManagement::getDevicePartitions(quint32 index)
{
    logMessage(QtDebugMsg, QStringLiteral("Enumerating partitions for device with index %1").arg(index));

    QMap<quint32, bool> result;
    if (!m_wmiInitialized) {
        logMessage(QtCriticalMsg, QStringLiteral("WMI interface is not initialized"));
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pPartitionObjects = NULL;
    std::wstring partitionQuery = L"SELECT * FROM MSFT_Partition WHERE DiskNumber = " + std::to_wstring(index);

    res = m_IWbemStorageServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(partitionQuery.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pPartitionObjects);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("Query for disk partitions failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return result;
    }

    while (true) {
        IWbemClassObject *pPartitionObject = NULL;
        ULONG uReturnPartition = 0;
        pPartitionObjects->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturnPartition);
        if (uReturnPartition == 0) {
            break;
        }

        VARIANT var;
        bool mountable = false;
        qint32 partitionIndex = -1;
        QString partitionPath;

        if ((pPartitionObject->Get(_bstr_t(L"PartitionNumber"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_I4) {
                partitionIndex = var.intVal;
            } else if (var.vt == VT_UI4) {
                partitionIndex = var.uintVal;
            }
            VariantClear(&var);
        }

        if ((pPartitionObject->Get(_bstr_t(L"__PATH"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                partitionPath = QString::fromWCharArray(var.bstrVal);
            }
            VariantClear(&var);
        }

        wchar_t driveLetter;
        if ((pPartitionObject->Get(_bstr_t(L"DriveLetter"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_I2) {
                driveLetter = static_cast<wchar_t>(var.iVal);
                mountable = driveLetter != L'\0';
            }
            VariantClear(&var);
        }

        pPartitionObject->Release();

        if (partitionIndex != -1) {
            if (mountable) {
                logMessage(QtDebugMsg, QStringLiteral("Found partition with index %1 mounted to %2").arg(partitionIndex).arg(QString::fromWCharArray(&driveLetter, 1)));
            } else {
                logMessage(QtDebugMsg, QStringLiteral("Found unmounted partition with index %1").arg(partitionIndex));
            }
            result.insert(static_cast<quint32>(partitionIndex), mountable);
        }
    }
    pPartitionObjects->Release();

    return result;
}

std::unique_ptr<WinDisk> WinDiskManagement::getDiskDriveInformation(quint32 index, const QString &diskPath)
{
    logMessage(QtDebugMsg, QStringLiteral("Obtaining disk drive information for disk with index %1").arg(index));

    std::unique_ptr<WinDisk> result = std::make_unique<WinDisk>(index, diskPath);
    if (!m_wmiInitialized) {
        logMessage(QtCriticalMsg, QStringLiteral("WMI interface is not initialized"));
        return result;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pEnumDiskObjects = NULL;

    std::wstring deviceQuery = L"SELECT * FROM MSFT_Disk WHERE Number = " + std::to_wstring(index);
    res = m_IWbemStorageServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(deviceQuery.c_str()), WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDiskObjects);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("WMI query failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
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
        if (result->path().isEmpty()) {
            if ((pDiskObject->Get(_bstr_t(L"__PATH"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
                if (var.vt == VT_BSTR) {
                    result->setPath(QString::fromWCharArray(var.bstrVal));
                    logMessage(QtDebugMsg, QStringLiteral("DeviceID %1").arg(result->path()));
                }
                VariantClear(&var);
            }
        }

        if ((pDiskObject->Get(_bstr_t(L"IsOffline"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BOOL) {
                result->setIsOffline(var.boolVal);
                logMessage(QtDebugMsg, QStringLiteral("Disk is offline: %1").arg(result->isOffline()));
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"FriendlyName"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setName(QString::fromWCharArray(var.bstrVal));
                logMessage(QtDebugMsg, QStringLiteral("Disk name: %1").arg(result->name()));
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"Size"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSize(QString::fromWCharArray(var.bstrVal).toULongLong());
                logMessage(QtDebugMsg, QStringLiteral("Size %1").arg(result->size()));
            } else if (var.vt == VT_I4) {
                result->setSize(var.intVal);
                logMessage(QtDebugMsg, QStringLiteral("Size %1").arg(result->size()));
            } else if (var.vt == VT_UI4) {
                result->setSize(var.uintVal);
                logMessage(QtDebugMsg, QStringLiteral("Size %1").arg(result->size()));
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"PhysicalSectorSize"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSectorSize(QString::fromWCharArray(var.bstrVal).toULongLong());
                logMessage(QtDebugMsg, QStringLiteral("Sector size %1").arg(result->sectorSize()));
            } else if (var.vt == VT_I4) {
                result->setSectorSize(var.intVal);
                logMessage(QtDebugMsg, QStringLiteral("Sector size %1").arg(result->sectorSize()));
            } else if (var.vt == VT_UI4) {
                result->setSectorSize(var.uintVal);
                logMessage(QtDebugMsg, QStringLiteral("Sector size %1").arg(result->sectorSize()));
            }
            VariantClear(&var);
        }

        if ((pDiskObject->Get(_bstr_t(L"SerialNumber"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSerialNumber(QString::fromWCharArray(var.bstrVal));
                logMessage(QtDebugMsg, QStringLiteral("Serial number %1").arg(result->serialNumber()));
            }
            VariantClear(&var);
        }

        if (result->serialNumber().isEmpty() && (pDiskObject->Get(_bstr_t(L"UniqueId"), 0, &var, 0, 0)) == WBEM_S_NO_ERROR) {
            if (var.vt == VT_BSTR) {
                result->setSerialNumber(QString::fromWCharArray(var.bstrVal));
                logMessage(QtDebugMsg, QStringLiteral("Using unique ID as serial number %1").arg(result->serialNumber()));
            }
            VariantClear(&var);
        }
        pDiskObject->Release();
    }
    pEnumDiskObjects->Release();

    return result;
}

bool WinDiskManagement::clearPartitions(qint32 index)
{
    logMessage(QtDebugMsg, QStringLiteral("Removing partitions on disk with index %1").arg(index));

    if (!m_wmiInitialized) {
        logMessage(QtWarningMsg, QStringLiteral("WMI interface is not initialized"));
        return false;
    }

    HRESULT res = S_OK;

    IEnumWbemClassObject *pPartitionObjects = NULL;
    std::wstring partitionQuery = L"SELECT * FROM MSFT_Partition WHERE DiskNumber = " + std::to_wstring(index);

    res = m_IWbemStorageServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(partitionQuery.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pPartitionObjects);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("Query for disk partitions failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return false;
    }

    while (true) {
        IWbemClassObject *pPartitionObject = NULL;
        ULONG uReturnPartition = 0;
        pPartitionObjects->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturnPartition);
        if (uReturnPartition == 0) {
            break;
        }
        VARIANT var;
        res = pPartitionObject->Get(L"__PATH", 0, &var, NULL, NULL);
        if (SUCCEEDED(res)) {
            QString partitionPath = QString::fromWCharArray(var.bstrVal);
            IWbemClassObject *pOutParams = NULL;
            res = m_IWbemStorageServices->ExecMethod(_bstr_t(partitionPath.toStdWString().c_str()), _bstr_t(L"DeleteObject"), 0, NULL, NULL, &pOutParams, NULL);
            if (FAILED(res)) {
                _com_error err(res);
                logMessage(QtCriticalMsg, QStringLiteral("Failed to delete partition. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
                return false;
            }

            if (pOutParams) {
                VARIANT returnValueVar;
                VariantInit(&returnValueVar);
                pOutParams->Get(L"ReturnValue", 0, &returnValueVar, NULL, NULL);
                if (returnValueVar.vt == VT_I4 && returnValueVar.intVal != 0) {
                    logMessage(QtCriticalMsg, QStringLiteral("Failed to delete partition. Error code: %1").arg(QString::number(returnValueVar.intVal, 16)));
                    VariantClear(&returnValueVar);
                    return false;
                }
                pOutParams->Release();
            }
        }
        pPartitionObject->Release();
    }
    pPartitionObjects->Release();

    logMessage(QtDebugMsg, QStringLiteral("Partitions deleted successfully."));

    return true;
}

bool WinDiskManagement::formatPartition(const QChar &driveLetter)
{
    logMessage(QtDebugMsg, QStringLiteral("Formatting partition mounted to drive letter %1:").arg(driveLetter));

    if (!m_wmiInitialized) {
        logMessage(QtCriticalMsg, QStringLiteral("WMI interface is not initialized"));
        return false;
    }

    HRESULT res = S_OK;
    IEnumWbemClassObject *pVolumeObjects = NULL;
    std::wstring query = L"SELECT * FROM MSFT_Volume WHERE DriveLetter='";
    query.push_back(driveLetter.toUpper().unicode());
    query.push_back(L'\'');

    res = m_IWbemStorageServices->ExecQuery(_bstr_t(L"WQL"), _bstr_t(query.c_str()), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pVolumeObjects);
    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("Query for MSFT_Volume failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return false;
    }

    bool volumeFound = false;
    while (true) {
        IWbemClassObject *pVolumeObject = NULL;
        ULONG uReturnVolume = 0;
        pVolumeObjects->Next(WBEM_INFINITE, 1, &pVolumeObject, &uReturnVolume);
        if (uReturnVolume == 0) {
            logMessage(QtWarningMsg, QStringLiteral("No volume object"));
            break;
        }

        QString volumePath;
        VARIANT volumePathVar;
        VariantInit(&volumePathVar);
        res = pVolumeObject->Get(L"__PATH", 0, &volumePathVar, NULL, NULL);
        if (SUCCEEDED(res)) {
            volumePath = QString::fromWCharArray(volumePathVar.bstrVal);
            VariantClear(&volumePathVar);
        }
        pVolumeObject->Release();

        IWbemClassObject *pClass = NULL;
        IWbemClassObject *pInParamsDefinition = NULL;
        IWbemClassObject *pInParams = NULL;
        IWbemClassObject *pOutParams = NULL;

        auto cleanup = qScopeGuard([=] {
            if (pOutParams) {
                pOutParams->Release();
            }
            if (pInParams) {
                pInParams->Release();
            }
            if (pInParamsDefinition) {
                pInParamsDefinition->Release();
            }
            if (pClass) {
                pClass->Release();
            }
        });

        res = m_IWbemStorageServices->GetObject(_bstr_t(L"MSFT_Volume"), 0, NULL, &pClass, NULL);
        if (FAILED(res)) {
            _com_error err(res);
            logMessage(QtCriticalMsg, QStringLiteral("WMI query to get MSFT_Volume object failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
            return false;
        }

        res = pClass->GetMethod(_bstr_t(L"Format"), 0, &pInParamsDefinition, NULL);
        if (FAILED(res)) {
            _com_error err(res);
            logMessage(QtCriticalMsg, QStringLiteral("WMI query to get 'Format' method on MSFT_Volume object failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
            return false;
        }

        res = pInParamsDefinition->SpawnInstance(0, &pInParams);
        if (FAILED(res)) {
            _com_error err(res);
            logMessage(QtCriticalMsg, QStringLiteral("WMI spawn instance failed. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
            return false;
        }

        VARIANT var;
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = _bstr_t(L"exFAT");
        res = pInParams->Put(L"FileSystem", 0, &var, 0);
        VariantClear(&var);

        if (FAILED(res)) {
            _com_error err(res);
            logMessage(QtCriticalMsg, QStringLiteral("Failed to set 'FileSystem' parameter. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
            return false;
        }

        var.vt = VT_BOOL;
        var.boolVal = VARIANT_FALSE; // Quick format
        res = pInParams->Put(L"Full", 0, &var, 0);
        VariantClear(&var);

        if (FAILED(res)) {
            logMessage(QtCriticalMsg, QStringLiteral("Failed to set 'Full' parameter."));
            return false;
        }

        volumeFound = true;
        res = m_IWbemStorageServices->ExecMethod(_bstr_t(volumePath.toStdString().c_str()), _bstr_t(L"Format"), 0, NULL, pInParams, &pOutParams, NULL);
        if (FAILED(res)) {
            _com_error err(res);
            logMessage(QtCriticalMsg, QStringLiteral("Failed to format the volume. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        }

        if (pOutParams) {
            VARIANT returnValueVar;
            VariantInit(&returnValueVar);
            pOutParams->Get(L"ReturnValue", 0, &returnValueVar, NULL, NULL);
            if (returnValueVar.vt == VT_I4 && returnValueVar.intVal != 0) {
                logMessage(QtCriticalMsg, QStringLiteral("Failed to format the volume. Error code: %1").arg(QString::number(returnValueVar.intVal, 16)));
                VariantClear(&returnValueVar);
                return false;
            } else {
                logMessage(QtDebugMsg, QStringLiteral("Volume successfully formatted to exFat."));
                volumeFound = true;
            }
            pOutParams->Release();
        }

        if (volumeFound) {
            break;
        }
    }
    pVolumeObjects->Release();

    if (!volumeFound) {
        logMessage(QtWarningMsg, QStringLiteral("No volumes found for the partition."));
        return false;
    }

    return true;
}

bool WinDiskManagement::refreshDiskDrive(const QString &diskPath)
{
    logMessage(QtDebugMsg, QStringLiteral("Refreshing disk drive information"));

    if (!m_wmiInitialized) {
        logMessage(QtCriticalMsg, QStringLiteral("WMI interface is not initialized"));
        return false;
    }

    HRESULT res = S_OK;
    IWbemClassObject *pOutParams = NULL;

    res = m_IWbemStorageServices->ExecMethod(_bstr_t(diskPath.toStdWString().c_str()), _bstr_t(L"Refresh"), 0, NULL, NULL, &pOutParams, NULL);

    if (pOutParams) {
        pOutParams->Release();
    }

    if (FAILED(res)) {
        _com_error err(res);
        logMessage(QtCriticalMsg, QStringLiteral("Failed to refresh the disk. Error = %1").arg(QString::fromWCharArray(err.ErrorMessage())));
        return false;
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully refreshed the disk."));
    return true;
}

static QString getLastError()
{
    TCHAR message[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
    return QString::fromWCharArray(message).trimmed();
}

bool WinDiskManagement::lockDrive(HANDLE driveHandle, int numRetries)
{
    logMessage(QtDebugMsg, QStringLiteral("Trying to lock the drive"));

    int attempts = 0;
    DWORD status;

    while (true) {
        if (!DeviceIoControl(driveHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &status, NULL)) {
            attempts++;
        } else {
            logMessage(QtDebugMsg, QStringLiteral("Successfully locked the drive"));
            return true;
        }

        if (attempts == numRetries) {
            logMessage(QtWarningMsg, QStringLiteral("Couldn't lock the drive: %1").arg(getLastError()));
            break;
        }

        QThread::sleep(2);
    }

    return false;
}

bool WinDiskManagement::unlockDrive(HANDLE driveHandle)
{
    logMessage(QtDebugMsg, QStringLiteral("Trying to unlock the drive"));

    bool ret = DeviceIoControl(driveHandle, FSCTL_UNLOCK_VOLUME, NULL, 0, NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtFatalMsg, QStringLiteral("Failed to unlock the drive"));
        return false;
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully unlocked the drive"));
    return true;
}

bool WinDiskManagement::disableIOBoundaryChecks(HANDLE driveHandle)
{
    logMessage(QtDebugMsg, QStringLiteral("Trying to disable I/O boundary checks"));

    bool ret = DeviceIoControl(driveHandle, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtFatalMsg, QStringLiteral("Failed to disable I/O boundary checks"));
        return false;
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully disabled I/O boundary checks"));
    return true;
}

bool WinDiskManagement::removeDriveLetters(quint32 index)
{
    logMessage(QtDebugMsg, QStringLiteral("Removing assigned drive letters for device with index %1").arg(index));

    DWORD drives = ::GetLogicalDrives();

    for (char i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            char currentDrive = 'A' + i;
            QString drivePath = QString("\\\\.\\%1:").arg(currentDrive);
            logMessage(QtDebugMsg, QStringLiteral("Checking drive: %1").arg(drivePath));
            HANDLE device = ::CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
            if (device == INVALID_HANDLE_VALUE) {
                logMessage(QtWarningMsg, QStringLiteral("Failed to open logical drive: %1").arg(currentDrive));
                continue;
            }

            auto cleanup = qScopeGuard([device] {
                CloseHandle(device);
            });

            DWORD bytesReturned;
            VOLUME_DISK_EXTENTS vde;
            if (DeviceIoControl(device, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &vde, sizeof(vde), &bytesReturned, NULL)) {
                for (uint j = 0; j < vde.NumberOfDiskExtents; j++) {
                    if (vde.Extents[j].DiskNumber == index) {
                        QString volumePath = QString("%1:\\").arg(currentDrive);
                        logMessage(QtWarningMsg, QStringLiteral("Checking volume: %1").arg(volumePath));
                        if (!DeleteVolumeMountPointA(volumePath.toStdString().c_str())) {
                            logMessage(QtCriticalMsg, QStringLiteral("Couldn't remove the drive: %1").arg(getLastError()));
                            return false;
                        }
                        logMessage(QtDebugMsg, QStringLiteral("Successfully removed mountpoints for volume: %1").arg(currentDrive));
                        break;
                    }
                }
            }
        }
    }

    return true;
}

bool WinDiskManagement::unmountVolume(HANDLE logicalHandle)
{
    logMessage(QtDebugMsg, QStringLiteral("Unmounting logical volume"));

    if (!DeviceIoControl(logicalHandle, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, NULL, NULL)) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't unmount drive: %1").arg(getLastError()));
        return false;
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully unmounted logical volume"));

    return true;
}

QChar WinDiskManagement::mountVolume(const QString &logicalName)
{
    logMessage(QtDebugMsg, QStringLiteral("Mounting logical volume"));

    char mountedLetter[27] = {0};
    DWORD size;
    if (::GetVolumePathNamesForVolumeNameA(logicalName.toStdString().c_str(), mountedLetter, sizeof(mountedLetter), &size) && (size > 1)) {
        logMessage(QtDebugMsg, QStringLiteral("Volume is already mounted"));
        return QChar(mountedLetter[0]);
    }

    char drives[256];
    DWORD driveSize = GetLogicalDriveStringsA(sizeof(drives), drives);
    if (!driveSize || driveSize > sizeof(drives)) {
        logMessage(QtCriticalMsg, QStringLiteral("Failed to get drive letter mountpoint: %1").arg(getLastError()));
        return QChar();
    }

    char driveLetter = 0;
    for (char letter = 'C'; letter <= 'Z'; letter++) {
        bool isDriveUsed = std::any_of(drives, drives + driveSize, [letter](char drive) {
            return toupper(drive) == letter;
        });

        if (!isDriveUsed) {
            driveLetter = letter;
            break;
        }
    }

    if (driveLetter == 0) {
        logMessage(QtWarningMsg, QStringLiteral("Couldn't find available drive letter for mountpoint."));
        return QChar();
    }

    std::string drivePath = std::string(1, driveLetter) + ":\\";
    if (!::SetVolumeMountPointA(drivePath.c_str(), logicalName.toStdString().c_str())) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't mount %1 as %2: %3").arg(logicalName).arg(driveLetter).arg(getLastError()));
        return QChar();
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfuly mounted logical volume %1 as %2").arg(logicalName).arg(driveLetter));
    return QChar(driveLetter);
}

qint64 WinDiskManagement::writeFileWithRetry(HANDLE driveHandle, char *buffer, qint64 numberOfBytesToWrite, int numberOfRetries)
{
    bool readFilePointer = false;
    LARGE_INTEGER filePointer;
    LARGE_INTEGER filePointerZero = {{0, 0}};
    DWORD writtenBytes = 0;
    DWORD totalWrittenBytes = 0;

    readFilePointer = SetFilePointerEx(driveHandle, filePointerZero, &filePointer, FILE_CURRENT);
    if (!readFilePointer) {
        logMessage(QtCriticalMsg, QStringLiteral("Could not read file pointer: %1").arg(getLastError()));
        return -1;
    }

    for (int attempts = 1; attempts <= numberOfRetries && totalWrittenBytes < numberOfBytesToWrite; attempts++) {
        DWORD bytesToWrite = static_cast<DWORD>(numberOfBytesToWrite - totalWrittenBytes);
        writtenBytes = 0;

        if (WriteFile(driveHandle, buffer + totalWrittenBytes, static_cast<DWORD>(bytesToWrite), &writtenBytes, NULL)) {
            totalWrittenBytes += writtenBytes;

            if (totalWrittenBytes == numberOfBytesToWrite) {
                return static_cast<qint64>(totalWrittenBytes);
            } else {
                logMessage(QtCriticalMsg, QStringLiteral("Only partial data was written to the drive: %1").arg(getLastError()));
            }
        } else {
            logMessage(QtCriticalMsg, QStringLiteral("Failed to write data to the drive: %1").arg(getLastError()));
        }

        if (totalWrittenBytes < numberOfBytesToWrite) {
            LARGE_INTEGER retryPointer;
            retryPointer.QuadPart = filePointer.QuadPart + totalWrittenBytes;
            if (!SetFilePointerEx(driveHandle, retryPointer, NULL, FILE_BEGIN)) {
                logMessage(QtCriticalMsg, QStringLiteral("Could not reset file pointer for retry."));
                break;
            }

            if (attempts < numberOfRetries) {
                logMessage(QtCriticalMsg, QStringLiteral("Retrying in 5 seconds (Attempt %1 of %2)").arg(attempts).arg(numberOfRetries));
                QThread::sleep(5);
            }
        }
    }

    if (totalWrittenBytes < numberOfBytesToWrite) {
        logMessage(QtCriticalMsg, QStringLiteral("Failed to complete the write after %1 retries").arg(numberOfRetries));
        return -1;
    }

    return totalWrittenBytes;
}

qint64 WinDiskManagement::writeFileAsync(HANDLE driveHandle, char *buffer, qint64 numberOfBytesToWrite, OVERLAPPED *overlap)
{
    DWORD writtenBytes = 0;
    if (WriteFile(driveHandle, buffer, static_cast<DWORD>(numberOfBytesToWrite), &writtenBytes, overlap)) {
        return static_cast<qint64>(writtenBytes);
    } else {
        DWORD error = GetLastError();

        if (error == ERROR_IO_PENDING) {
            if (WaitForSingleObject(overlap->hEvent, INFINITE) == WAIT_OBJECT_0) {
                if (GetOverlappedResult(driveHandle, overlap, &writtenBytes, TRUE)) {
                    return static_cast<qint64>(writtenBytes);
                } else {
                    DWORD overlappedError = GetLastError();
                    logMessage(QtCriticalMsg, QStringLiteral("Async write failed during GetOverlappedResult with error: %1").arg(overlappedError));
                    return -1;
                }
            } else {
                logMessage(QtCriticalMsg, QStringLiteral("Async write failed while waiting with error: %1").arg(error));
                return -1;
            }
        } else {
            logMessage(QtCriticalMsg, QStringLiteral("Async write failed immediately with error: %1").arg(error));
            return -1;
        }
    }
}

bool WinDiskManagement::clearPartitionTable(HANDLE driveHandle, quint64 driveSize, quint32 sectorSize)
{
    logMessage(QtDebugMsg, QStringLiteral("Clearing partition table information"));

    quint64 sectorsToClear = 128;
    LARGE_INTEGER filePointer;

    char *zeroBuffer = static_cast<char *>(calloc(sectorSize, sectorsToClear));
    if (!zeroBuffer) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't allocate zero buffer"));
        return false;
    }

    filePointer.QuadPart = 0ULL;
    if (!SetFilePointerEx(driveHandle, filePointer, &filePointer, FILE_BEGIN) || (filePointer.QuadPart != 0ULL)) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't reset disk position: %1").arg(getLastError()));
    }

    if (!writeFileWithRetry(driveHandle, zeroBuffer, sectorSize * sectorsToClear, 4)) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't write zero data to the beginning of drive: %1").arg(getLastError()));
    }

    filePointer.QuadPart = driveSize - (LONGLONG)sectorSize * sectorsToClear;
    if (SetFilePointerEx(driveHandle, filePointer, &filePointer, FILE_BEGIN)) {
        if (!writeFileWithRetry(driveHandle, zeroBuffer, sectorSize * sectorsToClear, 4)) {
            logMessage(QtCriticalMsg, QStringLiteral("Couldn't write zero data to the end of the drive: %1").arg(getLastError()));
        }
    }

    free(zeroBuffer);

    filePointer.QuadPart = 0ULL;
    if (!SetFilePointerEx(driveHandle, filePointer, &filePointer, FILE_BEGIN) || (filePointer.QuadPart != 0ULL)) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't reset disk to original position: %1").arg(getLastError()));
    }

    if (!refreshPartitionLayout(driveHandle)) {
        logMessage(QtWarningMsg, QStringLiteral("Couldn't update drive properties"));
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully cleared the partition table"));
    return true;
}

bool WinDiskManagement::clearDiskDrive(HANDLE driveHandle)
{
    logMessage(QtDebugMsg, QStringLiteral("Clearing disk drive"));

    BOOL ret;
    CREATE_DISK createDisk = {PARTITION_STYLE_RAW, {{0}}};

    ret = DeviceIoControl(driveHandle, IOCTL_DISK_CREATE_DISK, &createDisk, sizeof(createDisk), NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't delete drive layout"));
        return false;
    }

    if (!refreshPartitionLayout(driveHandle)) {
        logMessage(QtWarningMsg, QStringLiteral("Couldn't update drive properties"));
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully cleared the disk drive"));
    return true;
}

bool WinDiskManagement::createGPTPartition(HANDLE driveHandle, quint64 diskSize, quint32 sectorSize)
{
    logMessage(QtDebugMsg, QStringLiteral("Creating GPT partition table"));

    BOOL ret;

    CREATE_DISK createDisk = {PARTITION_STYLE_GPT, {{0}}};
    CoCreateGuid(&createDisk.Gpt.DiskId);

    ret = DeviceIoControl(driveHandle, IOCTL_DISK_CREATE_DISK, &createDisk, sizeof(createDisk), NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtCriticalMsg, QStringLiteral("Failed to create GPT partition table on the disk."));
        return false;
    }

    ret = DeviceIoControl(driveHandle, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't update disk properties."));
        return false;
    }

    DRIVE_LAYOUT_INFORMATION_EX driveLayout = {0};
    driveLayout.PartitionStyle = PARTITION_STYLE_GPT;
    driveLayout.PartitionCount = 1;

    PARTITION_INFORMATION_EX &partitionInfo = driveLayout.PartitionEntry[0];
    partitionInfo.PartitionStyle = PARTITION_STYLE_GPT;
    // GPT starts at sector 34 (after the GPT header)
    partitionInfo.StartingOffset.QuadPart = 34 * sectorSize;
    // Disk size - GPT header/footer sectors ;
    partitionInfo.PartitionLength.QuadPart = diskSize - ((34 + 33) * sectorSize);
    partitionInfo.Gpt.PartitionType = PARTITION_BASIC_DATA_GUID;
    partitionInfo.Gpt.PartitionId = createDisk.Gpt.DiskId;

    ret = DeviceIoControl(driveHandle, IOCTL_DISK_SET_DRIVE_LAYOUT_EX, &driveLayout, sizeof(driveLayout), NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtCriticalMsg, QStringLiteral("Failed to set GPT partition layout on the disk."));
        return false;
    }

    if (!refreshPartitionLayout(driveHandle)) {
        logMessage(QtWarningMsg, QStringLiteral("Couldn't update drive properties"));
    }

    logMessage(QtDebugMsg, QStringLiteral("Successfully created GPT partition over the whole disk."));
    return true;
}

QString WinDiskManagement::getLogicalName(quint32 index, bool keepTrailingBackslash)
{
    QString result;
    static const char *volumeStart = "\\\\?\\";
    char volumeName[2048];
    char path[2048];
    HANDLE drive = INVALID_HANDLE_VALUE;
    HANDLE volume = INVALID_HANDLE_VALUE;

    for (int i = 0; drive == INVALID_HANDLE_VALUE; i++) {
        if (i == 0) {
            volume = FindFirstVolumeA(volumeName, sizeof(volumeName));
            if (volume == INVALID_HANDLE_VALUE) {
                logMessage(QtWarningMsg, QStringLiteral("Couldn't access GUID volume: %1").arg(getLastError()));
                continue;
            }
        } else {
            if (!FindNextVolumeA(volume, volumeName, sizeof(volumeName))) {
                logMessage(QtDebugMsg, QStringLiteral("Couldn't access next GUID volume: %1").arg(getLastError()));
                break;
            }
        }

        size_t len = strnlen_s(volumeName, 2048);
        if (len <= 4 || _strnicmp(volumeName, volumeStart, 4) != 0 || volumeName[len - 1] != '\\') {
            logMessage(QtWarningMsg, QStringLiteral("Obtained wrong volume name: %1").arg(volumeName));
            continue;
        }

        volumeName[len - 1] = 0;
        if (QueryDosDeviceA(&volumeName[4], path, sizeof(path)) == 0) {
            logMessage(QtWarningMsg, QStringLiteral("Failed to get device path for GUID volume %1: %2").arg(volumeName).arg(getLastError()));
            continue;
        }
        drive = ::CreateFileA(volumeName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (drive == INVALID_HANDLE_VALUE) {
            logMessage(QtWarningMsg, QStringLiteral("Couldn't open GUID volume %1: %2").arg(volumeName).arg(getLastError()));
            continue;
        }

        DWORD size = 0;
        VOLUME_DISK_EXTENTS diskExtents;
        BOOL ret = DeviceIoControl(drive, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &diskExtents, sizeof(diskExtents), &size, NULL);
        CloseHandle(drive);
        drive = INVALID_HANDLE_VALUE;
        if (!ret || size == 0) {
            logMessage(QtWarningMsg, QStringLiteral("Couldn't open GUID volume %1: %2").arg(volumeName).arg(getLastError()));
            continue;
        }

        if (diskExtents.NumberOfDiskExtents == 0 || diskExtents.NumberOfDiskExtents != 1) {
            logMessage(QtWarningMsg, QStringLiteral("Wrong number of disk extents."));
            continue;
        }

        if (diskExtents.Extents[0].DiskNumber != index) {
            continue;
        }

        if (keepTrailingBackslash) {
            volumeName[len - 1] = '\\';
        }
        result = QString(volumeName);
        break;
    }

    if (result.isEmpty()) {
        logMessage(QtDebugMsg, QStringLiteral("No logical volume found. Device doesn't have any partition"));
    }

    return result;
}

bool WinDiskManagement::refreshPartitionLayout(HANDLE driveHandle)
{
    logMessage(QtDebugMsg, QStringLiteral("Refreshing information about partition layout"));

    BOOL ret = DeviceIoControl(driveHandle, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, NULL, NULL);
    if (!ret) {
        logMessage(QtCriticalMsg, QStringLiteral("Couldn't update disk properties."));
        return false;
    }

    return true;
}

WinDisk::WinDisk(quint32 index, const QString &path)
    : m_index(index)
    , m_path(path)
{
}

quint32 WinDisk::index() const
{
    return m_index;
}

bool WinDisk::isOffline() const
{
    return m_isOffline;
}

void WinDisk::setIsOffline(bool isOffline)
{
    m_isOffline = isOffline;
}

QString WinDisk::path() const
{
    return m_path;
}

void WinDisk::setPath(const QString &path)
{
    m_path = path;
}

QString WinDisk::name() const
{
    return m_name;
}

void WinDisk::setName(const QString &name)
{
    m_name = name;
}

quint64 WinDisk::size() const
{
    return m_size;
}

void WinDisk::setSize(quint64 size)
{
    m_size = size;
}

QString WinDisk::serialNumber() const
{
    return m_serialNumber;
}

void WinDisk::setSerialNumber(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

quint32 WinDisk::sectorSize()
{
    return m_sectorSize;
}

void WinDisk::setSectorSize(quint32 sectorSize)
{
    m_sectorSize = sectorSize;
}
