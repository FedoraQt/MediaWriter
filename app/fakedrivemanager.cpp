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

#include "fakedrivemanager.h"
#include "notifications.h"

#include <QTimer>

FakeDriveProvider::FakeDriveProvider(DriveManager *parent)
    : DriveProvider(parent) {
    QTimer::singleShot(0, this, SLOT(connectDrives()));
}

void FakeDriveProvider::createNewRestoreable() {
    static uint64_t size = 4444444444;
    emit driveConnected(new FakeDrive(this, "Contains Live", size, true));
    size++;
}

void FakeDriveProvider::createNewGetsDisconnected() {
    emit driveConnected(new FakeDrive(this, "Gets Disconnected", 1000000000, false));
}

void FakeDriveProvider::connectDrives() {
    emit driveConnected(new FakeDrive(this, "Okay", 12345678900, false));
    emit driveConnected(new FakeDrive(this, "Fails", 9876543210, false));
    emit driveConnected(new FakeDrive(this, "Not Large Enough", 10000, false));
    emit driveConnected(new FakeDrive(this, "Gets Disconnected", 10000000000, false));
    QTimer::singleShot(2000, this, SLOT(createNewRestoreable()));
}

FakeDrive::FakeDrive(FakeDriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, "", name, size, containsLive) {
    m_progress->setTo(size);
}

bool FakeDrive::write(ReleaseVariant *data) {
    m_image = data;
    m_image->setErrorString(QString());

    if (data && data->size() > 0 && size() > 0 && data->realSize() > size()) {
        m_image->setErrorString(tr("This drive is not large enough."));
        return false;
    }

    m_progress->setValue(0);
    m_image->setStatus(ReleaseVariant::WRITING);
    QTimer::singleShot(100, this, SLOT(writingAdvanced()));

    return true;
}

void FakeDrive::cancel() {

}

void FakeDrive::restore() {
    m_restoreStatus = Drive::RESTORING;
    emit restoreStatusChanged();
    QTimer::singleShot(5000, this, SLOT(restoringFinished()));
}

void FakeDrive::writingAdvanced() {
    m_progress->setValue(m_progress->value() + 123456789);
    if (m_progress->value() >= m_size) {
        m_image->setStatus(ReleaseVariant::FINISHED);
        Notifications::notify("Success", "Yes!");
    }
    else if (m_name == "Fails" && m_progress->value() >= m_size / 2) {
        m_image->setStatus(ReleaseVariant::FAILED);
        m_image->setErrorString("Some error string.");
        Notifications::notify("Failed", "FAILED");
    }
    else if (m_name == "Gets Disconnected" && m_progress->value() >= m_size / 2) {
        emit qobject_cast<FakeDriveProvider*>(parent())->driveRemoved(this);
        QTimer::singleShot(5000, qobject_cast<FakeDriveProvider*>(parent()), SLOT(createNewGetsDisconnected()));
        this->deleteLater();
        m_image->setStatus(ReleaseVariant::FAILED);
        m_image->setErrorString(QString("Drive %1 got disconnected.").arg(name()));
        Notifications::notify("Failed", "FAILED");
    }
    else {
        QTimer::singleShot(100, this, SLOT(writingAdvanced()));
    }
}

void FakeDrive::restoringFinished() {
    if (m_size % 2)
        m_restoreStatus = Drive::RESTORE_ERROR;
    else
        m_restoreStatus = Drive::RESTORED;
    emit restoreStatusChanged();

    QTimer::singleShot(5000, this, SLOT(selfdestruct()));
}

void FakeDrive::selfdestruct() {
    emit qobject_cast<FakeDriveProvider*>(parent())->driveRemoved(this);
    QTimer::singleShot(2000, qobject_cast<FakeDriveProvider*>(parent()), SLOT(createNewRestoreable()));
    this->deleteLater();
}
