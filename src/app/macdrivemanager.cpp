/*
 * Fedora Media Writer
 * Copyright (C) 2026 Jan Grulich <jgrulich@redhat.com
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

#include "macdrivemanager.h"
#include "macdrivearbiter.h"

#include <QDebug>
#include <QDir>
#include <QProcess>

MacDriveProvider *MacDriveProvider::_self = nullptr;

MacDriveProvider::MacDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    mDebug() << this->metaObject()->className() << "construction";
    _self = this;
    startArbiter(&MacDriveProvider::onDriveAdded, &MacDriveProvider::onDriveRemoved);
}

MacDriveProvider::~MacDriveProvider()
{
    stopArbiter();
}

MacDriveProvider *MacDriveProvider::instance()
{
    return _self;
}

void MacDriveProvider::onDriveAdded(const char *bsdName, const char *vendor, const char *model, unsigned long long size, bool restoreable)
{
    MacDriveProvider::instance()->addDrive(bsdName, vendor, model, size, restoreable);
}

void MacDriveProvider::addDrive(const QString &bsdName, const QString &vendor, const QString &model, uint64_t size, bool restoreable)
{
    mDebug() << this->metaObject()->className() << "drive added" << bsdName << vendor << model << size << restoreable;
    removeDrive(bsdName);
    Drive *drive = new Drive(this, QString("%1 %2").arg(vendor).arg(model), bsdName, QString(), size, restoreable);
    m_devices[bsdName] = drive;
    Q_EMIT driveConnected(drive);

    if (restoreable)
        return;

    QProcess *diskUtil = new QProcess(this);
    diskUtil->setProgram("diskutil");
    diskUtil->setArguments({"list", "-plist", QString("/dev/%1").arg(bsdName)});
    connect(diskUtil, &QProcess::finished, this, [this, bsdName, drive, diskUtil](int exitCode) {
        diskUtil->deleteLater();
        if (exitCode != 0 || !m_devices.contains(bsdName) || m_devices[bsdName] != drive)
            return;
        const QString output = QString::fromUtf8(diskUtil->readAllStandardOutput());
        if (output.contains(QStringLiteral("<string>EFI</string>")) || output.contains(QStringLiteral("<string>0xEF</string>"))) {
            mDebug() << this->metaObject()->className() << "drive" << bsdName << "contains a live image";
            drive->setRestoreStatus(Drive::CONTAINS_LIVE);
        }
    });
    diskUtil->start(QIODevice::ReadOnly);
}

void MacDriveProvider::onDriveRemoved(const char *bsdName)
{
    MacDriveProvider::instance()->removeDrive(bsdName);
}

void MacDriveProvider::removeDrive(const QString &bsdName)
{
    if (m_devices.contains(bsdName)) {
        mDebug() << this->metaObject()->className() << "drive removed" << bsdName;
        Q_EMIT driveRemoved(m_devices[bsdName]);
        m_devices[bsdName]->deleteLater();
        m_devices.remove(bsdName);
    }
}
