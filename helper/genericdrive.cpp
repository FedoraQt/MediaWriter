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

#include <libcheckisomd5.h>
#include <libimplantisomd5.h>
#include <lzma.h>

#include "blockdevice.h"
#include "page_aligned_buffer.h"
#include "write.h"

#ifndef MEDIAWRITER_LZMA_LIMIT
// 256MB memory limit for the decompressor
#define MEDIAWRITER_LZMA_LIMIT (1024 * 1024 * 256)
#endif

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
        throw std::runtime_error("Failed to start decompressing.");
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
        throw std::runtime_error("Source image is not readable");
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
            throw std::runtime_error("Source image is not readable");
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
        throw std::runtime_error("Your drive is probably damaged.");
    default:
        throw std::runtime_error("Unexpected error occurred during media check.");
    }
}

void GenericDrive::implantChecksum() {
    char *errstr;
    if (::implantISOFD(getDescriptor(), false, true, true, &errstr) != 0) {
        throw std::runtime_error(std::string(errstr));
    }
}

void GenericDrive::addOverlay(quint64 offset, quint64 size) {
    BlockDevice device(getDescriptor());
    device.read();
    device.addPartition(offset, size);
    device.formatOverlayPartition(offset, size);
}

void GenericDrive::writeIso(const QString &source, bool persistentStorage) {
    auto sourceFile = source.toStdString();
    if (::changePersistentStorage(sourceFile, persistentStorage)) {
        char *errstr;
        if (::implantISOFile(sourceFile.c_str(), false, true, true, &errstr) != 0) {
            throw std::runtime_error(std::string(errstr));
        }
    }
    umount();
    writeFile(source);
    checkChecksum();
    if (persistentStorage) {
        umount();
        QTextStream out(stdout);
        out << "OVERLAY\n";
        out.flush();
        auto size = QFileInfo(source).size();
        addOverlayPartition(size);
        implantChecksum();
        out << "DONE\n";
        out.flush();
    }
}
