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

#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>

#include <QtDBus>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>

#include <QDebug>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where)
{
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();
    QTimer::singleShot(0, this, &WriteJob::work);
}

void WriteJob::work()
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
                    message = partition.call("Unmount", Properties { {"force", true} });
                    err << "Unmounting " << currentDrivePath << ": " << message.errorMessage() << "\n";
                    err.flush();
                }
            }
        }
    }
    else {
        err << "So broken\n";
        err << message.errorMessage();
        err.flush();
        qApp->exit(2);
        return;
    }

    err << device.property("Id").toString();
    err.flush();
    QDBusReply<QDBusUnixFileDescriptor> reply = device.callWithArgumentList(QDBus::Block, "OpenForRestore", {Properties()} );
    QDBusUnixFileDescriptor fd = reply.value();

    if (!fd.isValid()) {
        err << reply.error().message();
        err.flush();
        qApp->exit(2);
        return;
    }

    QFile outFile;
    outFile.open(fd.fileDescriptor(), QIODevice::WriteOnly, QFileDevice::AutoCloseHandle);
    QFile inFile(what);
    inFile.open(QIODevice::ReadOnly);

    if (!outFile.isWritable()) {
        err << "FD is not writable";
        err.flush();
        qApp->exit(2);
        return;
    }
    if (!inFile.isReadable()) {
        err << "Source image is not readable";
        err.flush();
        qApp->exit(2);
        return;
    }

    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);
    qint64 total = 0;

    while(!inFile.atEnd()) {
        qint64 len = inFile.read(buffer.data(), BUFFER_SIZE);
        qint64 written = outFile.write(buffer.data(), len);
        if (written != len) {
            err << "Couldn't write data to the destination";
            err.flush();
            qApp->exit(3);
            return;
        }
        total += len;
        out << total << '\n';
        out.flush();
    }
    err << inFile.errorString() << "\n";
    err << outFile.errorString() << "\n";
    err << "OK\n";
    err.flush();
    qApp->exit(0);
}
