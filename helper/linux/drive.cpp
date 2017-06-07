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

#include "drive.h"

#include <unistd.h>

#include <algorithm>
#include <memory>

#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QtDBus>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

Drive::Drive(const QString &identifier)
    : err(stderr), fileDescriptor(QDBusUnixFileDescriptor(-1)), identifier(identifier),
      device(std::move(std::unique_ptr<QDBusInterface>(new QDBusInterface("org.freedesktop.UDisks2", identifier, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus())))),
      path(qvariant_cast<QDBusObjectPath>(device->property("Drive")).path()),
      drive(std::move(std::unique_ptr<QDBusInterface>(new QDBusInterface("org.freedesktop.UDisks2", path, "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus())))) {
}

/**
 * Open drive for writing.
 */
int Drive::open() {
    QDBusReply<QDBusUnixFileDescriptor> reply = device->callWithArgumentList(QDBus::Block, "OpenForBenchmark", { Properties{ { "writable", true } } });
    fileDescriptor = reply.value();
    if (!fileDescriptor.isValid()) {
        err << reply.error().message();
        err.flush();
        fileDescriptor = QDBusUnixFileDescriptor(-1);
        return 2;
    }
    return 0;
}

/**
 * Close drive for writing.
 */
int Drive::close() {
    fileDescriptor = QDBusUnixFileDescriptor(-1);
    return 0;
}

/**
 * Write buffer directly to drive.
 */
bool Drive::write(const void *buffer, std::size_t size) {
    int fd = getDescriptor();
    return static_cast<std::size_t>(::write(fd, buffer, size)) == size;
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() {
    return fileDescriptor.fileDescriptor();
}

/**
 * Create a new dos label on the partition. This essentially wipes all
 * existing information about partitions.
 */
int Drive::wipe() {
    QDBusReply<void> formatReply = device->call("Format", "dos", Properties());
    if (!formatReply.isValid() && formatReply.error().type() != QDBusError::NoReply) {
        err << formatReply.error().message() << "\n";
        err.flush();
        return 1;
    }
    return 0;
}

/**
 * Fill the rest of the drive with a primary partition that uses the fat
 * filesystem.
 */
int Drive::addPartition(const QString &label) {
    QDBusInterface partitionTable("org.freedesktop.UDisks2", identifier, "org.freedesktop.UDisks2.PartitionTable", QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> partitionReply = partitionTable.call("CreatePartition", 0ULL, device->property("Size").toULongLong(), "", "", Properties());
    if (!partitionReply.isValid()) {
        err << partitionReply.error().message();
        err.flush();
        return 2;
    }
    QString partitionPath = partitionReply.value().path();
    QDBusInterface partition("org.freedesktop.UDisks2", partitionPath, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    QDBusReply<void> formatPartitionReply = partition.call("Format", "vfat", Properties{ { "update-partition-type", true }, { "label", label } });
    if (!formatPartitionReply.isValid() && formatPartitionReply.error().type() != QDBusError::NoReply) {
        err << formatPartitionReply.error().message() << "\n";
        err.flush();
        return 3;
    }
    return 0;
}

/**
 * Mount specified partition.
 */
QString Drive::mount(const QString &partitionIdentifier) {
    QDBusInterface partition("org.freedesktop.UDisks2", partitionIdentifier, "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
    QDBusMessage reply = partition.call("Mount", Properties{});
    return reply.arguments().first().toString();
}

/**
 * Unmount all partitions managed by udisks.
 */
int Drive::umount() {
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");

    if (message.arguments().length() == 1) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
        DBusIntrospection objects;
        arg >> objects;
        for (auto i : objects.keys()) {
            if (objects[i].contains("org.freedesktop.UDisks2.Filesystem")) {
                QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
                if (currentDrivePath == path) {
                    QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
                    message = partition.call("Unmount", Properties{ { "force", true } });
                }
            }
        }
    }
    return 0;
}
