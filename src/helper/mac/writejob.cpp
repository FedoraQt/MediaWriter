/*
 * AOSC Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
 * Copyright (C) 2020 Jan Grulich <jgrulich@redhat.com>
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
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>
#include <QTimer>

#include <QDebug>

#include <fcntl.h>
#include <lzma.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "isomd5/libcheckisomd5.h"

AuthOpenProcess::AuthOpenProcess(int parentSocket, int clientSocket, const QString &device, QObject *parent)
    : QProcess(parent)
    , m_parentSocket(parentSocket)
    , m_clientSocket(clientSocket)
{
    setProgram(QStringLiteral("/usr/libexec/authopen"));
    setArguments({QStringLiteral("-stdoutpipe"), QStringLiteral("-o"), QString::number(O_RDWR), QStringLiteral("/dev/r") + device});

    setChildProcessModifier([=] {
        ::close(m_parentSocket);
        ::dup2(m_clientSocket, STDOUT_FILENO);
    });
}

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr)
    , what(what)
    , where(where)
{
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
    if (what.endsWith(".part")) {
        watcher.addPath(what);
    } else {
        QTimer::singleShot(0, this, &WriteJob::work);
    }
}

int WriteJob::staticOnMediaCheckAdvanced(void *data, long long offset, long long total)
{
    return ((WriteJob *)data)->onMediaCheckAdvanced(offset, total);
}

int WriteJob::onMediaCheckAdvanced(long long offset, long long total)
{
    Q_UNUSED(total)
    out << offset << "\n";
    out.flush();
    return 0;
}

void WriteJob::work()
{
    int sockets[2];
    int result = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
    if (result == -1) {
        err << tr("Unable to allocate socket pair") << "\n";
        err.flush();
        return;
    }

    QProcess diskUtil;
    diskUtil.setProgram("diskutil");
    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    AuthOpenProcess p(sockets[0], sockets[1], where);
    p.start(QIODevice::ReadOnly);

    close(sockets[1]);

    int fd = -1;
    const size_t bufferSize = sizeof(struct cmsghdr) + sizeof(int);
    char buffer[bufferSize];

    struct iovec io_vec[1];
    io_vec[0].iov_len = bufferSize;
    io_vec[0].iov_base = buffer;

    const socklen_t socketSize = static_cast<socklen_t>(CMSG_SPACE(sizeof(int)));
    char cmsg_socket[socketSize];

    struct msghdr message = {0};
    message.msg_iov = io_vec;
    message.msg_iovlen = 1;
    message.msg_control = cmsg_socket;
    message.msg_controllen = socketSize;

    ssize_t size = recvmsg(sockets[0], &message, 0);

    if (size > 0) {
        struct cmsghdr *socketHeader = CMSG_FIRSTHDR(&message);
        if (socketHeader && socketHeader->cmsg_level == SOL_SOCKET && socketHeader->cmsg_type == SCM_RIGHTS) {
            fd = *reinterpret_cast<int *>(CMSG_DATA(socketHeader));
        }
    }

    p.waitForFinished();

    if (fd == -1) {
        err << tr("Unable to open destination for writing") << "\n";
        err.flush();
        return;
    }

    out << "WRITE\n";
    out.flush();

    QFile target;
    target.open(fd, QIODevice::ReadWrite, QFileDevice::AutoCloseHandle);

    if (what.endsWith(".xz")) {
        if (!writeCompressed(target)) {
            return;
        }
    } else {
        if (!writePlain(target)) {
            return;
        }
    }

    check(target);
}

void WriteJob::onFileChanged(const QString &path)
{
    if (QFile::exists(path))
        return;

    what.replace(QRegularExpression("[.]part$"), "");

    work();
}

bool WriteJob::writePlain(QFile &target)
{
    qint64 bytesTotal = 0;

    QFile source(what);
    QByteArray buffer(BLOCK_SIZE, 0);

    out << -1 << "\n";
    out.flush();

    QProcess diskUtil;
    diskUtil.setProgram("diskutil");

    source.open(QIODevice::ReadOnly);

    while (source.isReadable() && !source.atEnd() && target.isWritable()) {
        qint64 bytes = source.read(buffer.data(), BLOCK_SIZE);
        bytesTotal += bytes;
        qint64 written = target.write(buffer.data(), bytes);
        if (written != bytes) {
            err << tr("Destination drive is not writable") << "\n";
            err.flush();
            qApp->exit(1);
            return true;
        }
        out << bytesTotal << "\n";
        out.flush();
    }

    target.flush();
    target.close();

    for (int i = 0; i < 5; i++) {
        diskUtil.setArguments(QStringList() << "disableJournal" << QString("%1s%2").arg(where).arg(i));
        diskUtil.start();
        diskUtil.waitForFinished();
    }

    diskUtil.setArguments(QStringList() << "unmountDisk" << where);
    diskUtil.start();
    diskUtil.waitForFinished();

    return true;
}

bool WriteJob::writeCompressed(QFile &target)
{
    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    uint8_t *inBuffer = new uint8_t[BLOCK_SIZE];
    uint8_t *outBuffer = new uint8_t[BLOCK_SIZE];

    QFile source(what);
    source.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        err << tr("Failed to start decompressing.");
        return false;
    }

    strm.next_in = inBuffer;
    strm.avail_in = 0;
    strm.next_out = outBuffer;
    strm.avail_out = BLOCK_SIZE;

    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = source.read((char *)inBuffer, BLOCK_SIZE);
            totalRead += len;

            strm.next_in = inBuffer;
            strm.avail_in = len;

            out << totalRead << "\n";
            out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            quint64 len = target.write((char *)outBuffer, BLOCK_SIZE - strm.avail_out);
            if (len != BLOCK_SIZE - strm.avail_out) {
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
            quint64 len = target.write((char *)outBuffer, BLOCK_SIZE - strm.avail_out);
            if (len != BLOCK_SIZE - strm.avail_out) {
                err << tr("Destination drive is not writable");
                qApp->exit(3);
                return false;
            }
            strm.next_out = outBuffer;
            strm.avail_out = BLOCK_SIZE;
        }
    }
}

void WriteJob::check(QFile &target)
{
    out << "CHECK\n";
    out.flush();
    switch (mediaCheckFD(target.handle(), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        out << "DONE\n";
        out.flush();
        err << "OK\n";
        err.flush();
        qApp->exit(0);
        break;
    case ISOMD5SUM_CHECK_FAILED:
        err << tr("Your drive is probably damaged.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    default:
        err << tr("Unexpected error occurred during media check.") << "\n";
        err.flush();
        qApp->exit(1);
        break;
    }

    qApp->exit();
}
