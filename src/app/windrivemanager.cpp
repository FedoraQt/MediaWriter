/*
 * Fedora Media Writer
 * Copyright (C) 2022-2024 Jan Grulich <jgrulich@redhat.com>
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

    for (const WinDrive *drive : m_drives) {
        // Ignore device change events when we are restoring or writting and schedule
        // re-check once we are done
        if (drive->busy()) {
            // Skip this round
            QTimer::singleShot(2500, this, &WinDriveProvider::checkDrives);
            return;
        }
    }

    QMap<int, WinDrive *> drives;
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
        WinDrive *currentDrive = new WinDrive(this, diskDrive->name(), diskDrive->size(), !mountable, diskDrive->index(), diskDrive->serialNumber());
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
            emit driveRemoved(m_drives[it.key()]);
            m_drives[it.key()]->deleteLater();
            m_drives.remove(it.key());
        }

        mDebug() << "Adding new drive to the list with index " << it.key();
        m_drives[it.key()] = it.value();
        emit driveConnected(it.value());

        driveIndexes.removeAll(it.key());
    }

    // Remove our previously stored drives that were not present in the last check
    for (int index : driveIndexes) {
        mDebug() << "Removing old drive with index" << index;
        emit driveRemoved(m_drives[index]);
        m_drives[index]->deleteLater();
        m_drives.remove(index);
    }

    QTimer::singleShot(2500, this, &WinDriveProvider::checkDrives);
}

WinDrive::WinDrive(WinDriveProvider *parent, const QString &name, uint64_t size, bool containsLive, int device, const QString &serialNumber)
    : Drive(parent, name, size, containsLive)
    , m_device(device)
    , m_serialNo(serialNumber)
{
}

bool WinDrive::write(ReleaseVariant *data)
{
    mDebug() << this->metaObject()->className() << "Preparing to write" << data->fullName() << "to drive" << m_device;
    if (!Drive::write(data))
        return false;

    // Create new process using unique_ptr
    m_process.reset(new QProcess(this));
    connect(m_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &WinDrive::onFinished);
    connect(m_process.get(), &QProcess::readyRead, this, &WinDrive::onReadyRead);
    connect(qApp, &QCoreApplication::aboutToQuit, m_process.get(), &QProcess::terminate);

    if (data->status() != ReleaseVariant::DOWNLOADING)
        m_image->setStatus(ReleaseVariant::WRITING);

    if (QFile::exists(qApp->applicationDirPath() + "/helper.exe")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper.exe");
    } else if (QFile::exists(qApp->applicationDirPath() + "/../helper.exe")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper.exe");
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
    m_process->setArguments(args);

    mDebug() << this->metaObject()->className() << "Starting" << m_process->program() << args;
    m_process->start();
    return true;
}

void WinDrive::cancel()
{
    Drive::cancel();
}

void WinDrive::restore()
{
    mDebug() << this->metaObject()->className() << "Preparing to restore disk" << m_device;
    
    m_process.reset(new QProcess(this));

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    if (QFile::exists(qApp->applicationDirPath() + "/helper.exe")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper.exe");
    } else if (QFile::exists(qApp->applicationDirPath() + "/../helper.exe")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper.exe");
    } else {
        m_restoreStatus = RESTORE_ERROR;
        return;
    }

    QStringList args;
    args << "restore";
    args << QString("%1").arg(m_device);
    m_process->setArguments(args);

    // connect(m_process.get(), &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process.get(), SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int, QProcess::ExitStatus)));
    connect(qApp, &QCoreApplication::aboutToQuit, m_process.get(), &QProcess::terminate);

    mDebug() << this->metaObject()->className() << "Starting" << m_process->program() << args;

    m_process->start(QIODevice::ReadOnly);
}

bool WinDrive::busy() const
{
    return (m_process && m_process->state() == QProcess::Running);
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

    if (!m_process)
        return;

    mDebug() << "Child finished" << exitCode << exitStatus;
    mDebug() << m_process->errorString();

    if (exitCode == 0) {
        m_image->setStatus(ReleaseVariant::FINISHED);
        Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
    } else if (exitCode == 1) {
        m_image->setErrorString(m_process->readAllStandardError().trimmed());
        m_image->setStatus(ReleaseVariant::FAILED);
    } else if (exitCode == 2) {
        m_image->setErrorString(tr("Writing has been cancelled"));
        m_image->setStatus(ReleaseVariant::FAILED);
    }
}

void WinDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (!m_process) {
        return;
    }

    mCritical() << "Process finished" << exitCode << exitStatus;
    mCritical() << m_process->readAllStandardError();

    if (exitCode == 0) {
        m_restoreStatus = RESTORED;
    } else {
        m_restoreStatus = RESTORE_ERROR;
    }
    emit restoreStatusChanged();
}

void WinDrive::onReadyRead()
{
    if (!m_process) {
        return;
    }

    m_progress->setTo(m_image->size());
    m_progress->setValue(NAN);

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING) {
        m_image->setStatus(ReleaseVariant::WRITING);
    }

    while (m_process->bytesAvailable() > 0) {
        QString line = m_process->readLine().trimmed();
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
            qint64 bytes = line.toLongLong(&ok);
            if (ok) {
                if (bytes < 0) {
                    m_progress->setValue(NAN);
                } else {
                    m_progress->setValue(bytes);
                }
            }
        }
    }
}
