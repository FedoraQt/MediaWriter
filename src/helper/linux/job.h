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

#ifndef JOB_H
#define JOB_H

#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>
#include <QObject>
#include <QTextStream>

#include <fcntl.h>
#include <unistd.h>

#include <memory>
#include <tuple>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;

class Job : public QObject
{
    Q_OBJECT
public:
    explicit Job(const QString &where);
    Job(const QString &what, const QString &where);

    QDBusUnixFileDescriptor getDescriptor();

public slots:
    virtual void work() = 0;

protected:
    QString what;
    QString where;
    QTextStream out{stdout};
    QTextStream err{stderr};
    QDBusUnixFileDescriptor fd{-1};
};

std::tuple<std::unique_ptr<char[]>, char *, std::size_t> pageAlignedBuffer(std::size_t pages = 1024);

#endif // JOB_H
