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

#include "isomd5/libcheckisomd5.h"

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where), dd(new QProcess(this))
{
    if (what.endsWith(".part")) {
        watcher.addPath(what);
    }
    else {
        QTimer::singleShot(0, this, &WriteJob::work);
    }
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob*)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    Q_UNUSED(total)
    out << offset << "\n";
    out.flush();
    return 0;
}

void WriteJob::work() {
    if (what.endsWith(".xz")) {
        if (!writePlain()) {
            return;
        }
    }
    else {
        if (!writeCompressed()) {
            return;
        }
    }
    check();
}

void WriteJob::onFileChanged(const QString &path) {
    Q_UNUSED(path);

    what = what.replace(QRegExp("[.]part$"), "");

    work();
}

bool WriteJob::writePlain() {
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
        qint64 written = target.write(buffer.data(), bytes);
        if (written != bytes) {
            err << tr("Destination drive is not writable") << "\n";
            err.flush();
            qApp->exit(1);
            return true;
        }
        out << bytesTotal << "\n";
        out.flush();
    }

    target.flush();
    target.close();

    for (int i = 0; i < 5; i++) {
        diskUtil.setArguments(QStringList() << "disableJournal" << QString("%1s%2").arg(where).arg(i));
        diskUtil.start();
        diskUtil.waitForFinished();
    }

    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    return true;
}

bool WriteJob::writeCompressed() {
    qApp->exit(1);
    return false;
}

void WriteJob::check() {
    QFile target("/dev/r"+where);
    target.open(QIODevice::ReadOnly);
    switch (mediaCheckFD(target.handle(), &WriteJob::staticOnMediaCheckAdvanced, this)) {
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

    qApp->exit();
}
