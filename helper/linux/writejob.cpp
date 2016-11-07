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

#include "isomd5/libcheckisomd5.h"

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

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob*)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    Q_UNUSED(total);
    out << offset << "\n";
    out.flush();
    return 0;
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
                }
            }
        }
    }
    else {
        err << message.errorMessage();
        err.flush();
        qApp->exit(2);
        return;
    }

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
        err << tr("Destination drive is not writable");
        err.flush();
        qApp->exit(2);
        return;
    }
    if (!inFile.isReadable()) {
        err << tr("Source image is not readable");
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
            err << tr("Destination drive is not writable");
            err.flush();
            qApp->exit(3);
            return;
        }
        total += len;
        out << total << '\n';
        out.flush();
    }

    outFile.close();
    inFile.close();

    out << "CHECK\n";
    out.flush();

    QDBusReply<QDBusUnixFileDescriptor> backupReply = device.callWithArgumentList(QDBus::Block, "OpenForBackup", {Properties()} );
    QDBusUnixFileDescriptor backupFd = backupReply.value();

    if (!backupReply.isValid() || !backupFd.isValid()) {
        err << reply.error().message();
        err.flush();
        qApp->exit(2);
        return;
    }

    switch (mediaCheckFD(backupFd.fileDescriptor(), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        err << "OK\n";
        err.flush();
        qApp->exit(0);
        break;
    case ISOMD5SUM_CHECK_FAILED:
        err << tr("Your drive is probably damaged.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    }
}
