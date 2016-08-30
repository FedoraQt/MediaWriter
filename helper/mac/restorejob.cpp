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
#include <QTextStream>
#include <QFile>
#include <QProcess>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr), where(where)
{
    QTimer::singleShot(0, this, &RestoreJob::work);
}

void RestoreJob::work() {
    QFile target("/dev/r"+where);
    QByteArray buffer(1024 * 512, 0);

    QProcess diskUtil;
    diskUtil.setProgram("diskutil");
    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    target.open(QIODevice::WriteOnly);

    for (int i = 0; i < 64; i++) {
        target.write(buffer);
    }

    target.close();

    diskUtil.setProcessChannelMode(QProcess::ForwardedChannels);
    diskUtil.setArguments(QStringList() << "partitionDisk" << where << "1" << "MBR" << "fat32" << "FLASHDISK" << "R");
    diskUtil.start();
    diskUtil.waitForFinished();

    qApp->exit();
}
