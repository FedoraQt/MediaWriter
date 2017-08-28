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

#include <cstring>
#include <unistd.h>

#include <iostream>
#include <array>
#include <algorithm>
#include <fstream>
#include <functional>
#include <ios>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <QObject>
#include <QString>
#include <QTextStream>
#include <QtGlobal>
#include <QtEndian>

#include <iso9660io.h>

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

static std::streamsize manipulateFatImage(std::fstream *iofile, const iso9660::File &fileinfo, bool persistentStorage) {
    Q_UNUSED(fileinfo);
    constexpr char configfile[] = "GRUB    CFG";
    /*
     * FIXME(squimrel): Find the location of the grub.cfg file content more
     * reliably by reading the cluster number of its directory entry and
     * locating it with the help of fatlength, sectors per cluster and
     * number of reserved sectors which also have to be read.
     */
    constexpr char grub_signature[] = "set default";
    constexpr int SECTOR_SIZE = 512;
    constexpr int FILESIZE_OFFSET = 28;
    char buffer[SECTOR_SIZE];
    std::streamsize entrypos = -1;
    std::streamsize growth = 0;
    quint32 filesize = 0;
    auto &file = *iofile;
    while (file.read(buffer, SECTOR_SIZE)) {
        std::size_t offset = 0;
        for (; offset < SECTOR_SIZE; offset += 0x20) {
            if (std::strncmp(buffer + offset, configfile, std::extent<decltype(configfile)>::value - 1) == 0) {
                break;
            }
        }
        // Figure out position of grub.cfg entry.
        if (offset < SECTOR_SIZE) {
            entrypos = (file.tellg() - static_cast<std::streamoff>(SECTOR_SIZE));
            entrypos += offset;
            std::memcpy(&filesize, buffer + offset + FILESIZE_OFFSET, sizeof(filesize));
            filesize = qFromLittleEndian(filesize);
            break;
        }
    }
    while (file.read(buffer, SECTOR_SIZE)) {
        // Modify overlay switches of grub.cfg.
        if (std::strncmp(buffer, grub_signature,
                    std::extent<decltype(grub_signature)>::value - 1) == 0) {
            file.seekg(-SECTOR_SIZE, std::ios::cur);
            iso9660::File fileinfo;
            fileinfo.size = filesize;
            if (persistentStorage) {
                growth = addOverlay(iofile, fileinfo);
            }
            else {
                growth = removeOverlay(iofile, fileinfo);
            }
            filesize += growth;
            break;
        }
    }
    if (entrypos >= 0) {
        file.seekg(entrypos + FILESIZE_OFFSET);
        filesize = qToLittleEndian(filesize);
        file.write(reinterpret_cast<char *>(&filesize), sizeof(filesize));
    }
    return 0;
}

/*
 * This solution is so much worse than the one for fat. It's surprising that it
 * works considering how ugly it is.
 */
