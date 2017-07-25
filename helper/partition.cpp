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
#include "partition.h"

#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

#include <algorithm>
#include <array>
#include <utility>

#include <QDateTime>
#include <QtAlgorithms>
#include <QtEndian>
#include <QtGlobal>

PartitionTable::PartitionTable(int fd) : m_fd(fd) {
}

void PartitionTable::setFileDescriptor(int fd) {
    m_fd = fd;
}

void PartitionTable::read() {
    seekEntry();
    for (std::size_t i = 0; i < MAX_PARTITIONS; ++i) {
        PartitionEntry entry;
        if (::read(m_fd, entry.data(), entry.size()) != entry.size()) {
            throw std::runtime_error("Failed to read partition table.");
        }
        const bool isEmtpy = std::all_of(entry.cbegin(), entry.cend(), [](const char c) { return c == '\0'; });
        if (isEmtpy) {
            continue;
        }
        m_entries.append(std::move(entry));
    }
}

void PartitionTable::seekEntry(std::size_t index) {
    if (::lseek(m_fd, PARTITION_ENTRY_OFFSET + PARTITION_ENTRY_SIZE * index, SEEK_SET) < 0) {
        throw std::runtime_error("Failed to seek to partition table.");
    }
}

void PartitionTable::seekto(std::size_t position) {
    if (::lseek(m_fd, position, SEEK_SET) < 0) {
        throw std::runtime_error("Failed to seek on block device.");
    }
}

void PartitionTable::writeZeros(std::size_t size) {
    constexpr std::size_t CHUNK_SIZE = 512;
    constexpr char zeros[CHUNK_SIZE]{};
    for (std::size_t i = 0; i < size; i += CHUNK_SIZE) {
        if (::write(m_fd, zeros, std::min(CHUNK_SIZE, size - i)) < 0) {
            throw std::runtime_error("Failed to write zeros to block device.");
        }
    }
}

void PartitionTable::wipeMac() {
    if (m_apmHeader == nullptr) {
        m_apmHeader = std::unique_ptr<std::array<char, APM_SIZE>>(new std::array<char, APM_SIZE>());
    }
    if (::lseek(m_fd, APM_OFFSET, SEEK_SET) < 0) {
        throw std::runtime_error("Failed to seek to apple partition header.");
    }
    auto bytes = ::read(m_fd, m_apmHeader->data(), m_apmHeader->size());
    if (bytes != static_cast<decltype(bytes)>(m_apmHeader->size())) {
        throw std::runtime_error("Failed to read apple partition header.");
    }
    {
        const std::array<char, APM_SIZE> zeros{};
        auto bytes = ::write(m_fd, zeros.data(), zeros.size());
        if (bytes != static_cast<decltype(bytes)>(zeros.size())) {
            throw std::runtime_error("Failed to wipe apple partition header.");
        }
    }
}

void PartitionTable::restoreMac() {
    if (m_apmHeader == nullptr)
        return;
    if (::lseek(m_fd, APM_OFFSET, SEEK_SET) < 0) {
        throw std::runtime_error("Failed to seek to apple partition header.");
    }
    auto bytes = ::write(m_fd, m_apmHeader->data(), m_apmHeader->size());
    if (bytes != static_cast<decltype(bytes)>(m_apmHeader->size())) {
        throw std::runtime_error("Failed to add partition.");
    }
    m_apmHeader = nullptr;
}

void PartitionTable::fillChs(char *chs, quint64 position) {
    Q_UNUSED(position);
    /**
     * It's guessed that the calculates values are correct but chs might as
     * well set to unused like so:
     */
    /*
    chs[0] = 0xff;
    chs[1] = 0xff;
    chs[2] = 0xef;
    */
    const int head = (position / NUM_SECTORS) % NUM_HEADS;
    const int sector = (position % NUM_SECTORS) + 1;
    const int cylinder = position / (NUM_HEADS * NUM_SECTORS);
    chs[0] = head & 0xff;
    // ccssssss
    chs[1] = ((cylinder >> 2) & 0xc0) | (sector & 0x3f);
    chs[2] = cylinder & 0xff;
}

