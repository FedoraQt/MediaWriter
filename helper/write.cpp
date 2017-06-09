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

#include "write.h"

#include <stdexcept>

#include <QFile>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"

#include "page_aligned_buffer.h"

static void writeCompressed(const QString &source, Drive* const drive) {
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *inBuffer = static_cast<char*>(buffers.get(0));
    char *outBuffer = static_cast<char*>(buffers.get(1));

    QFile file(source);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        throw std::runtime_error("Failed to start decompressing.");
    }

    strm.next_in = reinterpret_cast<uint8_t*>(inBuffer);
    strm.avail_in = 0;
    strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
    strm.avail_out = bufferSize;

    drive->open();

    QTextStream out(stdout);
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
            drive->write(outBuffer, bufferSize - strm.avail_out);
            return;
        }
        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                throw std::runtime_error("There is not enough memory to decompress the file.");
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                throw std::runtime_error("The downloaded compressed file is corrupted.");
                break;
            case LZMA_OPTIONS_ERROR:
                throw std::runtime_error("Unsupported compression options.");
                break;
            default:
                throw std::runtime_error("Unknown decompression error.");
                break;
            }
        }

        if (strm.avail_out == 0) {
            drive->write(outBuffer, bufferSize - strm.avail_out);

            strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
            strm.avail_out = bufferSize;
        }
    }
}

static void writePlain(const QString &source, Drive* const drive) {
    QFile inFile(source);
    inFile.open(QIODevice::ReadOnly);

    if (!inFile.isReadable()) {
        throw std::runtime_error("Source image is not readable");
    }

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *buffer = static_cast<char*>(buffers.get(0));

    drive->open();

    qint64 total = 0;
    while (!inFile.atEnd()) {
        qint64 len = inFile.read(buffer, bufferSize);
        if (len < 0) {
            throw std::runtime_error("Source image is not readable");
        }
        drive->write(buffer, len);
        total += len;
        QTextStream out(stdout);
        out << total << '\n';
        out.flush();
    }
}

static int onMediaCheckAdvanced(void *data, long long offset, long long total) {
    Q_UNUSED(data);
    Q_UNUSED(total);
    QTextStream out(stdout);
    out << offset << "\n";
    out.flush();
    return 0;
}

static void check(int fd) {
    QTextStream out(stdout);
    out << "CHECK\n";
    out.flush();

    switch (mediaCheckFD(fd, &onMediaCheckAdvanced, nullptr)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "OK\n";
        out.flush();
        break;
    case ISOMD5SUM_CHECK_FAILED:
        throw std::runtime_error("Your drive is probably damaged.");
    default:
        throw std::runtime_error("Unexpected error occurred during media check.");
    }
}

void write(const QString &source, Drive *const drive) {
    // Immediately trigger the UI into writing mode.
    QTextStream out(stdout);
    out << "1\n";
    out.flush();
    drive->umount();
    if (source.endsWith(".xz"))
        writeCompressed(source, drive);
    else
        writePlain(source, drive);
    check(drive->getDescriptor());
}
