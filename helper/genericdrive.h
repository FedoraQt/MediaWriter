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

#ifndef GENERICDRIVE_H
#define GENERICDRIVE_H

#include <QObject>
#include <QString>

class GenericDrive : public QObject {
    Q_OBJECT
public:
    virtual void init() = 0;
    virtual void write(const void *buffer, std::size_t size) = 0;
    virtual int getDescriptor() const = 0;
    virtual void wipe() = 0;
    virtual void addOverlayPartition(quint64 offset) = 0;
    virtual void umount() = 0;
    void writePlain(const QString &source);
    void writeCompressed(const QString &source);
    void writeFile(const QString& source);
    void writeIso(const QString& source, bool persistentStorage);
    void checkChecksum();
    void implantChecksum();
protected:
    void addOverlay(quint64 offset, quint64 size);
};

#endif // GENERICDRIVE_H
