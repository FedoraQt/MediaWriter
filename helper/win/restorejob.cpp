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
#include <QCoreApplication>
#include <QTextStream>
#include <QTimer>

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr)
{
    bool ok = false;
    m_where = where.toInt(&ok);
    if (!ok)
        qApp->exit(1);
    else
        QTimer::singleShot(0, this, &RestoreJob::work);
}

void RestoreJob::work() {
    m_diskpart.setProgram("diskpart.exe");
    m_diskpart.setProcessChannelMode(QProcess::ForwardedChannels);

    m_diskpart.start(QIODevice::ReadWrite);

    m_diskpart.write(qPrintable(QString("select disk %0\r\n").arg(m_where)));
    m_diskpart.write("clean\r\n");
    m_diskpart.write("create part pri\r\n");
    // again, for some reason this works, doing it once does not
    m_diskpart.write("clean\r\n");
    m_diskpart.write("create part pri\r\n");
    m_diskpart.write("select part 1\r\n");
    m_diskpart.write("format fs=fat32 quick\r\n");
    m_diskpart.write("assign\r\n");
    m_diskpart.write("exit\r\n");

    if (m_diskpart.waitForFinished()) {
        qApp->exit(0);
    }
    else {
        err << m_diskpart.readAllStandardError();
        err.flush();
        qApp->exit(1);
    }
}
