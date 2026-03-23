/*
 * Fedora Media Writer
 * Copyright (C) 2022-2026 Jan Grulich <jgrulich@redhat.com>
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

#include <QStringList>
#include <QTimer>

#include <dbt.h>

#pragma comment(lib, "wbemuuid.lib")

WinDriveProvider::WinDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
    , m_diskManagement(std::make_unique<WinDiskManagement>(this))
{
    mDebug() << this->metaObject()->className() << "construction";
    QTimer::singleShot(0, this, &WinDriveProvider::checkDrives);
}

void WinDriveProvider::checkDrives()
{
    mDebug() << this->metaObject()->className() << "Looking for the drives";

    for (const Drive *drive : m_drives) {
        // Ignore device change events when we are restoring or writting and schedule
        // re-check once we are done
        if (drive->isBusy()) {
            // Skip this round
            QTimer::singleShot(2500, this, &WinDriveProvider::checkDrives);
            return;
        }
    }

    QMap<int, Drive *> drives;
    auto usbDeviceList = m_diskManagement->getUSBDeviceList();
    for (const quint32 index : usbDeviceList) {
        bool mountable = true;
        auto partitionList = m_diskManagement->getDevicePartitions(index);
        if (partitionList.isEmpty()) {
            mountable = false;
        } else {
            for (auto it = partitionList.constBegin(); it != partitionList.constEnd(); it++) {
                if (!it.value()) {
                    mountable = false;
                    break;
                }
            }
        }

        auto diskDrive = m_diskManagement->getDiskDriveInformation(index);
        if (diskDrive->name().isEmpty() || !diskDrive->size() || diskDrive->serialNumber().isEmpty()) {
            continue;
        }
        Drive *currentDrive = new Drive(this, diskDrive->name(), QString::number(diskDrive->index()), diskDrive->serialNumber(), diskDrive->size(), !mountable);
        drives[diskDrive->index()] = currentDrive;
    }

    // Update our list of drives and notify about added and removed drives
    QList<int> driveIndexes = m_drives.keys();
    for (auto it = drives.constBegin(); it != drives.constEnd(); it++) {
        if (m_drives.contains(it.key()) && *m_drives[it.key()] == *it.value()) {
            mDebug() << "Drive " << it.key() << " already exists";
            it.value()->deleteLater();
            driveIndexes.removeAll(it.key());
            continue;
        }

        if (m_drives.contains(it.key())) {
            mDebug() << "Replacing old drive in the list on index " << it.key();
            Q_EMIT driveRemoved(m_drives[it.key()]);
            m_drives[it.key()]->deleteLater();
            m_drives.remove(it.key());
        }

        mDebug() << "Adding new drive to the list with index " << it.key();
        m_drives[it.key()] = it.value();
        Q_EMIT driveConnected(it.value());

        driveIndexes.removeAll(it.key());
    }

    for (int index : driveIndexes) {
        mDebug() << "Removing old drive with index" << index;
        Q_EMIT driveRemoved(m_drives[index]);
        m_drives[index]->deleteLater();
        m_drives.remove(index);
    }

    if (!m_initialized) {
        m_initialized = true;
        Q_EMIT initializedChanged();
    }

    QTimer::singleShot(2500, this, &WinDriveProvider::checkDrives);
}
