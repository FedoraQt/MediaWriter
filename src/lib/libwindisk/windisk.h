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

#ifndef WINDISKMANAGEMENT_H
#define WINDISKMANAGEMENT_H

#include <QObject>
#include <wbemidl.h>

class WinDisk;

class WinDiskManagement : public QObject
{
    Q_OBJECT
public:
    WinDiskManagement(QObject *parent, bool isHelper = false);
    ~WinDiskManagement();

    void logMessage(QtMsgType type, const QString &msg);

    /*
     *  WMI - Windows Management Instrumentation
     */
    // Returns a vector<index> with list of devices
    QVector<quint32> getUSBDeviceList();
    // Returns a map<index, mountable> of partitions for device with given @index
    QMap<quint32, bool> getDevicePartitions(quint32 index);
    // Returns information about disk drive on given @index
    std::unique_ptr<WinDisk> getDiskDriveInformation(quint32 index, const QString &diskPath = QString());
    // Remove all partitions on device on given @index
    bool clearPartitions(qint32 index);
    // Formats partition to exFAT on given @partitionPath
    bool formatPartition(const QChar &driveLetter);
    // Refreshes disk drive on given @diskPath
    bool refreshDiskDrive(const QString &diskPath);

    /*
     * WinAPI
     */
    // Locks the drive and try @numRetries attempts if we fail
    bool lockDrive(HANDLE driveHandle, int numRetries = 1);
    // Unlocks the drive
    bool unlockDrive(HANDLE driveHandle);
    // Try to disable I/O boundary checks
    bool disableIOBoundaryChecks(HANDLE driveHandle);
    // Remove all assigned drive letters
    bool removeDriveLetters(quint32 index);
    // Unmount volume provided by logical handle
    bool unmountVolume(HANDLE volumeHandle);
    // Mount volume provided by GUID path and return drive letter it's mounted to
    QChar mountVolume(const QString &volume);
    // Clears GPT/MBR records after writing ISO image
    bool clearPartitionTable(HANDLE driveHandle, quint64 driveSize, quint32 sectorSize);
    // Clears the drive and sets it to RAW state
    bool clearDiskDrive(HANDLE driveHandle);
    // Creates a GPT partition table on the drive provided by @driveHandle
    bool createGPTPartition(HANDLE driveHandle, quint64 diskSize, quint32 sectorSize);
    // Returns the GUID volume name
    QString getLogicalName(quint32 index, bool keepTrailingBackslash = true);
    // Refreshes the partition layout
    bool refreshPartitionLayout(HANDLE driveHandle);
    qint64 writeFileWithRetry(HANDLE driveHandle, char *buffer, qint64 numberOfBytesToWrite, int numberOfRetries = 1);
    qint64 writeFileAsync(HANDLE driveHandle, char *buffer, qint64 numberOfBytesToWrite, OVERLAPPED *overlap);

private:
    bool m_wmiInitialized = false;
    IWbemLocator *m_IWbemLocator = NULL;
    IWbemServices *m_IWbemServices = NULL;
    IWbemServices *m_IWbemStorageServices = NULL;
    FILE *m_debugFile = nullptr;
};

class WinDisk
{
public:
    WinDisk(quint32 index, const QString &path = QString());

    quint32 index() const;

    bool isOffline() const;
    void setIsOffline(bool offline);

    QString path() const;
    void setPath(const QString &path);

    QString name() const;
    void setName(const QString &name);

    quint64 size() const;
    void setSize(quint64 size);

    QString serialNumber() const;
    void setSerialNumber(const QString &serialNumber);

    quint32 sectorSize();
    void setSectorSize(quint32 sectorSize);

private:
    bool m_isOffline = false;
    quint32 m_index = 0;
    quint32 m_sectorSize = 0;
    quint64 m_size = 0;
    QString m_path;
    QString m_name;
    QString m_serialNumber;
};

#endif // WINDISKMANAGEMENT_H
