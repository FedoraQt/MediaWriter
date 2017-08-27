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

#ifndef PAGE_ALIGNED_BUFFER_H
#define PAGE_ALIGNED_BUFFER_H

#include <memory>
#include <tuple>
#include <utility>

#ifdef Q_OS_WIN
#include <windows.h>
std::size_t getpagesize() {
    static std::size_t pageSize = 0;
    if (pageSize == 0) {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);
        pageSize = static_cast<std::size_t>(systemInfo.dwPageSize);
    }
    return pageSize;
}
#else
#include <unistd.h>
#endif

/**
 * If the page cache size would be known at compile-time this would be much
 * easier to solve since std::aligned_alloc could simply be used.
 *
 * This class owns the memory used by the page aligned buffers.
 */
template <std::size_t NUM_BUFFERS>
class PageAlignedBuffer {
public:
    std::size_t size;
    /**
     * Since the size is provided in pages only all following buffers are
     * guaranteed to be aligned.
     */
    PageAlignedBuffer(const std::size_t pages = 1);
    void *get(const std::size_t num);

private:
    std::unique_ptr<char[]> owner;
    void *alignedBuffer;
};

template <std::size_t NUM_BUFFERS>
PageAlignedBuffer<NUM_BUFFERS>::PageAlignedBuffer(const std::size_t pages) {
    static const std::size_t pagesize = getpagesize();
    size = pages * pagesize;
    const std::size_t bufferSize = size * NUM_BUFFERS;
    std::size_t space = bufferSize + pagesize;
    owner = std::unique_ptr<char[]>(new char[space]);
    alignedBuffer = owner.get();
    std::align(pagesize, bufferSize, alignedBuffer, space);
}

/**
 * The caller must use this method and the returned buffer with care since
 * it won't do bounds checks.
 */
template <std::size_t NUM_BUFFERS>
void *PageAlignedBuffer<NUM_BUFFERS>::get(const std::size_t num) {
    return static_cast<char*>(alignedBuffer) + num * size;
}

#endif // PAGE_ALIGNED_BUFFER_H
