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

#include "restorejob.h"

#include <QCoreApplication>
#include <QThread>
#include <QTimer>

#include <QDBusArgument>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>

#include <stdio.h>
#include <sys/types.h>

RestoreJob::RestoreJob(const QString &where)
    : Job(where)
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

    // Wipe out first and last 128 blocks with zeroes
    fd = getDescriptor();
    if (fd.fileDescriptor() < 0) {
        err << tr("Failed to open device for writing");
        err.flush();
        qApp->exit(1);
    }

    auto bufferOwner = pageAlignedBuffer();
    char *buffer = std::get<1>(bufferOwner);
    qint64 size = std::get<2>(bufferOwner);

    memset(buffer, '\0', size);

    // Overwrite first 128 blocks with zeroes
    for (int i = 0; i < 128; i++) {
        qint64 written = ::write(fd.fileDescriptor(), buffer, size);
        if (written != size) {
            err << tr("Destination drive is not writable");
            err.flush();
            qApp->exit(1);
        }
    }

    // Rewind the filepointer to the last 128 blocks
    off_t filesize = lseek(fd.fileDescriptor(), 0, SEEK_END);
    if (filesize == static_cast<off_t>(-1)) {
        err << tr("Failed to get file size");
        err.flush();
        qApp->exit(1);
    }

    off_t offset = filesize - (128 * size);
    if (offset < 0) {
        err << tr("File size is smaller than 128 blocks");
        err.flush();
        qApp->exit(1);
    }

    // Move the file pointer to the calculated offset
    if (lseek(fd.fileDescriptor(), offset, SEEK_SET) == static_cast<off_t>(-1)) {
        err << tr("Failed to move file pointer to the end region");
        err.flush();
        qApp->exit(1);
    }

    // Overwrite last 128 blocks with zeroes
    for (int i = 0; i < 128; i++) {
        qint64 written = ::write(fd.fileDescriptor(), buffer, size);
        if (written != size) {
            err << tr("Destination drive is not writable");
            err.flush();
            qApp->exit(1);
        }
    }

    // Ensure data is flushed to disk
    if (::fsync(fd.fileDescriptor()) == -1) {
        err << tr("Failed to sync data to disk");
        err.flush();
        qApp->exit(1);
    }

    QDBusReply<void> formatReply = device.call("Format", "gpt", Properties());
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