int PartitionTable::addPartition(quint64 offset, quint64 size) {
    if (size == 0) {
        size = diskSize() - offset;
    }
    const quint32 lba = offset / SECTOR_SIZE;
    const quint32 count = size / SECTOR_SIZE;
    PartitionEntry entry;
    entry[0] = 0x0;
    fillChs(entry.data() + 1, offset);
    entry[4] = 0xb; // fat32
    fillChs(entry.data() + 5, offset + size);
    quint32 tmp = qToLittleEndian(lba);
    std::memcpy(entry.data() + 8, &tmp, sizeof(tmp));
    tmp = qToLittleEndian(count);
    std::memcpy(entry.data() + 12, &tmp, sizeof(tmp));

    const int num = m_entries.size() + 1;
    if (num > MAX_PARTITIONS) {
        throw std::runtime_error("Partition table is full.");
    }
    m_entries.append(std::move(entry));
    seekEntry(num - 1);
    auto bytes = ::write(m_fd, entry.data(), entry.size());
    if (bytes != static_cast<decltype(bytes)>(entry.size())) {
        throw std::runtime_error("Failed to add partition.");
    }
    return num;
}

#include <iostream>
void PartitionTable::formatPartition(quint64 offset, const QString &label, quint64 size) {
    /**
     * Currently unused because the label "OVERLAY    " is hardcoded into
     * formatPartition at the moment.
     * TODO(squimrel): Fix this.
     */
    Q_UNUSED(label);
    constexpr int RESERVED_SECTORS = 32;
    constexpr int NR_FATS = 2;
    auto getbyte = [](int number, int i) -> quint8 {
        return (number >> (8 * i)) & 0xff;
    };
    auto align = [](int number, int alignment) -> int {
        return (number + alignment - 1) & ~(alignment - 1);
    };
    auto divCeil = [](int a, int b) -> int {
        return (a + b - 1) / b;
    };

    const quint32 volumeId = QDateTime::currentMSecsSinceEpoch() & 0xffffffff;
    auto u = [&volumeId, &getbyte](int i) {
        return getbyte(volumeId, i);
    };
    /*
     * Determine the number of sectors per cluster.
     */
    quint64 sizeMb = size / (1024 * 1024);
    const std::array<int, 4> ranges = { 260, 1024 * 8, 1024 * 16, 1024 * 32 };
    auto found = qLowerBound(ranges.begin(), ranges.end(), sizeMb);
    const int sectorsPerCluster = found == ranges.begin() ? 1 : (found - ranges.begin()) * 8;
    const int num_sectors = size / SECTOR_SIZE;
    const quint64 fatdata = num_sectors - RESERVED_SECTORS;
    const int clusters = (fatdata * SECTOR_SIZE + NR_FATS * 8) / (sectorsPerCluster * SECTOR_SIZE + NR_FATS * 4);
    const int fatlength = align(divCeil((clusters + 2) * 4, SECTOR_SIZE), sectorsPerCluster);
    /**
     * Magic values were generated by mkfs.fat (dosfstools).
     */
    constexpr quint8 bootSign[] = { 0x55, 0xaa };
    constexpr quint8 infoSector[] = { 0x52, 0x52, 0x61, 0x61 };
    constexpr std::size_t fsinfoOffset = 0x1e0;
    const quint8 fsinfo[] = { 0x00, 0x00, 0x00, 0x00, 0x72, 0x72, 0x41, 0x61,
        0x8a, 0x7f, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00 };
    constexpr quint8 fat[] = {
        0xf8, 0xff, 0xff, 0x0f, 0xff, 0xff, 0xff, 0x0f, 0xf8, 0xff, 0xff, 0x0f
    };
    const quint8 clsz = sectorsPerCluster;
    const quint8 bootSector[] = { 0xeb, 0x58, 0x90, 0x6d, 0x6b, 0x66, 0x73,
        0x2e, 0x66, 0x61, 0x74, 0x00, 0x02, clsz, 0x20, 0x00, 0x02, 0x00, 0x00,
        0x00, 0x00, 0xf8, 0x00, 0x00, 0x3e, 0x00, 0xf7, 0x00, 0x00, 0x98, 0x2e,
        0x00, 0x00, 0x28, 0xc0, 0x00, 0xf8, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x29,
        u(3), u(2), u(1), u(0), 0x4f, 0x56, 0x45, 0x52, 0x4c, 0x41, 0x59, 0x20,
        0x20, 0x20, 0x20, 0x46, 0x41, 0x54, 0x33, 0x32, 0x20, 0x20, 0x20 };
    const quint8 bootCode[] = { 0x0e, 0x1f, 0xbe, 0x77, 0x7c, 0xac, 0x22, 0xc0,
        0x74, 0x0b, 0x56, 0xb4, 0x0e, 0xbb, 0x07, 0x00, 0xcd, 0x10, 0x5e, 0xeb,
        0xf0, 0x32, 0xe4, 0xcd, 0x16, 0xcd, 0x19, 0xeb, 0xfe, 0x54, 0x68, 0x69,
        0x73, 0x20, 0x69, 0x73, 0x20, 0x6e, 0x6f, 0x74, 0x20, 0x61, 0x20, 0x62,
        0x6f, 0x6f, 0x74, 0x61, 0x62, 0x6c, 0x65, 0x20, 0x64, 0x69, 0x73, 0x6b,
        0x2e, 0x20, 0x20, 0x50, 0x6c, 0x65, 0x61, 0x73, 0x65, 0x20, 0x69, 0x6e,
        0x73, 0x65, 0x72, 0x74, 0x20, 0x61, 0x20, 0x62, 0x6f, 0x6f, 0x74, 0x61,
        0x62, 0x6c, 0x65, 0x20, 0x66, 0x6c, 0x6f, 0x70, 0x70, 0x79, 0x20, 0x61,
        0x6e, 0x64, 0x0d, 0x0a, 0x70, 0x72, 0x65, 0x73, 0x73, 0x20, 0x61, 0x6e,
        0x79, 0x20, 0x6b, 0x65, 0x79, 0x20, 0x74, 0x6f, 0x20, 0x74, 0x72, 0x79,
        0x20, 0x61, 0x67, 0x61, 0x69, 0x6e, 0x20, 0x2e, 0x2e, 0x2e, 0x20, 0x0d,
        0x0a };
    constexpr std::size_t bootCodeSize = 420;
    const auto datetime = QDateTime::currentDateTimeUtc();
    const auto timeOnly = datetime.time();
    const auto dateOnly = datetime.date();
    quint64 time = ((timeOnly.second() + 1) >> 1) + (timeOnly.minute() << 5) + (timeOnly.hour() << 11);
    quint64 date = dateOnly.day() + (dateOnly.month() << 5) + (dateOnly.year() - 1980);
    const quint8 tlo = time & 0xff;
    const quint8 thi = time >> 8;
    const quint8 dlo = date & 0xff;
    const quint8 dhi = date >> 8;
    const quint8 rootDir[] = { 0x4f, 0x56, 0x45, 0x52, 0x4c, 0x41, 0x59, 0x20,
        0x20, 0x20, 0x20, 0x08, 0x00, 0x00, tlo, thi, dlo, dhi, dlo, dhi, 0x00,
        0x00, tlo, thi, dlo, dhi, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    seekto(offset);

    writeBytes<decltype(bootSector)>(bootSector);
    writeBytes<decltype(bootCode)>(bootCode);
    writeZeros(bootCodeSize - std::extent<decltype(bootCode)>::value);
    writeBytes<decltype(bootSign)>(bootSign);

    writeBytes<decltype(infoSector)>(infoSector);
    writeZeros(fsinfoOffset - std::extent<decltype(infoSector)>::value);
    writeBytes<decltype(fsinfo)>(fsinfo);
    writeZeros(14);
    writeBytes<decltype(bootSign)>(bootSign);

    writeZeros(SECTOR_SIZE * 4);

    writeBytes<decltype(bootSector)>(bootSector);
    writeBytes<decltype(bootCode)>(bootCode);
    writeZeros(bootCodeSize - std::extent<decltype(bootCode)>::value);
    writeBytes<decltype(bootSign)>(bootSign);

    writeZeros(SECTOR_SIZE * 25);
    writeBytes<decltype(fat)>(fat);
    writeZeros(SECTOR_SIZE - std::extent<decltype(fat)>::value);
    writeZeros(SECTOR_SIZE * fatlength);
    writeBytes<decltype(fat)>(fat);
    writeZeros(SECTOR_SIZE - std::extent<decltype(fat)>::value);
    writeZeros(SECTOR_SIZE * fatlength);
    writeBytes<decltype(rootDir)>(rootDir);
    writeZeros(SECTOR_SIZE * sectorsPerCluster - std::extent<decltype(rootDir)>::value);
}

quint64 PartitionTable::diskSize() {
    static quint64 size = 0;
    if (size != 0)
        return size;
    struct stat buf;
    ::fstat(m_fd, &buf);
    size = buf.st_size;
    return size;
}
