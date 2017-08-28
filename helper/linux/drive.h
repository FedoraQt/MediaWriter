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

#include <utility>

#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QTextStream>
#include <QtGlobal>

#include "genericdrive.h"

class Drive : public GenericDrive {
    Q_OBJECT
public:
    /**
     * Shared public interface across platforms.
     */
    explicit Drive(const QString &driveIdentifier);
    ~Drive();
    void init();
    void write(const void *buffer, std::size_t size);
    int getDescriptor() const;
    void wipe();
    void addOverlayPartition(quint64 offset);
    void umount();

private:
    QDBusUnixFileDescriptor m_fileDescriptor;
    QString m_identifier;
    QScopedPointer<QDBusInterface> m_device;
    QString m_path;
};

#endif // DRIVE_H
