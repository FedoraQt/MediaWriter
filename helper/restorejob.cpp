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

#include "restorejob.h"

#include <algorithm>
#include <memory>

#include <QCoreApplication>
#include <QString>
#include <QTimer>

#include "drive.h"

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr), drive(std::move(std::unique_ptr<Drive>(new Drive(where)))) {
    QTimer::singleShot(0, this, SLOT(work()));
}

void RestoreJob::work() {
    qApp->exit(drive->umount() || drive->wipe() || drive->addPartition());
}
