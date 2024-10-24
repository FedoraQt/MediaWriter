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

#include "writejob.h"

#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

#include <io.h>

#include <lzma.h>

#include "isomd5/libcheckisomd5.h"

static QString getLastError()
{
    TCHAR message[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message, 255, NULL);
    return QString::fromWCharArray(message).trimmed();
}

WriteJob::WriteJob(const QString &image, const QString &driveNumber, QObject *parent)
    : QObject(parent)
    , m_image(image)
{
    const int wmiDriveNumber = driveNumber.toInt();

    m_diskManagement = std::make_unique<WinDiskManagement>(this, true);
    m_disk = m_diskManagement->getDiskDriveInformation(wmiDriveNumber);

    if (m_image.endsWith(".part")) {
        connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &WriteJob::onFileChanged);
        m_watcher.addPath(m_image);
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
    Q_UNUSED(total);
    m_out << offset << "\n";
    m_out.flush();
    return 0;
}

void WriteJob::work()
{
    if (!write()) {
        qApp->exit(1);
        return;
    }

    if (!check()) {
        qApp->exit(1);
        return;
    }

    qApp->exit(0);
}

void WriteJob::onFileChanged(const QString &path)
{
    if (QFile::exists(path))
        return;
    QRegularExpression reg("[.]part$");
    m_image = m_image.replace(reg, "");

    m_out << "WRITE\n";
    m_out.flush();

    work();
}

