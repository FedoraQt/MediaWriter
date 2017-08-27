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

#include <unistd.h>

#include <QTextStream>
#include <QtGlobal>

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
