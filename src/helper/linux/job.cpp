/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

#include "job.h"

#include <QDBusArgument>
#include <QDBusInterface>
#include <QtDBus>

Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

Job::Job(const QString &where)
    : Job(QString(), where)
{
}

Job::Job(const QString &what, const QString &where)
    : what(what)
    , where(where)
{
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();
}

QDBusUnixFileDescriptor Job::getDescriptor()
{
    QDBusInterface device("org.freedesktop.UDisks2", where, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QString drivePath = qvariant_cast<QDBusObjectPath>(device.property("Drive")).path();
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
    } else {
        err << message.errorMessage();
        err.flush();
        qApp->exit(2);
        return QDBusUnixFileDescriptor(-1);
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = device.call(QDBus::Block, "OpenDevice", "rw", Properties{{"flags", O_DIRECT | O_SYNC | O_CLOEXEC}});
    QDBusUnixFileDescriptor fd = reply.value();

    if (!fd.isValid()) {
        err << reply.error().message();
        err.flush();
        qApp->exit(2);
        return QDBusUnixFileDescriptor(-1);
    }

    return fd;
}
