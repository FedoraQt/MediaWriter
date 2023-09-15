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

#include "restorejob.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QtDBus>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr)
    , where(where)
{
    QTimer::singleShot(0, this, SLOT(work()));
}

void RestoreJob::work()
{
    QDBusInterface device("org.freedesktop.UDisks2", where, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QString drivePath = qvariant_cast<QDBusObjectPath>(device.property("Drive")).path();
    QDBusInterface drive("org.freedesktop.UDisks2", drivePath, "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus(), this);
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");

    if (message.arguments().length() == 1) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
        DBusIntrospection objects;
        arg >> objects;
        for (auto i : objects.keys()) {
            if (objects[i].contains("org.freedesktop.UDisks2.Filesystem")) {
                QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
                if (currentDrivePath == drivePath) {
                    QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
                    message = partition.call("Unmount", Properties{{"force", true}});
                }
            }
        }
    }

    QDBusReply<void> formatReply = device.call("Format", "dos", Properties());
    if (!formatReply.isValid() && formatReply.error().type() != QDBusError::NoReply) {
        err << formatReply.error().message() << "\n";
        err.flush();
        qApp->exit(1);
        return;
    }

    QDBusInterface partitionTable("org.freedesktop.UDisks2", where, "org.freedesktop.UDisks2.PartitionTable", QDBusConnection::systemBus(), this);
    QDBusReply<QDBusObjectPath> partitionReply = partitionTable.call("CreatePartition", 1ULL, 0ULL, "", "", Properties());
    if (!partitionReply.isValid()) {
        err << partitionReply.error().message();
        err.flush();
        qApp->exit(2);
        return;
    }
    QString partitionPath = partitionReply.value().path();
    QDBusInterface partition("org.freedesktop.UDisks2", partitionPath, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QDBusReply<void> formatPartitionReply = partition.call("Format", "exfat", Properties{{"update-partition-type", true}});
    if (!formatPartitionReply.isValid() && formatPartitionReply.error().type() != QDBusError::NoReply) {
        err << formatPartitionReply.error().message() << "\n";
        err.flush();
        qApp->exit(3);
        return;
    }
    err.flush();

    qApp->exit(0);
}
