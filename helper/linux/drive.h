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

#ifndef DRIVE_H
#define DRIVE_H

#include <memory>

#include <QString>
#include <QTextStream>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>

class Drive {
public:
    /**
     * Shared public interface across platforms.
     */
    Drive(const QString &driveIdentifier);
    void open();
    void close();
    void write(const void *buffer, std::size_t size);
    int getDescriptor();
    void wipe();
    void addPartition(const QString &label = "");
    QString mount(const QString &partitionIdentifier);
    void umount();

private:
    QTextStream err;
    QDBusUnixFileDescriptor fileDescriptor;
    QString identifier;
    std::unique_ptr<QDBusInterface> device;
    QString path;
    std::unique_ptr<QDBusInterface> drive;
};

#endif // DRIVE_H
