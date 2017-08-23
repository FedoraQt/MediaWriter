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

#ifndef BLOCKDEVICE_H
#define BLOCKDEVICE_H

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#include <unistd.h>

#include <array>
#include <memory>
#include <type_traits>
#include <utility>

#include <QString>
#include <QVector>
#include <QtGlobal>

#include "write.h"

class BlockDevice {
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
    void seekEntry(std::size_t index = 0);
    void seekto(std::size_t position);
    template <class T>
    void writeBytes(const T buffer);
    void writeZeros(std::size_t size);

public:
    BlockDevice(int fd);
    void read();
    int addPartition(quint64 offset, quint64 size);
    void formatOverlayPartition(quint64 offset, quint64 size);

private:
    int m_fd;
    std::size_t m_bytesWritten;
    std::size_t m_totalBytes;
    ProgressStats m_progress;
    QVector<PartitionEntry> m_entries;
};

/**
 * Originally the idea of having this template was so that one does not have to
 * pass in the size.
 * Poorly when type auto deduction is done type information is lost and the
 * resulting type is a pointer.
 * Therefore the user has to explicitly pass in the type.
 */
template <class T>
void BlockDevice::writeBytes(const T buffer) {
    const auto bytes = std::is_array<T>::value ? std::extent<T>::value : sizeof(typename std::remove_pointer<T>::type);
    const auto len = ::write(m_fd, buffer, bytes);
    if (len < 0) {
        throw std::runtime_error("Failed to write buffer to block device.");
    }
    m_bytesWritten += len;
    ::onProgress(&m_progress, m_bytesWritten, m_totalBytes);
}

#endif // BLOCKDEVICE_H
