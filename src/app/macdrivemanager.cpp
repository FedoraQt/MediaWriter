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
#include "notifications.h"

#include <QDebug>
#include <QDir>

MacDriveProvider *MacDriveProvider::_self = nullptr;

MacDriveProvider::MacDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    mDebug() << this->metaObject()->className() << "construction";
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
    mDebug() << this->metaObject()->className() << "drive added" << bsdName << vendor << model << size << restoreable;
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
        mDebug() << this->metaObject()->className() << "drive removed" << bsdName;
        emit driveRemoved(m_devices[bsdName]);
        m_devices[bsdName]->deleteLater();
        m_devices.remove(bsdName);
    }
}


MacDrive::MacDrive(DriveProvider *parent, const QString &name, uint64_t size, bool containsLive, const QString &bsdDevice)
    : Drive(parent, name, size, containsLive), m_bsdDevice(bsdDevice)
{

}

bool MacDrive::write(ReleaseVariant *data) {
    if (!Drive::write(data))
        return false;

    if (m_image->status() == ReleaseVariant::READY || m_image->status() == ReleaseVariant::FAILED ||
            m_image->status() == ReleaseVariant::FAILED_VERIFICATION || m_image->status() == ReleaseVariant::FINISHED)
        m_image->setStatus(ReleaseVariant::WRITING);

    if (m_child) {
        // TODO some handling of an already present process
        m_child->deleteLater();
    }
    m_child = new QProcess(this);
    connect(m_child, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MacDrive::onFinished);
    connect(m_child, &QProcess::readyRead, this, &MacDrive::onReadyRead);

    if (QFile::exists(qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper")) {
        m_child->setProgram(QString("%1/../../../../helper/mac/helper.app/Contents/MacOS/helper").arg(qApp->applicationDirPath()));
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_child->setProgram(QString("%1/helper").arg(qApp->applicationDirPath()));
    }
    else {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        setDelayedWrite(false);
        return false;
    }
    QStringList args;
    args.append("write");
    if (data->status() == ReleaseVariant::WRITING) {
        args.append(QString("%1").arg(data->iso()));
    }
    else {
        args.append(QString("%1").arg(data->temporaryPath()));
    }
    args.append(m_bsdDevice);

    mCritical() << "The command is" << m_child->program() << args;
    m_child->setArguments(args);

    m_child->start();

    return true;
}

void MacDrive::cancel() {
    Drive::cancel();
    if (m_child) {
        m_child->kill();
        m_child->deleteLater();
        m_child = nullptr;
    }
}

void MacDrive::restore() {
    mCritical() << "starting to restore";
    if (m_child)
        m_child->deleteLater();

    m_child = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    if (QFile::exists(qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper")) {
        m_child->setProgram(QString("%1/../../../../helper/mac/helper.app/Contents/MacOS/helper").arg(qApp->applicationDirPath()));
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_child->setProgram(QString("%1/helper").arg(qApp->applicationDirPath()));
    }
    else {
        m_restoreStatus = RESTORE_ERROR;
        return;
    }

    QStringList args;
    args.append("restore");
    args.append(m_bsdDevice);
    m_child->setArguments(args);

    m_child->setProcessChannelMode(QProcess::ForwardedChannels);

    connect(m_child, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MacDrive::onRestoreFinished);

    mCritical() << "The command is" << m_child->program() << args;

    m_child->start();
}

void MacDrive::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    Q_UNUSED(exitStatus)

    setDelayedWrite(false);

    if (!m_child)
        return;

    if (exitCode != 0) {
        QString output = m_child->readAllStandardError();
        QRegularExpression re("^.+:.+: ");
        QStringList lines = output.split('\n');
        if (lines.length() > 0) {
            QString line = lines.first().replace(re, "");
            m_image->setErrorString(line);
        }
        Notifications::notify(tr("Error"), tr("Writing %1 failed").arg(m_image->fullName()));
        m_image->setStatus(ReleaseVariant::FAILED);
    }
    else {
        Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
        m_image->setStatus(ReleaseVariant::FINISHED);
    }
}

void MacDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (!m_child)
        return;

    mCritical() << "Process finished" << exitCode << exitStatus;
    mCritical() << m_child->readAllStandardError();

    if (exitCode == 0)
        m_restoreStatus = RESTORED;
    else
        m_restoreStatus = RESTORE_ERROR;
    emit restoreStatusChanged();

    m_child->deleteLater();
    m_child = nullptr;
}

void MacDrive::onReadyRead() {
    if (!m_child)
        return;

    if (m_image->status() == ReleaseVariant::WRITING) {
        m_progress->setTo(m_image->size());
        m_progress->setValue(0.0/0.0);
    }

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING)
        m_image->setStatus(ReleaseVariant::WRITING);

    while (m_child->bytesAvailable() > 0) {
        bool ok;
        int64_t bytes = m_child->readLine().trimmed().toULongLong(&ok);
        if (ok)
            m_progress->setValue(bytes);
    }
}
