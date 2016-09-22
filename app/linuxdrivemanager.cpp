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

#include "linuxdrivemanager.h"

#include <QtDBus/QtDBus>
#include <QDBusArgument>

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();

    QTimer::singleShot(0, this, &LinuxDriveProvider::delayedConstruct);
}

void LinuxDriveProvider::delayedConstruct() {
    m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());

    QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager" ,"InterfacesAdded", this, SLOT(onInterfacesAdded(QDBusObjectPath,InterfacesAndProperties)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager" ,"InterfacesRemoved", this, SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList)));
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w) {
    QDBusPendingReply<DBusIntrospection> reply = *w;

    if (reply.isError()) {
        qWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            continue;

        if (introspection[i].contains("org.freedesktop.UDisks2.Partition"))
            continue;

        QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(introspection[i]["org.freedesktop.UDisks2.Block"]["Drive"]);
        QString devicePath = driveId.path();

        if (!driveId.path().isEmpty() && driveId.path() != "/") {
            bool portable = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Removable"].toBool();
            QStringList mediaCompatibility = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["MediaCompatibility"].toStringList();
            // usb drives should support no inserted media
            if (portable && (mediaCompatibility.isEmpty() || mediaCompatibility.contains("thumb"))) {
                QString vendor = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Vendor"].toString();
                QString model = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Model"].toString();
                uint64_t size = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Size"].toULongLong();
                bool isoLayout = introspection[i]["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

                LinuxDrive *d = new LinuxDrive(this, i.path(), QString("%1 %2").arg(vendor).arg(model), size, isoLayout);
                if (m_drives.contains(i)) {
                    LinuxDrive *tmp = m_drives[i];
                    emit DriveProvider::driveRemoved(tmp);
                }
                m_drives[i] = d;
                emit DriveProvider::driveConnected(d);
            }
        }
    }
}

void LinuxDriveProvider::onInterfacesAdded(QDBusObjectPath object_path, InterfacesAndProperties interfaces_and_properties) {
    QRegExp r("part[0-9]+$");

    if (interfaces_and_properties.keys().contains("org.freedesktop.UDisks2.Block")) {
        if (!m_drives.contains(object_path)) {
            QString deviceId = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Id"].toString();
            QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);
            QString devicePath = driveId.path();

            QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

            if (!deviceId.isEmpty() && r.indexIn(deviceId) < 0 && !driveId.path().isEmpty() && driveId.path() != "/") {
                bool portable = driveInterface.property("Removable").toBool();
                QStringList mediaCompatibility = driveInterface.property("MediaCompatibility").toStringList();

                if (portable && (mediaCompatibility.isEmpty() || mediaCompatibility.contains("thumb"))) {
                    QString vendor = driveInterface.property("Vendor").toString();
                    QString model = driveInterface.property("Model").toString();
                    uint64_t size = driveInterface.property("Size").toULongLong();
                    bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

                    LinuxDrive *d = new LinuxDrive(this, object_path.path(), QString("%1 %2").arg(vendor).arg(model), size, isoLayout);
                    if (m_drives.contains(object_path)) {
                        LinuxDrive *tmp = m_drives[object_path];
                        emit DriveProvider::driveRemoved(tmp);
                    }
                    m_drives[object_path] = d;
                    emit DriveProvider::driveConnected(d);
                }
            }
        }
    }
}

void LinuxDriveProvider::onInterfacesRemoved(QDBusObjectPath object_path, QStringList interfaces) {
    if (interfaces.contains("org.freedesktop.UDisks2.Block")) {
        if (m_drives.contains(object_path)) {
            emit driveRemoved(m_drives[object_path]);
            m_drives[object_path]->deleteLater();
            m_drives.remove(object_path);
        }
    }
}


LinuxDrive::LinuxDrive(LinuxDriveProvider *parent, QString device, QString name, uint64_t size, bool isoLayout)
    : Drive(parent, name, size, isoLayout), m_device(device) {
}

void LinuxDrive::write(ReleaseVariant *data) {
    Drive::write(data);

    m_image->setStatus(ReleaseVariant::WRITING);

    if (!m_process)
        m_process = new QProcess(this);

    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper/linux/helper");
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper");
    }
    else if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper"))) {
        m_process->setProgram(QString("%1/%2").arg(LIBEXECDIR).arg("helper"));
    }
    else {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        data->setStatus(ReleaseVariant::FAILED);
        return;
    }
    args << "write";
    args << data->iso();
    args << m_device;
    m_process->setArguments(args);

    connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    connect(m_process, &QProcess::errorOccurred, this, &LinuxDrive::onErrorOccurred);

    m_progress->setTo(data->size());
    m_progress->setValue(0.0/0.0);
    m_process->start(QIODevice::ReadOnly);
}

void LinuxDrive::cancel() {
    if (m_process) {
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void LinuxDrive::restore() {
    if (!m_process)
        m_process = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper/linux/helper");
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper");
    }
    else if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper"))) {
        m_process->setProgram(QString("%1/%2").arg(LIBEXECDIR).arg("helper"));
    }
    else {
        qWarning() << "Couldn't find the helper binary.";
        setRestoreStatus(RESTORE_ERROR);
        return;
    }
    args << "restore";
    args << m_device;
    m_process->setArguments(args);

    connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int,QProcess::ExitStatus)));

    m_process->start(QIODevice::ReadOnly);
}

void LinuxDrive::onReadyRead() {
    while (m_process->bytesAvailable() > 0) {
        QString line = m_process->readLine().trimmed();
        bool ok = false;
        qreal val = line.toULongLong(&ok);
        if (ok && val > 0.0)
            m_progress->setValue(val);
    }
}

void LinuxDrive::onFinished(int exitCode, QProcess::ExitStatus status) {
    if (!m_process)
        return;

    if (exitCode != 0) {
        m_image->setErrorString(m_process->readAllStandardError());
        m_image->setStatus(ReleaseVariant::FAILED);
    }
    else {
        m_image->setStatus(ReleaseVariant::FINISHED);
    }
}

void LinuxDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus status) {
    if (exitCode != 0) {
        m_restoreStatus = RESTORE_ERROR;
    }
    else {
        m_restoreStatus = RESTORED;
    }
    emit restoreStatusChanged();
}

void LinuxDrive::onErrorOccurred(QProcess::ProcessError e) {
    if (!m_process)
        return;

    m_image->setErrorString(m_process->errorString());
    m_process->deleteLater();
    m_image->setStatus(ReleaseVariant::FAILED);
}
