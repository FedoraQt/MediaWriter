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

#ifndef WRITEJOB_H
#define WRITEJOB_H

#include <memory>

#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include <QTextStream>

// Platform specific drive handler.
#include "drive.h"

#ifndef MEDIAWRITER_LZMA_LIMIT
// 256MB memory limit for the decompressor
#define MEDIAWRITER_LZMA_LIMIT (1024 * 1024 * 256)
#endif

class WriteJob : public QObject {
    Q_OBJECT
public:
    WriteJob(const QString &what, const QString &where);

    static int staticOnMediaCheckAdvanced(void *data, long long offset, long long total);
    int onMediaCheckAdvanced(long long offset, long long total);
    void work();
    void write();
    void writeCompressed();
    void writePlain();
    void check();
public slots:
    void boot();
    void onFileChanged(const QString &path);

private:
    QString what;
    QTextStream out;
    QTextStream err;
    std::unique_ptr<Drive> drive;
    QFileSystemWatcher watcher;
};

#endif // WRITEJOB_H
