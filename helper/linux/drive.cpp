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
#include <fcntl.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QObject>
#include <QPair>
#include <QtDBus>
#include <QtGlobal>

#include "error.h"

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

Drive::Drive(const QString &identifier)
    : m_fileDescriptor(QDBusUnixFileDescriptor(-1)), m_identifier(identifier),
      m_device(new QDBusInterface("org.freedesktop.UDisks2", m_identifier, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus())),
      m_path(qvariant_cast<QDBusObjectPath>(m_device->property("Drive")).path()) {
}

Drive::~Drive() {
}

void Drive::init() {
    /**
     * Simply try to use OpenDevice which is introduced in udisks 2.7.3 and
     * otherwise use open directly which requires privileges.
     */
    QDBusReply<QDBusUnixFileDescriptor> reply = m_device->call(QDBus::Block, "OpenDevice", "rw", Properties{});
    m_fileDescriptor = reply.value();
    if (m_fileDescriptor.isValid())
        return;
    m_fileDescriptor = QDBusUnixFileDescriptor(-1);

    int fd = ::open(qPrintable(m_device->property("Device").toString()), O_RDWR);
    if (fd < 0) {
        throw HelperError(Error::DRIVE_USE, drive());
    }
    m_fileDescriptor = QDBusUnixFileDescriptor(fd);
}

/**
 * Write buffer directly to drive.
 */
void Drive::write(const void *buffer, std::size_t size) {
    int fd = getDescriptor();
    if (static_cast<std::size_t>(::write(fd, buffer, size)) != size) {
        throw HelperError(Error::DRIVE_WRITE, drive());
    }
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() const {
    return m_fileDescriptor.fileDescriptor();
}

QString Drive::drive() const {
    return m_identifier;
}

void Drive::wipe() {
    QDBusReply<void> formatReply = m_device->call("Format", "dos", Properties());
    if (!formatReply.isValid() && formatReply.error().type() != QDBusError::NoReply) {
        throw HelperError(Error::DRIVE_WRITE, drive(), formatReply.error().message());
    }
    QDBusInterface partitionTable("org.freedesktop.UDisks2", m_identifier, "org.freedesktop.UDisks2.PartitionTable", QDBusConnection::systemBus());
    QDBusReply<QDBusObjectPath> reply = partitionTable.call("CreatePartitionAndFormat", 0ULL, m_device->property("Size").toULongLong(), "0xb", "", Properties{}, "vfat", Properties{});
    if (!reply.isValid()) {
        throw HelperError(Error::DRIVE_WRITE, drive(), reply.error().message());
    }
}

/**
 * Unmount all partitions managed by udisks.
 */
void Drive::umount() {
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");
    if (message.arguments().length() != 1)
        return;
    QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
    DBusIntrospection objects;
    arg >> objects;
    for (auto i : objects.keys()) {
        if (!objects[i].contains("org.freedesktop.UDisks2.Filesystem"))
            continue;
        QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
        if (currentDrivePath == m_path) {
            QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
            message = partition.call("Unmount", Properties{ { "force", true } });
        }
    }
}
