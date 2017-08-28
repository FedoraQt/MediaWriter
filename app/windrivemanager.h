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

#ifndef WINDRIVEMANAGER_H
#define WINDRIVEMANAGER_H

#include "drivemanager.h"

#include <QProcess>

class WinDriveProvider;
class WinDrive;

class WinDriveProvider : public DriveProvider
{
    Q_OBJECT
public:
    WinDriveProvider(DriveManager *parent);

public slots:
    void checkDrives();
private:
    QSet<int> findPhysicalDrive(char driveLetter);
    bool describeDrive(int driveNumber, bool hasLetter, bool verbose);

    QMap<int, WinDrive*> m_drives;
};

class WinDrive : public Drive {
    Q_OBJECT
public:
    WinDrive(WinDriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive, const QString &serialNumber);
    ~WinDrive();

    QString serialNumber() const;

    bool operator==(const WinDrive &o) const;

private:
    QString helperBinary() override;

private:
    QString m_serialNo;
};

#endif // WINDRIVEMANAGER_H
