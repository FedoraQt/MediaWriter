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
#include <QFile>

#include <QDebug>

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where), dd(new QProcess(this))
{
    QTimer::singleShot(0, this, &WriteJob::work);
}

void WriteJob::work() {
    qint64 bytesTotal = 0;

    QFile source(what);
    QFile target("/dev/r"+where);
    QByteArray buffer(1024 * 512, 0);

    out << -1 << "\n";
    out.flush();

    QProcess diskUtil;
    diskUtil.setProgram("diskutil");
    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    source.open(QIODevice::ReadOnly);
    target.open(QIODevice::WriteOnly);

    while (source.isReadable() && !source.atEnd() && target.isWritable()) {
        qint64 bytes = source.read(buffer.data(), 1024 * 512);
        bytesTotal += bytes;
        target.write(buffer.data(), bytes);
        out << bytesTotal << "\n";
        out.flush();
    }

    target.flush();
    target.close();

    diskUtil.setArguments(QStringList() << "disableJournal" << where + "s1");
    diskUtil.start();
    diskUtil.waitForFinished();
    diskUtil.setArguments(QStringList() << "disableJournal" << where + "s2");
    diskUtil.start();
    diskUtil.waitForFinished();
    diskUtil.setArguments(QStringList() << "disableJournal" << where + "s3");
    diskUtil.start();
    diskUtil.waitForFinished();

    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();
/*
    diskUtil.setArguments(QStringList() << "eject" << where);
    diskUtil.start();
    diskUtil.waitForFinished();
*/
    qApp->exit();
}
