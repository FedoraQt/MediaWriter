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

#include <algorithm>
#include <memory>
#include <random>
#include <tuple>

#include <QCoreApplication>
#include <QFile>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QtGlobal>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"
// Platform specific drive handler.
#include "drive.h"
#include "page_aligned_buffer.h"

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), out(stdout), err(stderr),
      drive(std::move(std::unique_ptr<Drive>(new Drive(where)))) {
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
    QTimer::singleShot(0, this, SLOT(boot()));
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob *) data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    Q_UNUSED(total);
    out << offset << "\n";
    out.flush();
    return 0;
}

void WriteJob::boot() {
    if (what.endsWith(".part")) {
        watcher.addPath(what);
        return;
    }
    work();
}

bool WriteJob::work() {
    return write() && check();
}

void WriteJob::onFileChanged(const QString &path) {
    if (QFile::exists(path))
        return;

    what = what.replace(QRegExp("[.]part$"), "");

    if (!QFile::exists(what)) {
        qApp->exit(4);
        return;
    }
    // Immediately trigger the UI into writing mode.
    out << "1\n";
    out.flush();

    work();
}

bool WriteJob::write() {
    if (what.endsWith(".xz"))
        return writeCompressed();
    else
        return writePlain();
}

bool WriteJob::writeCompressed() {
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *inBuffer = static_cast<char*>(buffers.get(0));
    char *outBuffer = static_cast<char*>(buffers.get(1));

    QFile file(what);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = reinterpret_cast<uint8_t*>(inBuffer);
    strm.avail_in = 0;
    strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
    strm.avail_out = bufferSize;

    drive->open();

    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = file.read(inBuffer, bufferSize);
            totalRead += len;

            strm.next_in = reinterpret_cast<uint8_t*>(inBuffer);
            strm.avail_in = len;

            out << totalRead << "\n";
            out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            if (!drive->write(outBuffer, bufferSize - strm.avail_out)) {
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
            if (!drive->write(outBuffer, bufferSize - strm.avail_out)) {
                return false;
            }

            strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
            strm.avail_out = bufferSize;
        }
    }
}

bool WriteJob::writePlain() {

    QFile inFile(what);
    inFile.open(QIODevice::ReadOnly);

    if (!inFile.isReadable()) {
        err << tr("Source image is not readable") << what;
        err.flush();
        qApp->exit(2);
        return false;
    }

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *buffer = static_cast<char*>(buffers.get(0));

    drive->open();

    qint64 total = 0;
    while (!inFile.atEnd()) {
        qint64 len = inFile.read(buffer, bufferSize);
        if (len < 0) {
            err << tr("Source image is not readable");
            err.flush();
            qApp->exit(3);
            return false;
        }
        if (!drive->write(buffer, len)) {
            err << tr("Destination drive is not writable");
            err.flush();
            qApp->exit(3);
            return false;
        }
        total += len;
        out << total << '\n';
        out.flush();
    }
    return true;
}

bool WriteJob::check() {
    out << "CHECK\n";
    out.flush();

    switch (mediaCheckFD(drive->getDescriptor(), &WriteJob::staticOnMediaCheckAdvanced, this)) {
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
        return false;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        return false;
    }

    return true;
}
