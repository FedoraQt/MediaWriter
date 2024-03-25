/*
 * AOSC Media Writer
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
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QTimer>

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr)
    , where(where)
{
    QTimer::singleShot(0, this, &RestoreJob::work);
}

void RestoreJob::work()
{
    QProcess diskUtil;
    diskUtil.setProgram("diskutil");
    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    diskUtil.setProcessChannelMode(QProcess::ForwardedChannels);
    diskUtil.setArguments(QStringList() << "partitionDisk" << where << "1"
                                        << "MBR"
                                        << "ExFAT"
                                        << "FLASHDISK"
                                        << "R");
    diskUtil.start();
    diskUtil.waitForFinished();

    qApp->exit();
}
