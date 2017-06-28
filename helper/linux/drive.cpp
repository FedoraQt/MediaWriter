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
#include <stdexcept>
#include <utility>

#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QObject>
#include <QPair>
#include <QtDBus>
#include <QtGlobal>

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

/**
 * Open drive for writing.
 */
void Drive::open() {
    if (getDescriptor() != -1)
        return;
    QDBusReply<QDBusUnixFileDescriptor> reply = m_device->callWithArgumentList(QDBus::Block, "OpenForBenchmark", { Properties{ { "writable", true } } });
    m_fileDescriptor = reply.value();
    if (!m_fileDescriptor.isValid()) {
        throw std::runtime_error(reply.error().message().toStdString());
        m_fileDescriptor = QDBusUnixFileDescriptor(-1);
    }
}

/**
 * Close drive for writing.
 */
void Drive::close() {
    m_fileDescriptor = QDBusUnixFileDescriptor(-1);
}

/**
 * Write buffer directly to drive.
 */
void Drive::write(const void *buffer, std::size_t size) {
    int fd = getDescriptor();
    if (static_cast<std::size_t>(::write(fd, buffer, size)) != size) {
        throw std::runtime_error(QObject::tr("Destination drive is not writable").toStdString());
    }
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() const {
    return m_fileDescriptor.fileDescriptor();
}

/**
 * Create a new dos label on the partition. This essentially wipes all
 * existing information about partitions.
 */
void Drive::wipe() {
    QDBusReply<void> formatReply = m_device->call("Format", "dos", Properties());
    if (!formatReply.isValid() && formatReply.error().type() != QDBusError::NoReply) {
        throw std::runtime_error(formatReply.error().message().toStdString());
    }
}

/**
 * Fill the rest of the drive with a primary partition that uses the fat
 * filesystem.
 */
QPair<QString, qint64> Drive::addPartition(quint64 offset, const QString &label) {
    QDBusInterface partitionTable("org.freedesktop.UDisks2", m_identifier, "org.freedesktop.UDisks2.PartitionTable", QDBusConnection::systemBus());
    const quint64 proposedSize = m_device->property("Size").toULongLong() - offset;
    QDBusReply<QDBusObjectPath> partitionReply = partitionTable.call("CreatePartition", offset, proposedSize, "0xb", "", Properties{ { "partition-type", "primary" } });
    if (!partitionReply.isValid()) {
        throw std::runtime_error(partitionReply.error().message().toStdString());
    }
    QString partitionPath = partitionReply.value().path();
    QDBusInterface partition("org.freedesktop.UDisks2", partitionPath, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus());
    QDBusReply<void> formatPartitionReply = partition.call("Format", "vfat", Properties{ { "update-partition-type", true }, { "label", label } });
    if (!formatPartitionReply.isValid() && formatPartitionReply.error().type() != QDBusError::NoReply) {
        throw std::runtime_error(formatPartitionReply.error().message().toStdString());
    }
    const qint64 size = partition.property("Size").toULongLong();
    return qMakePair(partitionPath, size);
}

/**
 * Mount specified partition.
 */
QString Drive::mount(const QString &partitionIdentifier) {
    QDBusInterface partition("org.freedesktop.UDisks2", partitionIdentifier, "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
    QDBusReply<QString> reply = partition.call("Mount", Properties{});
    if (!reply.isValid()) {
        throw std::runtime_error(reply.error().message().toStdString());
    }
    return reply;
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
