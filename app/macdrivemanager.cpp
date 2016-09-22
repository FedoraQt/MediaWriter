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
#include <QDir>

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
    //osascript -e "do shell script \"$*\" with administrator privileges"
    Drive::write(data);

    if (m_child) {
        // TODO some handling of an already present process
        m_child->deleteLater();
    }
    m_child = new QProcess(this);
    connect(m_child, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &MacDrive::onFinished);
    connect(m_child, &QProcess::readyRead, this, &MacDrive::onReadyRead);

    m_child->setProgram("osascript");

    QString command;
    command.append("do shell script \"");
    if (QFile::exists(qApp->applicationDirPath() + "/../../../helper.app/Contents/MacOS/helper")) {
        command.append(QString("'%1/helper'").arg(qApp->applicationDirPath()));
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        command.append(QString("'%1/helper'").arg(qApp->applicationDirPath()));
    }
    else {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        return;
    }
    command.append(" write ");
    command.append(QString("'%1'").arg(data->iso()));
    command.append(" ");
    command.append(m_bsdDevice);
    command.append("\" with administrator privileges without altering line endings");

    QStringList args;
    args << "-e";
    args << command;
    qCritical() << "The command is" << command;
    m_child->setArguments(args);

    m_progress->setTo(data->size());
    m_image->setStatus(ReleaseVariant::WRITING);

    m_progress->setValue(0.0/0.0);
    m_child->start();
}

void MacDrive::cancel() {
    if (m_child) {
        m_child->kill();
        m_child->deleteLater();
        m_child = nullptr;
    }
}

void MacDrive::restore() {
    qCritical() << "starting to restore";
    if (m_child)
        m_child->deleteLater();

    m_child = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    m_child->setProgram("osascript");

    QString command;
    command.append("do shell script \"");
    if (QFile::exists(qApp->applicationDirPath() + "/../../../helper.app/Contents/MacOS/helper")) {
        command.append(QString("'%1/helper'").arg(qApp->applicationDirPath()));
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        command.append(QString("'%1/helper'").arg(qApp->applicationDirPath()));
    }
    else {
        m_restoreStatus = RESTORE_ERROR;
        return;
    }
    command.append(" restore ");
    command.append(m_bsdDevice);
    command.append("\" with administrator privileges without altering line endings");

    QStringList args;
    args << "-e";
    args << command;
    qCritical() << "The command is" << command;
    m_child->setArguments(args);

    //connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_child, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int,QProcess::ExitStatus)));

    m_child->start(QIODevice::ReadOnly);
}

void MacDrive::onFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_progress->setValue(m_image->size());
    m_image->setStatus(ReleaseVariant::FINISHED);
}

void MacDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qCritical() << "Process finished" << exitCode << exitStatus;
    qCritical() << m_child->readAllStandardError();

    if (exitCode == 0)
        m_restoreStatus = RESTORED;
    else
        m_restoreStatus = RESTORE_ERROR;
    emit restoreStatusChanged();

    m_child->deleteLater();
    m_child = nullptr;
}

void MacDrive::onReadyRead() {
    while (m_child->bytesAvailable() > 0) {
        bool ok;
        int64_t bytes = m_child->readLine().trimmed().toULongLong(&ok);
        if (ok)
            m_progress->setValue(bytes);
    }
}
