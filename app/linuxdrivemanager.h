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

#ifndef LINUXDRIVEMANAGER_H
#define LINUXDRIVEMANAGER_H

#include "drivemanager.h"

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QProcess>

typedef QHash<QString, QVariantMap> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

class LinuxDriveProvider;
class LinuxDrive;

class LinuxDriveProvider : public DriveProvider {
    Q_OBJECT
public:
    LinuxDriveProvider(DriveManager *parent);

private slots:
    void delayedConstruct();
    void init(QDBusPendingCallWatcher *w);
    void onInterfacesAdded(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties);
    void onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces);
    void onPropertiesChanged(const QString &interface_name, const QVariantMap &changed_properties, const QStringList &invalidated_properties);

private:
    QDBusObjectPath handleObject(const QDBusObjectPath &path, const InterfacesAndProperties &interface);

private:
    QDBusInterface *m_objManager { nullptr };
    QHash<QDBusObjectPath, LinuxDrive*> m_drives;
};

class LinuxDrive : public Drive {
    Q_OBJECT
public:
    LinuxDrive(DriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive);
    ~LinuxDrive();

private:
    QString helperBinary() override;

};

#endif // LINUXDRIVEMANAGER_H
