/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

#include <libwindisk/windisk.h>

#include <QFileSystemWatcher>
#include <QObject>
#include <QTextStream>

#include <windows.h>

#ifndef MEDIAWRITER_LZMA_LIMIT
// 256MB memory limit for the decompressor
#define MEDIAWRITER_LZMA_LIMIT (1024 * 1024 * 256)
#endif

class WriteJob : public QObject
{
    Q_OBJECT
public:
    explicit WriteJob(const QString &image, const QString &driveNumber, QObject *parent);

    static int staticOnMediaCheckAdvanced(void *data, long long offset, long long total);
    int onMediaCheckAdvanced(long long offset, long long total);

private:
    bool check();
    bool write();
    bool writeCompressed(HANDLE driveHandle);
    bool writePlain(HANDLE driveHandle);

private slots:
    void onFileChanged(const QString &path);
    void work();

private:
    QString m_image;
    std::unique_ptr<WinDiskManagement> m_diskManagement;
    std::unique_ptr<WinDisk> m_disk;

    QTextStream m_out{stdout};
    QTextStream m_err{stderr};

    QFileSystemWatcher m_watcher;
};

#endif // WRITEJOB_H
