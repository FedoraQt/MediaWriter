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

#include "linuxdrivemanager.h"
#include "utilities.h"

#include <QDBusArgument>
#include <QtDBus/QtDBus>

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    mDebug() << this->metaObject()->className() << "construction";
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();

    QTimer::singleShot(0, this, SLOT(delayedConstruct()));
}

void LinuxDriveProvider::delayedConstruct()
{
    m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());

    mDebug() << this->metaObject()->className() << "Calling GetManagedObjects over DBus";
    QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", nullptr, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(onInterfacesAdded(QDBusObjectPath, InterfacesAndProperties)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(onInterfacesRemoved(QDBusObjectPath, QStringList)));
}

QDBusObjectPath LinuxDriveProvider::handleObject(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties)
{
    QRegularExpression numberRE("[0-9]$");
    QRegularExpression mmcRE("[0-9]p[0-9]$");
    QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);

    QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

    const bool isPartition = (numberRE.match(object_path.path()).hasMatch() && !object_path.path().startsWith("/org/freedesktop/UDisks2/block_devices/mmcblk")) || mmcRE.match(object_path.path()).hasMatch();

    if (isPartition) {
        // For partitions, check if they carry an iso9660 filesystem and if so
        // mark the parent drive as containing a live image. This handles the
        // startup case where UDisks2 reports an empty IdType on the whole-disk
        // block device because it has already processed the partition table.
        if (interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660") {
            const QDBusObjectPath blockPath = m_driveToBlock.value(driveId.path());
            if (!blockPath.path().isEmpty() && m_drives.contains(blockPath)) {
                m_drives[blockPath]->setRestoreStatus(Drive::CONTAINS_LIVE);
            }
        }
        return QDBusObjectPath();
    }

    if (!driveId.path().isEmpty() && driveId.path() != "/") {
        bool portable = driveInterface.property("Removable").toBool();
        bool optical = driveInterface.property("Optical").toBool();
        bool containsMedia = driveInterface.property("MediaAvailable").toBool();
        QString connectionBus = driveInterface.property("ConnectionBus").toString().toLower();
        bool isValid = containsMedia && !optical && (portable || connectionBus == "usb");

        QString vendor = driveInterface.property("Vendor").toString();
        QString model = driveInterface.property("Model").toString();
        QString serial = driveInterface.property("Serial").toString();
        uint64_t size = driveInterface.property("Size").toULongLong();
        bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

        QString name;
        if (vendor.isEmpty()) {
            if (model.isEmpty()) {
                name = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();
            } else {
                name = model;
            }
        } else if (model.isEmpty()) {
            name = vendor;
        } else {
            name = QString("%1 %2").arg(vendor).arg(model);
        }

        mDebug() << this->metaObject()->className() << "New drive" << driveId.path() << "-" << name << "(" << size << "bytes;" << (isValid ? "removable;" : "nonremovable;") << connectionBus << ")";

        if (isValid) {
            m_driveToBlock[driveId.path()] = object_path;

            if (m_drives.contains(object_path)) {
                m_drives[object_path]->updateDrive(name, size, isoLayout);
                return object_path;
            }

            Drive *d = new Drive(this, name, object_path.path(), serial, size, isoLayout);
            m_drives[object_path] = d;
            Q_EMIT DriveProvider::driveConnected(d);

            return object_path;
        }
    }
    return QDBusObjectPath();
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w)
{
    mDebug() << this->metaObject()->className() << "Got a reply to GetManagedObjects, parsing";

    QDBusPendingReply<DBusIntrospection> reply = *w;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    const QList<QDBusObjectPath> paths = m_drives.keys();
    QSet<QDBusObjectPath> oldPaths(paths.begin(), paths.end());
#else
    QSet<QDBusObjectPath> oldPaths = m_drives.keys().toSet();
#endif
    QSet<QDBusObjectPath> newPaths;

    if (reply.isError()) {
        mWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        Q_EMIT backendBroken(tr("UDisks2 seems to be unavailable or unaccessible on your system."));
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    QList<QDBusObjectPath> blockDevicePaths;
    for (auto i : introspection.keys()) {
        if (i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            blockDevicePaths.append(i);
    }
    // Sort so whole-disk devices (e.g. sdb) are processed before their
    // partitions (e.g. sdb1, sdb2), ensuring m_driveToBlock is populated
    // before partition iso9660 detection runs in handleObject().
    std::sort(blockDevicePaths.begin(), blockDevicePaths.end(), [](const QDBusObjectPath &a, const QDBusObjectPath &b) {
        return a.path() < b.path();
    });
    for (auto i : blockDevicePaths) {
        QDBusObjectPath path = handleObject(i, introspection[i]);
        if (!path.path().isEmpty()) {
            newPaths.insert(path);
        }
    }

    for (auto i : oldPaths - newPaths) {
        Q_EMIT driveRemoved(m_drives[i]);
        m_drives[i]->deleteLater();
        m_drives.remove(i);
    }

    m_initialized = true;
    Q_EMIT initializedChanged();
}

void LinuxDriveProvider::onInterfacesAdded(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties)
{
    if (interfaces_and_properties.keys().contains("org.freedesktop.UDisks2.Block")) {
        if (!m_drives.contains(object_path)) {
            handleObject(object_path, interfaces_and_properties);
        }
    }
}

void LinuxDriveProvider::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces)
{
    if (interfaces.contains("org.freedesktop.UDisks2.Block")) {
        if (m_drives.contains(object_path)) {
            mDebug() << this->metaObject()->className() << "Drive at" << object_path.path() << "removed";
            Q_EMIT driveRemoved(m_drives[object_path]);
            m_drives[object_path]->deleteLater();
            m_drives.remove(object_path);
            m_driveToBlock.remove(m_driveToBlock.key(object_path));
        }
    }
}

void LinuxDriveProvider::onPropertiesChanged(const QString &interface_name, const QVariantMap &changed_properties, const QStringList &invalidated_properties)
{
    Q_UNUSED(interface_name)
    const QSet<QString> watchedProperties = {"MediaAvailable", "Size"};
    const QList<QString> changedPropertyKeys = changed_properties.keys();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSet<QString> changedPropertiesSet(changedPropertyKeys.begin(), changedPropertyKeys.end());
    QSet<QString> invalidatedPropertiesSet(invalidated_properties.begin(), invalidated_properties.end());
    // not ideal but it works alright without a huge lot of code
    if (!changedPropertiesSet.intersect(watchedProperties).isEmpty() || !invalidatedPropertiesSet.intersect(watchedProperties).isEmpty()) {
#else
    // not ideal but it works alright without a huge lot of code
    if (!changed_properties.keys().toSet().intersect(watchedProperties).isEmpty() || !invalidated_properties.toSet().intersect(watchedProperties).isEmpty()) {
#endif
        QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
        QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

        connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
    }
}
