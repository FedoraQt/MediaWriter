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
    qDebug() << this->metaObject()->className() << "construction";
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();

    QTimer::singleShot(0, this, SLOT(delayedConstruct()));
}

void LinuxDriveProvider::delayedConstruct() {
    m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());

    qDebug() << this->metaObject()->className() << "Calling GetManagedObjects over DBus";
    QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", 0, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(onInterfacesAdded(QDBusObjectPath,InterfacesAndProperties)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList)));
}

QDBusObjectPath LinuxDriveProvider::handleObject(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegExp numberRE("[0-9]$");
    QRegExp mmcRE("[0-9]p[0-9]$");
    QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);

    QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

    if ((numberRE.indexIn(object_path.path()) >= 0 && !object_path.path().startsWith("/org/freedesktop/UDisks2/block_devices/mmcblk")) ||
            mmcRE.indexIn(object_path.path()) >= 0)
        return QDBusObjectPath();

    if (!driveId.path().isEmpty() && driveId.path() != "/") {
        bool portable = driveInterface.property("Removable").toBool();
        bool optical = driveInterface.property("Optical").toBool();
        bool containsMedia = driveInterface.property("MediaAvailable").toBool();
        QString connectionBus = driveInterface.property("ConnectionBus").toString().toLower();
        bool isValid = containsMedia && !optical && (portable || connectionBus == "usb");

        QString vendor = driveInterface.property("Vendor").toString();
        QString model = driveInterface.property("Model").toString();
        uint64_t size = driveInterface.property("Size").toULongLong();
        bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

        QString name;
        if (vendor.isEmpty())
            if (model.isEmpty())
                name = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();
            else
                name = model;
        else
            if (model.isEmpty())
                name = vendor;
            else
                name = QString("%1 %2").arg(vendor).arg(model);

        qDebug() << this->metaObject()->className() << "New drive" << driveId.path() << "-" << name << "(" << size << "bytes;" << (isValid ? "removable;" : "nonremovable;") << connectionBus << ")";

        if (isValid) {

            // TODO find out why do I do this
            if (m_drives.contains(object_path)) {
                LinuxDrive *tmp = m_drives[object_path];
                emit DriveProvider::driveRemoved(tmp);
            }
            LinuxDrive *d = new LinuxDrive(this, object_path.path(), name, size, isoLayout);
            m_drives[object_path] = d;
            emit DriveProvider::driveConnected(d);

            return object_path;
        }
    }
    return QDBusObjectPath();
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w) {
    qDebug() << this->metaObject()->className() << "Got a reply to GetManagedObjects, parsing";

    QDBusPendingReply<DBusIntrospection> reply = *w;
    QSet<QDBusObjectPath> oldPaths = m_drives.keys().toSet();
    QSet<QDBusObjectPath> newPaths;

    if (reply.isError()) {
        qWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        emit backendBroken(tr("UDisks2 seems to be unavailable or unaccessible on your system."));
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            continue;

        QDBusObjectPath path = handleObject(i, introspection[i]);
        if (!path.path().isEmpty())
            newPaths.insert(path);
    }

    for (auto i : oldPaths - newPaths) {
        emit driveRemoved(m_drives[i]);
        m_drives[i]->deleteLater();
        m_drives.remove(i);
    }
}

void LinuxDriveProvider::onInterfacesAdded(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegExp numberRE("[0-9]$");

    if (interfaces_and_properties.keys().contains("org.freedesktop.UDisks2.Block")) {
        if (!m_drives.contains(object_path)) {
            handleObject(object_path, interfaces_and_properties);
        }
    }
}

void LinuxDriveProvider::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces) {
    if (interfaces.contains("org.freedesktop.UDisks2.Block")) {
        if (m_drives.contains(object_path)) {
            qDebug() << this->metaObject()->className() << "Drive at" << object_path.path() << "removed";
            emit driveRemoved(m_drives[object_path]);
            m_drives[object_path]->deleteLater();
            m_drives.remove(object_path);
        }
    }
}

void LinuxDriveProvider::onPropertiesChanged(const QString &interface_name, const QVariantMap &changed_properties, const QStringList &invalidated_properties) {
    Q_UNUSED(interface_name)
    const QSet<QString> watchedProperties = { "MediaAvailable", "Size" };

    // not ideal but it works alright without a huge lot of code
    if (!changed_properties.keys().toSet().intersect(watchedProperties).isEmpty() ||
            !invalidated_properties.toSet().intersect(watchedProperties).isEmpty()) {
        QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
        QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

        connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
    }
}

LinuxDrive::LinuxDrive(DriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, device, name, size, containsLive)
{

}

LinuxDrive::~LinuxDrive() {
    if (m_image && m_image->status() == ReleaseVariant::WRITING) {
        m_image->setErrorString(tr("The drive was removed while it was written to."));
        m_image->setStatus(ReleaseVariant::FAILED);
    }
}

QString LinuxDrive::helperBinary() {
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper")) {
        return qApp->applicationDirPath() + "/../helper/linux/helper";
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        return qApp->applicationDirPath() + "/helper";
    }
    else if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper"))) {
        return QString("%1/%2").arg(LIBEXECDIR).arg("helper");
    }
    return "";
}
