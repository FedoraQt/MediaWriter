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

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where)
{
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
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
    out << "WRITE\n";
    out.flush();

    if (what.endsWith(".xz")) {
        if (!writeCompressed()) {
            return;
        }
    }
    else {
        if (!writePlain()) {
            return;
        }
    }
    check();
}

void WriteJob::onFileChanged(const QString &path) {
    if (QFile::exists(path))
        return;

    what = what.replace(QRegExp("[.]part$"), "");

    work();
}

bool WriteJob::writePlain() {
    qint64 bytesTotal = 0;

    QFile source(what);
    QFile target("/dev/r"+where);
    QByteArray buffer(BLOCK_SIZE, 0);

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
        qint64 bytes = source.read(buffer.data(), BLOCK_SIZE);
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
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    uint8_t *inBuffer = new uint8_t[BLOCK_SIZE];
    uint8_t *outBuffer = new uint8_t[BLOCK_SIZE];

    QFile source(what);
    source.open(QIODevice::ReadOnly);
    QFile target("/dev/r"+where);
    target.open(QIODevice::WriteOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = inBuffer;
    strm.avail_in = 0;
    strm.next_out = outBuffer;
    strm.avail_out = BLOCK_SIZE;

    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = source.read((char*) inBuffer, BLOCK_SIZE);
            totalRead += len;

            strm.next_in = inBuffer;
            strm.avail_in = len;

            out << totalRead << "\n";
            out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            quint64 len = target.write((char*) outBuffer, BLOCK_SIZE - strm.avail_out);
            if (len != BLOCK_SIZE - strm.avail_out) {
                err << tr("Destination drive is not writable");
                qApp->exit(3);
                return false;
            }
            return true;
        }
        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                err << tr("There is not enough memory to decompress the file.");
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                err << tr("The downloaded compressed file is corrupted.");
                break;
            case LZMA_OPTIONS_ERROR:
                err << tr("Unsupported compression options.");
                break;
            default:
                err << tr("Unknown decompression error.");
                break;
            }
            qApp->exit(4);
            return false;
        }

        if (strm.avail_out == 0) {
            quint64 len = target.write((char*) outBuffer, BLOCK_SIZE - strm.avail_out);
            if (len != BLOCK_SIZE - strm.avail_out) {
                err << tr("Destination drive is not writable");
                qApp->exit(3);
                return false;
            }
            strm.next_out = outBuffer;
            strm.avail_out = BLOCK_SIZE;
        }
    }
}

void WriteJob::check() {
    QFile target("/dev/r"+where);
    target.open(QIODevice::ReadOnly);
    out << "CHECK\n";
    out.flush();
    switch (mediaCheckFD(target.handle(), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "DONE\n";
        out.flush();
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
