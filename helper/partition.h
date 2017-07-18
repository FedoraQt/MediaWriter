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

#ifndef PARTITION_H
#define PARTITION_H

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <unistd.h>

#include <array>
#include <memory>
#include <utility>

#include <QVector>
#include <QtGlobal>

class PartitionTable {
private:
    static constexpr std::size_t APM_OFFSET = 2048;
    static constexpr std::size_t APM_SIZE = 2048 * 3;
    /*
     * Not doing fancy guessing and simply using the same values as isohybrid
     * uses.
     */
    static constexpr int NUM_HEADS = 64;
    static constexpr int NUM_SECTORS = 32;
    static constexpr int SECTOR_SIZE = 512;
    static constexpr int MAX_PARTITIONS = 4;
    static constexpr off_t PARTITION_ENTRY_OFFSET = 0x1be;
    static constexpr std::size_t PARTITION_ENTRY_SIZE = 16;
    static constexpr std::size_t PARTITION_TABLE_SIZE = MAX_PARTITIONS * PARTITION_ENTRY_SIZE;
    using PartitionEntry = std::array<char, PARTITION_ENTRY_SIZE>;

    quint64 diskSize();
    void fillChs(char *chs, quint64 position);
    void seek(std::size_t index = 0);

public:
    PartitionTable(int fd);
    void setFileDescriptor(int fd);
    void read();
    int addPartition(quint64 offset = 1024ULL * 1024ULL, quint64 size = 0);
    void wipeMac();
    void restoreMac();

private:
    int m_fd;
    QVector<PartitionEntry> m_entries;
    std::unique_ptr<std::array<char, APM_SIZE>> m_apmHeader;
};

#endif // PARTITION_H
