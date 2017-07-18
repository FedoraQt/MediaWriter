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

#include <QtGlobal>
#include <QtEndian>

PartitionTable::PartitionTable(int fd) : m_fd(fd) {
}

void PartitionTable::setFileDescriptor(int fd) {
    m_fd = fd;
}

void PartitionTable::read() {
    seek();
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

void PartitionTable::seek(std::size_t index) {
    if (::lseek(m_fd, PARTITION_ENTRY_OFFSET + PARTITION_ENTRY_SIZE * index, SEEK_SET) < 0) {
        throw std::runtime_error("Failed to seek to partition table.");
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
    seek(num - 1);
    auto bytes = ::write(m_fd, entry.data(), entry.size());
    if (bytes != static_cast<decltype(bytes)>(entry.size())) {
        throw std::runtime_error("Failed to add partition.");
    }
    return num;
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
