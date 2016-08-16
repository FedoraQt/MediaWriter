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

#include "macdrivemanager.h"
#include "macdrivearbiter.h"

#include <QDebug>

MacDriveProvider *MacDriveProvider::_self = nullptr;

MacDriveProvider::MacDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    _self = this;
    startArbiter(&MacDriveProvider::onDriveAdded, &MacDriveProvider::onDriveRemoved);
}

MacDriveProvider::~MacDriveProvider() {
    stopArbiter();
}

MacDriveProvider *MacDriveProvider::instance() {
    return _self;
}

void MacDriveProvider::onDriveAdded(const char *bsdName, const char *vendor, const char *model, unsigned long long size, bool restoreable) {
    MacDriveProvider::instance()->addDrive(bsdName, vendor, model, size, restoreable);
}

void MacDriveProvider::addDrive(const QString &bsdName, const QString &vendor, const QString &model, uint64_t size, bool restoreable) {
    removeDrive(bsdName);
    MacDrive *drive = new MacDrive(this, QString("%1 %2").arg(vendor).arg(model), size, restoreable, bsdName);
    m_devices[bsdName] = drive;
    emit driveConnected(drive);
}

void MacDriveProvider::onDriveRemoved(const char *bsdName) {
    MacDriveProvider::instance()->removeDrive(bsdName);
}

void MacDriveProvider::removeDrive(const QString &bsdName) {
    if (m_devices.contains(bsdName)) {
        emit driveRemoved(m_devices[bsdName]);
        m_devices[bsdName]->deleteLater();
        m_devices.remove(bsdName);
    }
}


MacDrive::MacDrive(DriveProvider *parent, const QString &name, uint64_t size, bool containsLive, const QString &bsdDevice)
    : Drive(parent, name, size, containsLive), m_bsdDevice(bsdDevice)
{

}

void MacDrive::write(ReleaseVariant *data) {

}

void MacDrive::restore() {

}