bool WriteJob::write()
{
    m_diskManagement->logMessage(QtDebugMsg, "Preparing device for image writing");

    /*
     * Device preparation part
     */

    if (!m_diskManagement->removeDriveLetters(m_disk->index())) {
        m_err << tr("Couldn't remove drive mountpoints") << "\n";
        m_err.flush();
        return false;
    }

    // This doesn't need to be fatal and we can try to continue
    if (!m_diskManagement->clearPartitions(m_disk->index())) {
        m_err << tr("Failed to remove partitions from the drive") << "\n";
        m_err.flush();
    }

    HANDLE drive;
    const QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(m_disk->index());

    drive = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (drive == INVALID_HANDLE_VALUE) {
        m_err << tr("Failed to open the drive for formatting") << "\n";
        m_err.flush();
        return false;
    }

    if (!m_diskManagement->lockDrive(drive, 10)) {
        m_err << tr("Couldn't lock the drive") << "\n";
        m_err.flush();
        return false;
    }

    m_diskManagement->refreshPartitionLayout(drive);

    HANDLE logicalHandle = NULL;
    const QString logicalVolumePath = m_diskManagement->getLogicalName(m_disk->index(), false);
    if (!logicalVolumePath.isEmpty()) {
        m_diskManagement->logMessage(QtDebugMsg, "Trying to lock and unmount logical volume");
        logicalHandle = CreateFile(logicalVolumePath.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (logicalHandle == INVALID_HANDLE_VALUE) {
            m_err << tr("Couldn't open the logical handle") << "\n";
            m_err.flush();
            return false;
        }

        if (!m_diskManagement->lockDrive(logicalHandle, 10)) {
            m_err << tr("Couldn't lock the logical drive") << "\n";
            m_err.flush();
            return false;
        }

        // This doesn't need to be fatal and we can try to continue
        if (!m_diskManagement->unmountVolume(logicalHandle)) {
            m_err << tr("Couldn't unmount drive") << "\n";
            m_err.flush();
        }
    }

    if (!m_diskManagement->clearPartitionTable(drive, m_disk->size(), m_disk->sectorSize())) {
        m_err << tr("Failed to clear the partition table on the drive") << "\n";
        m_err.flush();
        return false;
    }

    if (!m_diskManagement->clearDiskDrive(drive)) {
        m_err << tr("Failed to set the drive to RAW partition style") << "\n";
        m_err.flush();
        return false;
    }

    m_diskManagement->unlockDrive(drive);
    CloseHandle(drive);

    // FIXME: isn't this too much? We used to have this even before as
    // apparently it was suggested after diskpart operations
    QThread::sleep(15);

    /*
     * Writing part
     */

    m_diskManagement->logMessage(QtDebugMsg, "Starting to write image file");

    // drive = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    drive = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, NULL);
    if (drive == INVALID_HANDLE_VALUE) {
        m_err << tr("Couldn't open the drive for writing") << "\n";
        m_err.flush();
        return false;
    }

    // m_diskManagement->disableIOBoundaryChecks(drive);

    if (!m_diskManagement->lockDrive(drive, 10)) {
        m_err << tr("Couldn't lock the drive") << "\n";
        m_err.flush();
        return false;
    }

    bool result;
    if (m_image.endsWith(".xz")) {
        result = writeCompressed(drive);
    } else {
        result = writePlain(drive);
    }

    if (result) {
        m_diskManagement->refreshPartitionLayout(drive);
    }

    if (logicalHandle && logicalHandle != INVALID_HANDLE_VALUE) {
        m_diskManagement->unlockDrive(logicalHandle);
        CloseHandle(logicalHandle);
    }
    CloseHandle(drive);

    if (!result) {
        qApp->exit(1);
        return false;
    }

    return true;
}

bool WriteJob::writeCompressed(HANDLE driveHandle)
{
    const qint64 blockSize = m_disk->sectorSize() * 512;

    QFile isoFile(m_image);
    isoFile.open(QIODevice::ReadOnly);
    if (!isoFile.isOpen()) {
        m_err << tr("Source image is not readable");
        m_err.flush();
        return false;
    }
    auto isoCleanup = qScopeGuard([&isoFile] {
        isoFile.close();
    });

    QFile drive;
    drive.open(_open_osfhandle(reinterpret_cast<intptr_t>(driveHandle), 0), QIODevice::WriteOnly | QIODevice::Unbuffered, QFile::AutoCloseHandle);
    if (!drive.isOpen()) {
        m_err << tr("Failed to open device for writing") << "\n";
        m_err.flush();
        return false;
    }
    auto driveCleanup = qScopeGuard([&drive] {
        drive.close();
    });

    void *outBuffer = NULL;
    outBuffer = VirtualAlloc(NULL, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (outBuffer == NULL) {
        m_err << tr("Failed to allocate the buffer") << "\n";
        m_err.flush();
        return false;
    }
    auto outBufferCleanup = qScopeGuard([outBuffer, blockSize] {
        VirtualFree(outBuffer, blockSize, MEM_DECOMMIT | MEM_RELEASE);
    });

    qint64 totalRead = 0;

    lzma_stream strm = LZMA_STREAM_INIT;
    lzma_ret ret;

    uint8_t *inBuffer = new uint8_t[blockSize];
    auto inBufferCleanup = qScopeGuard([inBuffer] {
        delete[] inBuffer;
    });

    QFile file(m_image);
    file.open(QIODevice::ReadOnly);

    ret = lzma_stream_decoder(&strm, MEDIAWRITER_LZMA_LIMIT, LZMA_CONCATENATED);
    if (ret != LZMA_OK) {
        m_err << tr("Failed to start decompressing.") << "\n";
        return false;
    }

    strm.next_in = inBuffer;
    strm.avail_in = 0;
    strm.next_out = static_cast<uint8_t *>(outBuffer);
    strm.avail_out = blockSize;

    const qint64 sectorSize = m_disk->sectorSize();
    while (true) {
        if (strm.avail_in == 0) {
            qint64 len = file.read((char *)inBuffer, blockSize);
            totalRead += len;

            strm.next_in = inBuffer;
            strm.avail_in = len;

            m_out << totalRead << "\n";
            m_out.flush();
        }

        ret = lzma_code(&strm, strm.avail_in == 0 ? LZMA_FINISH : LZMA_RUN);
        if (ret == LZMA_STREAM_END) {
            qint64 writtenBytes = 0;
            qint64 readBytes = blockSize - strm.avail_out;
            readBytes = ((readBytes + sectorSize - 1) / sectorSize) * sectorSize;
            writtenBytes = drive.write(reinterpret_cast<char *>(outBuffer), readBytes);

            if (writtenBytes <= 0) {
                m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Destination drive is not writable: %1").arg(getLastError()));
                m_err << tr("Destination drive is not writable") << ": " << getLastError() << "\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            if (writtenBytes != readBytes) {
                m_err << tr("The last block was not fully written") << "\n";
                m_err.flush();
                return false;
            }

            return true;
        }

        if (ret != LZMA_OK) {
            switch (ret) {
            case LZMA_MEM_ERROR:
                m_err << tr("There is not enough memory to decompress the file.") << "\n";
                break;
            case LZMA_FORMAT_ERROR:
            case LZMA_DATA_ERROR:
            case LZMA_BUF_ERROR:
                m_err << tr("The downloaded compressed file is corrupted.") << "\n";
                break;
            case LZMA_OPTIONS_ERROR:
                m_err << tr("Unsupported compression options.") << "\n";
                break;
            default:
                m_err << tr("Unknown decompression error.") << "\n";
                break;
            }
            qApp->exit(4);
            return false;
        }

        if (strm.avail_out == 0) {
            qint64 writtenBytes = 0;
            writtenBytes = drive.write(reinterpret_cast<char *>(outBuffer), sectorSize);
            if (writtenBytes <= 0) {
                m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Destination drive is not writable: %1").arg(getLastError()));
                m_err << tr("Destination drive is not writable") << ": " << getLastError() << "\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            if (writtenBytes != sectorSize) {
                m_err << tr("The last block was not fully written") << "\n";
                m_err.flush();
                qApp->exit(1);
                return false;
            }

            strm.next_out = static_cast<uint8_t *>(outBuffer);
            strm.avail_out = blockSize;
        }
    }
    return false;
}

bool WriteJob::writePlain(HANDLE driveHandle)
{
    const qint64 blockSize = m_disk->sectorSize() * 512;

    QFile isoFile(m_image);
    isoFile.open(QIODevice::ReadOnly);
    if (!isoFile.isOpen()) {
        m_err << tr("Source image is not readable") << "\n";
        m_err.flush();
        return false;
    }
    auto isoCleanup = qScopeGuard([&isoFile] {
        isoFile.close();
    });

    QFile drive;
    drive.open(_open_osfhandle(reinterpret_cast<intptr_t>(driveHandle), 0), QIODevice::WriteOnly /*| QIODevice::Unbuffered*/, QFile::AutoCloseHandle);
    if (!drive.isOpen()) {
        m_err << tr("Failed to open device for writing") << "\n";
        m_err.flush();
        return false;
    }
    auto driveCleanup = qScopeGuard([&drive] {
        drive.close();
    });

    uint8_t *buffer = NULL;
    buffer = static_cast<uint8_t *>(_mm_malloc(blockSize, m_disk->sectorSize()));
    if (!buffer) {
        m_err << tr("Failed to allocate the buffer") << "\n";
        m_err.flush();
        return false;
    }
    auto bufferCleanup = qScopeGuard([buffer] {
        if (buffer) {
            _mm_free(buffer);
        }
    });

    qint64 sectorSize = m_disk->sectorSize();
    qint64 totalBytes = 0;
    qint64 readBytes;
    qint64 writtenBytes;
    while (true) {
        if ((readBytes = isoFile.read(reinterpret_cast<char *>(buffer), blockSize)) <= 0) {
            break;
        }

        readBytes = ((readBytes + sectorSize - 1) / sectorSize) * sectorSize;
        writtenBytes = drive.write(reinterpret_cast<char *>(buffer), readBytes);
        if (writtenBytes <= 0) {
            m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Destination drive is not writable: %1").arg(getLastError()));
            m_err << tr("Destination drive is not writable") << ": " << getLastError() << "\n";
            m_err.flush();
            return false;
        }

        if (writtenBytes != readBytes) {
            m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("The last block was not fully written: %1").arg(getLastError()));
            m_err << tr("The last block was not fully written") << "\n";
            m_err.flush();
            return false;
        }

        totalBytes += readBytes;
        m_out << totalBytes << "\n";
        m_out.flush();

        if (readBytes != blockSize || isoFile.atEnd()) {
            break;
        }
    }

    if (readBytes < 0) {
        m_err << tr("Failed to read the image file: ") << isoFile.errorString() << "\n";
        m_err.flush();
        return false;
    }

    return true;
}

bool WriteJob::check()
{
    m_out << "CHECK\n";
    m_out.flush();

    const QString drivePath = QString("\\\\.\\PhysicalDrive%0").arg(m_disk->index());
    HANDLE drive = CreateFile(drivePath.toStdWString().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (drive == INVALID_HANDLE_VALUE) {
        m_err << tr("Couldn't open the drive for data verification for %1:").arg(drivePath) << " (" << getLastError() << ")\n";
        m_err.flush();
        qApp->exit(1);
        return false;
    }
    auto driveloseGuard = qScopeGuard([&drive] {
        CloseHandle(drive);
    });

    switch (mediaCheckFD(_open_osfhandle(reinterpret_cast<intptr_t>(drive), 0), &WriteJob::staticOnMediaCheckAdvanced, this)) {
    case ISOMD5SUM_CHECK_NOT_FOUND:
    case ISOMD5SUM_CHECK_PASSED:
        m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Check passed"));
        m_out << "DONE\n";
        m_out.flush();
        m_err << "OK\n";
        m_err.flush();
        return true;
    case ISOMD5SUM_CHECK_FAILED:
        m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Check failed"));
        m_err << tr("Your drive is probably damaged.") << "\n";
        m_err.flush();
        return false;
    default:
        m_diskManagement->logMessage(QtCriticalMsg, QStringLiteral("Check unexpected error"));
        m_err << tr("Unexpected error occurred during media check.") << "\n";
        m_err.flush();
        return false;
    }

    return true;
}
