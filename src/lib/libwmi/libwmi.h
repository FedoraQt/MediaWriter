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

#ifndef LIBWMI_H
#define LIBWMI_H

#include <QObject>
#include <wbemidl.h>

class LibWMIDiskDrive;

class LibWMI : public QObject
{
    Q_OBJECT
public:
    LibWMI(QObject *parent);
    ~LibWMI();

    // Returns a map<index, deviceID> with list of devices
    QMap<quint32, QString> getUSBDeviceList();
    // Returns a list of partition IDs for device with given index
    QStringList getDevicePartitions(quint32 index);
    // Returns a list of logical disks IDs for given partition
    QStringList getLogicalDisks(const QString &partitionID);
    // Returns information about disk drive
    std::unique_ptr<LibWMIDiskDrive> getDiskDriveInformation(quint32 index, const QString &deviceID = QString());

private:
    bool initialized = false;
    IWbemLocator *m_IWbemLocator = NULL;
    IWbemServices *m_IWbemServices = NULL;
};

class LibWMIDiskDrive
{
public:
    LibWMIDiskDrive(quint32 index, const QString &deviceID = QString());

    quint32 index() const;

    QString deviceID() const;
    void setDeviceID(const QString &deviceID);

    QString model() const;
    void setModel(const QString &model);

    quint64 size() const;
    void setSize(quint64 size);

    QString serialNumber() const;
    void setSerialNumber(const QString &serialNumber);

    quint32 sectorSize();
    void setSectorSize(quint32 sectorSize);

private:
    quint32 m_index = 0;
    quint32 m_sectorSize = 0;
    quint64 m_size = 0;
    QString m_deviceID;
    QString m_model;
    QString m_serialNumber;
};

#endif // LIBWMI_H
