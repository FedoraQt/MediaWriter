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

#ifndef MACDRIVEMANAGER_H
#define MACDRIVEMANAGER_H

#include "drivemanager.h"
#include <QMap>
#include <QProcess>
#include <QString>
#include <QStringList>

class MacDrive;

class MacDriveProvider : public DriveProvider {
    Q_OBJECT
public:
    MacDriveProvider(DriveManager *parent);
    ~MacDriveProvider();

    static MacDriveProvider *instance();
    static void onDriveAdded(const char *bsdName, const char *vendor, const char *model, unsigned long long size, bool restoreable);
    void addDrive(const QString &bsdName, const QString &vendor, const QString &model, uint64_t size, bool restoreable);
    static void onDriveRemoved(const char *bsdName);
    void removeDrive(const QString &bsdName);
private:
    static MacDriveProvider *_self;
    QMap<QString, MacDrive*> m_devices;
};

class MacDrive : public Drive {
    Q_OBJECT
public:
    MacDrive(DriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive);

private:
    void prepareProcess(const QString &binary, const QStringList& arguments) override;
    QString helperBinary() override;
};

#endif // MACDRIVEMANAGER_H