static std::streamsize manipulateHfsImage(std::fstream *iofile, const iso9660::File &fileinfo, bool persistentStorage) {
    Q_UNUSED(fileinfo);
#ifdef Q_OS_WIN
    Q_UNUSED(iofile);
    Q_UNUSED(persistentStorage);
    return 0;
#else
    constexpr std::size_t SECTOR_SIZE = 512;
    auto sector_align = [SECTOR_SIZE](std::size_t size) {
        return (size + (SECTOR_SIZE - 1)) & -SECTOR_SIZE;
    };
    /*
     * Assume that there're two grub.cfg files are on this image and that they
     * have equal content.
     * If there're less than two files nothing will be modified.
     * If they don't have equal content the image might get corrupted if we're
     * unlucky.
     */
    constexpr std::size_t NUM_FILES = 2;
    std::array<std::streamsize, NUM_FILES> entrypos = { -1, -1 };
    std::array<quint32, NUM_FILES> filesize = { 0, 0 };
    // Depends on configfile length.
    constexpr std::size_t FILESIZE_OFFSET = 108;
    constexpr std::size_t SKIP = FILESIZE_OFFSET + sizeof(filesize[0]);
    constexpr std::size_t MAGIC_OFFSET = 0x40;
    constexpr char MAGIC_VALUE = '\x3f';
    constexpr char configfile[] = "\0g\0r\0u\0b\0.\0c\0f\0g";
    std::size_t file_index = 0;
    char buffer[SECTOR_SIZE];
    auto &file = *iofile;
    while (file.read(buffer, SECTOR_SIZE)) {
        char *entry = static_cast<char *>(
                memmem(buffer, SECTOR_SIZE - SKIP, configfile,
                        std::extent<decltype(configfile)>::value - 1));
        if (entry != nullptr && entry[MAGIC_OFFSET] == MAGIC_VALUE) {
            entrypos[file_index] =
                    (file.tellg() - static_cast<std::streamoff>(SECTOR_SIZE)) +
                    (entry - buffer);
            std::memcpy(&filesize[file_index], entry + FILESIZE_OFFSET,
                    sizeof(filesize[file_index]));
            filesize[file_index] = qFromBigEndian(filesize[file_index]);
            if (file_index < NUM_FILES - 1) {
                ++file_index;
            }
            else {
                // Make sure we're aligned for the grub.cfg signature search.
                file.seekg(sector_align(file.tellg()));
                file_index = 0;
                break;
            }
        }
        file.seekg(-SKIP, std::ios::cur);
    }
    while (file.read(buffer, SECTOR_SIZE)) {
        constexpr char grub_signature[] = "set default";
        // Modify overlay switches of grub.cfg.
        if (std::strncmp(buffer, grub_signature,
                    std::extent<decltype(grub_signature)>::value - 1) == 0) {
            file.seekg(-SECTOR_SIZE, std::ios::cur);
            iso9660::File fileinfo;
            fileinfo.size = filesize[file_index];
            if (persistentStorage) {
                filesize[file_index] += addOverlay(iofile, fileinfo);
            }
            else {
                filesize[file_index] += removeOverlay(iofile, fileinfo);
            }
            if (file_index < NUM_FILES - 1) {
                // Make sure we're aligned for the next grub.cfg signature search.
                file.seekg(sector_align(file.tellg()));
                ++file_index;
            }
            else {
                break;
            }
        }
    }
    for (file_index = 0; file_index < NUM_FILES; ++file_index) {
        if (entrypos[file_index] >= 0) {
            file.seekg(entrypos[file_index] + FILESIZE_OFFSET);
            filesize[file_index] = qToBigEndian(filesize[file_index]);
            file.write(reinterpret_cast<char *>(&filesize[file_index]),
                    sizeof(filesize[file_index]));
        }
    }
    return 0;
#endif // Q_OS_WIN
}

static void modifyFatImage(iso9660::Image *const image, const char *const filename, bool persistentStorage) {
    using namespace std::placeholders;
    auto file = image->find(filename);
    if (file == nullptr)
        return;

    image->modify_file(*file, std::bind(manipulateFatImage, _1, _2, persistentStorage));
}

static void modifyHfsImage(iso9660::Image *const image, const char *const filename, bool persistentStorage) {
    using namespace std::placeholders;
    auto file = image->find(filename);
    if (file == nullptr)
        return;

    image->modify_file(*file, std::bind(manipulateHfsImage, _1, _2, persistentStorage));
}

static bool modifyIso(iso9660::Image *const image, const char *const filename, bool persistentStorage) {
    auto file = image->find(filename);
    if (file == nullptr)
        return false;

    if (persistentStorage) {
        return image->modify_file(*file, addOverlay);
    }
    return image->modify_file(*file, removeOverlay);
}

bool changePersistentStorage(const std::string &filename, bool persistentStorage) {
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
    modifyFatImage(&image, "efiboot.img", persistentStorage);
    modifyHfsImage(&image, "macboot.img", persistentStorage);
    image.write();
    return changed;
}

int onProgress(void *data, long long offset, long long total) {
    constexpr long long MAGIC = 234;
    ProgressStats &stats = *static_cast<ProgressStats *>(data);
    const long long progress = (offset * MAGIC) / total;
    if (progress > stats.progress) {
        stats.progress = progress;
        if (offset > total)
            offset = total;
        QTextStream out(stdout);
        int percentage = (offset * 10000) / total;
#ifndef Q_OS_WIN
        // Flush every 32mb of progress to disk.
        if (offset >= 32 * 1024 * 1024 * (stats.syncs + 1)) {
            ++stats.syncs;
            ::fsync(stats.fd);
        }
#endif // Q_OS_WIN
        out << percentage << "\n";
        out.flush();
    }
    return 0;
}
