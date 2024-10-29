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

#ifndef RESTOREJOB_H
#define RESTOREJOB_H

#include <libwindisk/windisk.h>

#include <QObject>
#include <QProcess>
#include <QTextStream>

class RestoreJob : public QObject
{
    Q_OBJECT
public:
    explicit RestoreJob(const QString &where, QObject *parent);

signals:

private slots:
    void work();

private:
    QTextStream m_out{stdout};
    QTextStream m_err{stderr};

    std::unique_ptr<WinDiskManagement> m_diskManagement;
    std::unique_ptr<WinDisk> m_disk;
};

#endif // RESTOREJOB_H
