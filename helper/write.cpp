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

#include <algorithm>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <utility>
#include <utility>
#include <vector>

#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal>

#include <iso9660io.h>
#include <lzma.h>

extern "C" {
#include <libcheckisomd5.h>
#include <libimplantisomd5.h>
}

#include "page_aligned_buffer.h"

constexpr char partitionLabel[] = "OVERLAY";
constexpr char overlayFilename[] = "OVERLAY.IMG";

static std::streamsize addOverlay(std::fstream *iofile, const iso9660::File &fileinfo) {
    static const std::string needle = "rd.live.image";
    static const std::string overlaySwitch = QString(" rd.live.overlay=LABEL=%1:/%2")
                                                     .arg(partitionLabel)
                                                     .arg(overlayFilename)
                                                     .toStdString();
    const std::size_t maxGrowth = fileinfo.max_growth();
    std::size_t growth = 0;
    std::size_t insertPosition = 0;
    std::size_t readBytes = 0;
    auto &file = *iofile;
    std::vector<char> fileContent;
    for (std::string line; std::getline(file, line);) {
        line += '\n';
        readBytes += line.size();
        if (readBytes > fileinfo.size) {
            throw std::runtime_error("Source image is corrupt.");
        }
        if (growth == 0 && line.find(overlaySwitch) != std::string::npos) {
            return 0;
        }
        const auto position = line.find(needle);
        if (position != std::string::npos) {
            growth += overlaySwitch.size();
            if (growth > maxGrowth) {
                throw std::runtime_error("Source image is unusable.");
            }
            const std::size_t lineSize = line.size();
            const std::size_t lineOffset = position + needle.size();
            line.insert(lineOffset, overlaySwitch);
            if (insertPosition == 0) {
                insertPosition = file.tellg();
                insertPosition -= lineSize - lineOffset;
                fileContent.reserve((fileinfo.size + maxGrowth) - readBytes);
                std::move(line.begin() + lineOffset, line.end(), std::back_inserter(fileContent));
            }
            else {
                std::move(line.begin(), line.end(), std::back_inserter(fileContent));
            }
        }
        else if (insertPosition != 0) {
            std::move(line.begin(), line.end(), std::back_inserter(fileContent));
        }
        if (readBytes == fileinfo.size)
            break;
    }
    file.clear();
    file.seekp(insertPosition);
    file.write(fileContent.data(), fileContent.size());
    return growth;
}

/**
 * It's probably be a bad idea to copy the entire image just so that there's
 * one version that has persistent storage and one that doesn't therefore it
 * needs to be possible to remove the overlay switches from the image again.
 */
static std::streamsize removeOverlay(std::fstream *iofile, const iso9660::File &fileinfo) {
    std::string needle = QString(" rd.live.overlay=LABEL=%1:/%2")
                                 .arg(partitionLabel)
                                 .arg(overlayFilename)
                                 .toStdString();
    std::size_t numSwitches = 0;
    std::size_t insertPosition = 0;
    std::size_t readBytes = 0;
    std::string line;
    auto &file = *iofile;
    while (std::getline(file, line)) {
        line += '\n';
        readBytes += line.size();
        if (readBytes > fileinfo.size) {
            throw std::runtime_error("Source image is corrupt.");
        }
        const auto position = line.find(needle);
        if (position != std::string::npos) {
            ++numSwitches;
            if (insertPosition == 0) {
                line = line.substr(position + needle.size());
                insertPosition = static_cast<std::size_t>(file.tellg()) - line.size() - needle.size();
            }
            else {
                line = line.substr(0, position) + line.substr(position + needle.size());
            }
        }
        if (insertPosition != 0) {
            auto readPosition = file.tellg();
            file.clear();
            file.seekp(insertPosition);
            file.write(line.data(), line.size());
            file.seekg(readPosition);
            insertPosition += line.size();
        }
        if (readBytes == fileinfo.size)
            break;
    }
    if (insertPosition != 0) {
        std::fill(needle.begin(), needle.end(), '\0');
        file.clear();
        file.seekp(insertPosition);
        for (std::size_t i = 0; i < numSwitches; ++i) {
            file.write(needle.data(), needle.size());
        }
        return -(numSwitches * needle.size());
    }
    return 0;
}

static bool modifyIso(iso9660::Image *const image, const char *const filename, bool persistentStorage) {
    auto file = image->find(filename);
    if (file == nullptr) {
        return false;
    }
    if (persistentStorage) {
        return image->modify_file(*file, addOverlay);
    }
    return image->modify_file(*file, removeOverlay);
}

static bool modifyIso(const std::string &filename, bool persistentStorage) {
    constexpr const char *const configfiles[] = { "isolinux.cfg", "grub.cfg", "grub.conf" };
    std::fstream isofile(filename, std::ios::binary | std::ios::in | std::ios::out);
    iso9660::Image image(&isofile);
    image.read();
    bool changed = false;
    for (const char *const configfile : configfiles) {
        if (modifyIso(&image, configfile, persistentStorage)) {
            changed = true;
        }
    }
    image.write();
    return changed;
}

static void zeroFile(const QString &filename, qint64 size) {
    constexpr int FOURKB = 4096;
    constexpr qint64 MAX_FILE_SIZE = FOURKB * 1024L * 1024L - 1;
    size = std::max(MAX_FILE_SIZE, size);
    QByteArray zeros(FOURKB, '\0');
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    qint64 iterations = size / zeros.size();
    while (iterations--) {
        file.write(zeros);
    }
    int remaining = size - iterations * zeros.size();
    if (remaining > 0) {
        zeros.truncate(remaining);
        file.write(zeros);
    }
}

static void writeCompressed(const QString &source, Drive *const drive) {
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

static void writePlain(const QString &source, Drive *const drive) {
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

void write(const QString &source, Drive *const drive, bool persistentStorage) {
    // Immediately trigger the UI into writing mode.
    QTextStream out(stdout);
    out << "1\n";
    out.flush();
    auto sourceFile = source.toStdString();
    if (modifyIso(sourceFile, persistentStorage)) {
        char *errstr;
        if (implantISOFile(sourceFile.c_str(), false, true, true, &errstr) != 0) {
            throw std::runtime_error(std::string(errstr));
        }
    }
    drive->umount();
    if (source.endsWith(".xz"))
        writeCompressed(source, drive);
    else
        writePlain(source, drive);
    check(drive->getDescriptor());
    if (persistentStorage) {
        drive->umount();
        auto size = QFileInfo(source).size();
        auto partitionInfo = drive->addPartition(size, partitionLabel);
        QString mountpoint = drive->mount(partitionInfo.first);
        zeroFile(mountpoint + overlayFilename, partitionInfo.second);
        drive->umount();
        drive->open();
        out.flush();
        char *errstr;
        if (implantISOFD(drive->getDescriptor(), false, true, true, &errstr) != 0) {
            throw std::runtime_error(errstr);
        }
        drive->close();
    }
}
