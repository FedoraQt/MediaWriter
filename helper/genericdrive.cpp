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

#include "genericdrive.h"

#include <string>
#include <stdexcept>

#include <QString>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>

#include "isomd5/libcheckisomd5.h"
#include <lzma.h>

#include "error.h"
#include "page_aligned_buffer.h"
#include "write.h"

void GenericDrive::writeCompressed(const QString &source) {
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *inBuffer = static_cast<char *>(buffers.get(0));
    char *outBuffer = static_cast<char *>(buffers.get(1));
    auto total = QFileInfo(source).size();
    ProgressStats progress;
    progress.fd = getDescriptor();

    QFile file(source);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        throw HelperError(Error::DECOMPRESS_INIT, source);
    }

    strm.next_in = reinterpret_cast<uint8_t *>(inBuffer);
    strm.avail_in = 0;
    strm.next_out = reinterpret_cast<uint8_t *>(outBuffer);
    strm.avail_out = bufferSize;

    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = file.read(inBuffer, bufferSize);
            totalRead += len;

            strm.next_in = reinterpret_cast<uint8_t *>(inBuffer);
            strm.avail_in = len;

            onProgress(&progress, totalRead, total);
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            write(outBuffer, bufferSize - strm.avail_out);
            return;
        }
        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                throw HelperError(Error::DECOMPRESS_MEM, source);
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                throw HelperError(Error::FILE_CORRUPT, source);
                break;
            case LZMA_OPTIONS_ERROR:
                throw HelperError(Error::DECOMPRESS_OPTIONS, source);
                break;
            default:
                throw HelperError(Error::DECOMPRESS_UNKNOWN, source);
                break;
            }
        }

        if (strm.avail_out == 0) {
            write(outBuffer, bufferSize - strm.avail_out);

            strm.next_out = reinterpret_cast<uint8_t *>(outBuffer);
            strm.avail_out = bufferSize;
        }
    }
}

void GenericDrive::writePlain(const QString &source) {
    QFile inFile(source);
    inFile.open(QIODevice::ReadOnly);

    if (!inFile.isReadable()) {
        throw HelperError(Error::FILE_READ, source);
    }

    PageAlignedBuffer<2> buffers;
    const std::size_t bufferSize = buffers.size;
    char *buffer = static_cast<char *>(buffers.get(0));
    auto total = QFileInfo(source).size();
    ProgressStats progress;
    progress.fd = getDescriptor();

    QTextStream out(stdout);
    qint64 bytesWritten = 0;
    while (!inFile.atEnd()) {
        qint64 len = inFile.read(buffer, bufferSize);
        if (len < 0) {
            throw HelperError(Error::FILE_READ, source);
        }
        write(buffer, len);
        bytesWritten += len;

        onProgress(&progress, bytesWritten, total);
    }
}

void GenericDrive::writeFile(const QString &source) {
    // Immediately trigger the UI into writing mode.
    QTextStream out(stdout);
    out << "1\n";
    out.flush();
    if (source.endsWith(".xz"))
        writeCompressed(source);
    else
        writePlain(source);
}

void GenericDrive::checkChecksum() {
    QTextStream out(stdout);
    out << "CHECK\n";
    out.flush();

    ProgressStats progress;
    switch (mediaCheckFD(getDescriptor(), &onProgress, &progress)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "OK\n";
        out.flush();
        break;
    case ISOMD5SUM_CHECK_FAILED:
        throw HelperError(Error::DRIVE_DAMAGED, drive());
    default:
        throw HelperError(Error::UNEXPECTED);
    }
}

void GenericDrive::writeIso(const QString &source) {
    umount();
    writeFile(source);
    checkChecksum();
}
