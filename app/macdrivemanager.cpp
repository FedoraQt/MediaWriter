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
    qDebug() << this->metaObject()->className() << "construction";
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
    qDebug() << this->metaObject()->className() << "drive added" << bsdName << vendor << model << size << restoreable;
    removeDrive(bsdName);
    MacDrive *drive = new MacDrive(this, bsdName, QString("%1 %2").arg(vendor).arg(model), size, restoreable);
    m_devices[bsdName] = drive;
    emit driveConnected(drive);
}

void MacDriveProvider::onDriveRemoved(const char *bsdName) {
    MacDriveProvider::instance()->removeDrive(bsdName);
}

void MacDriveProvider::removeDrive(const QString &bsdName) {
    if (m_devices.contains(bsdName)) {
        qDebug() << this->metaObject()->className() << "drive removed" << bsdName;
        emit driveRemoved(m_devices[bsdName]);
        m_devices[bsdName]->deleteLater();
        m_devices.remove(bsdName);
    }
}


MacDrive::MacDrive(DriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, device, name, size, containsLive)
{

}

QString MacDrive::helperBinary() {
    if (QFile::exists(qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper")) {
        return qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper";
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        return qApp->applicationDirPath() + "/helper";
    }
    return "";
}

void MacDrive::prepareProcess(const QString &binary, const QStringList &arguments) {
    if (m_process) {
        // TODO some handling of an already present process
        m_process->deleteLater();
    }
    m_process = new QProcess(this);

    QStringList helperCommand;
    helperCommand << binary << arguments;
    for (QString& argument : helperCommand) {
        argument = "'" + argument.replace("'", "\\'") + "'"
    }
    QString command;
    command.append("do shell script \"");
    command.append(helperCommand.join(" ").replace("\"", "\\\"");
    command.append("\" with administrator privileges without altering line endings");

    /*
     * TODO: Somehow gain privileges differently so that process interaction
     * can be done.
     */
    m_process->setProgram("osascript");
    QStringList args;
    args << "-e" << command;
    m_process->setArguments(args);
}
