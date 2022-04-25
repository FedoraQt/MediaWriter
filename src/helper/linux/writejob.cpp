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

#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>
#include <QtGlobal>

#include <QtDBus>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>

#include <fcntl.h>
#include <unistd.h>

#include <tuple>
#include <utility>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"

#include <QDebug>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where)
{
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
    QTimer::singleShot(0, this, SLOT(work()));
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total) {
    return ((WriteJob*)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total) {
    Q_UNUSED(total);
    out << offset << "\n";
    out.flush();
    return 0;
}

QDBusUnixFileDescriptor WriteJob::getDescriptor() {
    QDBusInterface device("org.freedesktop.UDisks2", where, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QString drivePath = qvariant_cast<QDBusObjectPath>(device.property("Drive")).path();
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");

    if (message.arguments().length() == 1) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
        DBusIntrospection objects;
        arg >> objects;
        for (auto i : objects.keys()) {
            if (objects[i].contains("org.freedesktop.UDisks2.Filesystem")) {
                QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
                if (currentDrivePath == drivePath) {
                    QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
                    message = partition.call("Unmount", Properties { {"force", true} });
                }
            }
        }
    }
    else {
        err << message.errorMessage();
        err.flush();
        qApp->exit(2);
        return QDBusUnixFileDescriptor(-1);
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = device.call(QDBus::Block, "OpenDevice", "rw", Properties{{"flags", O_DIRECT | O_SYNC | O_CLOEXEC}} );
    QDBusUnixFileDescriptor fd = reply.value();

    if (!fd.isValid()) {
        err << reply.error().message();
        err.flush();
        qApp->exit(2);
        return QDBusUnixFileDescriptor(-1);
    }

    return fd;
}

bool WriteJob::write(int fd) {
    if (what.endsWith(".xz"))
        return writeCompressed(fd);
    else
        return writePlain(fd);
}

bool WriteJob::writeCompressed(int fd) {
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    auto inBufferOwner = pageAlignedBuffer();
    char *inBuffer = std::get<1>(inBufferOwner);

    auto outBufferOwner = pageAlignedBuffer();
    char *outBuffer = std::get<1>(outBufferOwner);
    std::size_t bufferSize = std::get<2>(outBufferOwner);

    QFile file(what);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = reinterpret_cast<uint8_t*>(inBuffer);
    strm.avail_in = 0;
    strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
    strm.avail_out = bufferSize;

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
            quint64 len = ::write(fd, outBuffer, bufferSize - strm.avail_out);
            if (len != bufferSize - strm.avail_out) {
                err << tr("Destination drive is not writable");
                qApp->exit(3);
                return false;
            }
            return true;
        }
        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                err << tr("There is not enough memory to decompress the file.");
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                err << tr("The downloaded compressed file is corrupted.");
                break;
            case LZMA_OPTIONS_ERROR:
                err << tr("Unsupported compression options.");
                break;
            default:
                err << tr("Unknown decompression error.");
                break;
            }
            qApp->exit(4);
            return false;
        }

        if (strm.avail_out == 0) {
            quint64 len = ::write(fd, outBuffer, bufferSize - strm.avail_out);
            if (len != bufferSize - strm.avail_out) {
                err << tr("Destination drive is not writable");
                qApp->exit(3);
                return false;
            }
            strm.next_out = reinterpret_cast<uint8_t*>(outBuffer);
            strm.avail_out = bufferSize;
        }
    }
}

bool WriteJob::writePlain(int fd) {
    QFile inFile(what);
    inFile.open(QIODevice::ReadOnly);

    if (!inFile.isReadable()) {
        err << tr("Source image is not readable") << what;
        err.flush();
        qApp->exit(2);
        return false;
    }

    // Doing this so that ownership will kept in std::get<0>(bufferOwner).
    auto bufferOwner = pageAlignedBuffer();
    char *buffer = std::get<1>(bufferOwner);
    qint64 size = std::get<2>(bufferOwner);
    qint64 total = 0;

    while(!inFile.atEnd()) {
        qint64 len = inFile.read(buffer, size);
        if (len < 0) {
            err << tr("Source image is not readable");
            err.flush();
            qApp->exit(3);
            return false;
        }
        qint64 written = ::write(fd, buffer, len);
        if (written != len) {
            err << tr("Destination drive is not writable");
            err.flush();
            qApp->exit(3);
            return false;
        }
        total += len;
        out << total << '\n';
        out.flush();
    }

    inFile.close();
    sync();

    return true;
}

bool WriteJob::check(int fd) {
    out << "CHECK\n";
    out.flush();
    switch (mediaCheckFD(fd, &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "DONE\n";
        out.flush();
        err << "OK\n";
        err.flush();
        qApp->exit(0);
        return false;
    case ISOMD5SUM_CHECK_FAILED:
        err << tr("Your drive is probably damaged.") << "\n";
        err.flush();
        qApp->exit(1);
        return false;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        return false;
    }
    return true;
}

void WriteJob::work() {
    // have to keep the QDBus wrapper, otherwise the file gets closed
    fd = getDescriptor();
    if (fd.fileDescriptor() < 0)
        return;

    if (what.endsWith(".part")) {
        watcher.addPath(what);
    }
    else {
        if (!write(fd.fileDescriptor()))
            return;

        check(fd.fileDescriptor());
    }
}

void WriteJob::onFileChanged(const QString &path) {
    if (QFile::exists(path))
        return;

    what.replace(QRegularExpression(".part$"), "");

    if (!QFile::exists(what)) {
        qApp->exit(4);
        return;
    }

    //to immediately trigger the UI into writing mode
    out << "WRITE\n";
    out.flush();
    out << "1\n";
    out.flush();

    if (!write(fd.fileDescriptor())) {
        qApp->exit(4);
        return;
    }

    check(fd.fileDescriptor());
}

std::tuple<std::unique_ptr<char[]>, char*, std::size_t> pageAlignedBuffer(std::size_t pages) {
    static const std::size_t pagesize = getpagesize();
    const std::size_t size = pages * pagesize;
    std::size_t space = size + pagesize;
    std::unique_ptr<char[]> owner(new char[space]);
    void *buffer = owner.get();
    Q_ASSERT(space - size == pagesize);
    void *ptr = std::align(pagesize, size, buffer, space);
    // This should never fail since std::align should not return a nullptr if the assertion above passes.
    Q_CHECK_PTR(ptr);
    return std::make_tuple(std::move(owner), static_cast<char*>(buffer), size);
};
