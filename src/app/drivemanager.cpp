/*
 * Fedora Media Writer
 * Copyright (C) 2026 Jan Grulich <jgrulich@redhat.com
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

#include "drivemanager.h"
#include "notifications.h"
#include "utilities.h"

#ifdef __linux__
#include "linuxdrivemanager.h"
#endif // __linux__

#ifdef __APPLE__
#include "macdrivemanager.h"
#endif // __APPLE__

#ifdef _WIN32
#include "windrivemanager.h"
#endif // _WIN32

#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QtQml>

DriveManager *DriveManager::_self = nullptr;

void DriveOperationDeleter::operator()(QProcess *process)
{
    if (process->state() != QProcess::NotRunning) {
#ifdef Q_OS_WIN
        process->kill();
        process->waitForFinished(3000);
#else
        process->terminate();
        if (!process->waitForFinished(3000)) {
            process->kill();
            process->waitForFinished(3000);
        }
#endif
    }

    delete process;
}

DriveManager::DriveManager(QObject *parent)
    : QAbstractListModel(parent)
    , m_provider(DriveProvider::create(this))
{
    mDebug() << this->metaObject()->className() << "construction";
    qmlRegisterUncreatableType<Drive>("MediaWriter", 1, 0, "Drive", "");

    connect(m_provider, &DriveProvider::driveConnected, this, &DriveManager::onDriveConnected);
    connect(m_provider, &DriveProvider::driveRemoved, this, &DriveManager::onDriveRemoved);
    connect(m_provider, &DriveProvider::backendBroken, this, &DriveManager::onBackendBroken);
}

DriveManager *DriveManager::instance()
{
    if (!_self) {
        _self = new DriveManager();
    }

    return _self;
}

QVariant DriveManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);

    if (role == DriveRole) {
        return "drive";
    } else if (role == DriveNameRole) {
        return "display";
    }

    return QVariant();
}

QHash<int, QByteArray> DriveManager::roleNames() const
{
    QHash<int, QByteArray> ret;
    ret.insert(DriveRole, "drive");
    ret.insert(DriveNameRole, "display");
    return ret;
}

int DriveManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_drives.count();
}

QVariant DriveManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == DriveRole) {
        return QVariant::fromValue(m_drives[index.row()]);
    } else if (role == DriveNameRole) {
        return QVariant::fromValue(m_drives[index.row()]->name());
    }

    return QVariant();
}

Drive *DriveManager::selected() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_drives.count()) {
        return m_drives[m_selectedIndex];
    }
    return nullptr;
}

int DriveManager::selectedIndex() const
{
    return m_selectedIndex;
}

void DriveManager::setSelectedIndex(int index)
{
    if (m_selectedIndex != index && index < m_drives.count() && index >= 0) {
        m_selectedIndex = index;
        Q_EMIT selectedChanged();
    }
}

int DriveManager::length() const
{
    return m_drives.count();
}

Drive *DriveManager::lastRestoreable()
{
    return m_lastRestoreable;
}
bool DriveManager::isBackendBroken()
{
    return !m_errorString.isEmpty();
}

QString DriveManager::errorString()
{
    return m_errorString;
}

void DriveManager::setLastRestoreable(Drive *drive)
{
    if (m_lastRestoreable != drive) {
        m_lastRestoreable = drive;
        Q_EMIT restoreableDriveChanged();
    }
}

void DriveManager::onDriveConnected(Drive *drive)
{
    int position = 0;
    for (auto i : m_drives) {
        if (drive->size() < i->size()) {
            break;
        }
        position++;
    }

    beginInsertRows(QModelIndex(), position, position);
    m_drives.insert(position, drive);
    endInsertRows();
    Q_EMIT drivesChanged();

    if (selected() && selected()->isBusy()) {
        return;
    }

    if (m_provider->initialized()) {
        m_selectedIndex = position;
        Q_EMIT selectedChanged();
    } else {
        m_selectedIndex = 0;
        Q_EMIT selectedChanged();
    }

    connect(drive, &Drive::restoreStatusChanged, this, [=]() {
        if (drive->restoreStatus() == Drive::CONTAINS_LIVE) {
            setLastRestoreable(drive);
        } else if (drive == m_lastRestoreable) {
            setLastRestoreable(nullptr);
        }
    });

    if (drive->restoreStatus() == Drive::CONTAINS_LIVE) {
        setLastRestoreable(drive);
    }
}

void DriveManager::onDriveRemoved(Drive *drive)
{
    const int i = m_drives.indexOf(drive);
    if (i >= 0) {
        beginRemoveRows(QModelIndex(), i, i);
        m_drives.removeAt(i);
        endRemoveRows();
        Q_EMIT drivesChanged();

        if (i == m_selectedIndex) {
            m_selectedIndex = 0;
        }
        Q_EMIT selectedChanged();

        if (drive == m_lastRestoreable) {
            setLastRestoreable(nullptr);
        }
    }
}

void DriveManager::onBackendBroken(const QString &message)
{
    m_errorString = message;
    Q_EMIT isBackendBrokenChanged();
}

DriveProvider *DriveProvider::create(DriveManager *parent)
{
#ifdef __APPLE__
    return new MacDriveProvider(parent);
#endif // APPLE

#ifdef _WIN32
    return new WinDriveProvider(parent);
#endif // _WIN32

#ifdef __linux__
    return new LinuxDriveProvider(parent);
#endif // linux
}

bool DriveProvider::initialized() const
{
    return m_initialized;
}

DriveProvider::DriveProvider(DriveManager *parent)
    : QObject(parent)
{
}

Drive::Drive(DriveProvider *parent, const QString &deviceName, const QString &deviceIdentifier, const QString &serialNumber, uint64_t size, bool containsLive)
    : QObject(parent)
    , m_progress(new Progress(this))
    , m_deviceName(deviceName)
    , m_deviceIdentifier(deviceIdentifier)
    , m_serialNumber(serialNumber)
    , m_size(size)
    , m_restoreStatus(containsLive ? CONTAINS_LIVE : CLEAN)
{
}

Drive::~Drive()
{
    if (m_image && (m_image->status() == ReleaseVariant::WRITING || m_image->status() == ReleaseVariant::WRITE_VERIFYING)) {
        m_image->setErrorString(tr("The drive was removed while it was written to."));
        m_image->setStatus(ReleaseVariant::FAILED);
    }
}

Progress *Drive::progress() const
{
    return m_progress;
}

void Drive::updateDrive(const QString &name, uint64_t size, bool containsLive)
{
    m_deviceName = name;
    m_size = size;
    // Only update status when we are not in the "restoration" process
    if (m_restoreStatus == CONTAINS_LIVE || m_restoreStatus == CLEAN)
        setRestoreStatus(containsLive ? CONTAINS_LIVE : CLEAN);
}

QString Drive::name() const
{
    return QString("%1 (%2)").arg(m_deviceName).arg(readableSize());
}

QString Drive::identifier() const
{
    return m_deviceIdentifier;
}

QString Drive::serialNumber() const
{
    return m_serialNumber;
}

QString Drive::readableSize() const
{
    QString sizeStr;
    if (m_size < (1000UL)) {
        sizeStr = QString("%1 B").arg(m_size);
    } else if (m_size < (1000000UL)) {
        sizeStr = QString("%1 KB").arg(m_size / 1000.0, 0, 'f', 1);
    } else if (m_size < (1000000000UL)) {
        sizeStr = QString("%1 MB").arg(m_size / 1000000.0, 0, 'f', 1);
    } else if (m_size < (1000000000000UL)) {
        sizeStr = QString("%1 GB").arg(m_size / 1000000000.0, 0, 'f', 1);
    } else if (m_size < (1000000000000000UL)) {
        sizeStr = QString("%1 TB").arg(m_size / 1000000000000.0, 0, 'f', 1);
    } else { // better be ready for exabyte images and drives
        sizeStr = QString("%1 EB").arg(m_size / 1000000000000000.0, 0, 'f', 1);
    }
    return sizeStr;
}

qreal Drive::size() const
{
    return m_size;
}

Drive::RestoreStatus Drive::restoreStatus()
{
    return m_restoreStatus;
}

bool Drive::isBusy() const
{
    return m_process && m_process->state() != QProcess::NotRunning;
}

void Drive::setImage(ReleaseVariant *data)
{
    m_image = data;
    if (m_image)
        m_image->setErrorString(QString());
}

static QString helperPath()
{
#if defined(__linux__)
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper"))
        return qApp->applicationDirPath() + "/../helper/linux/helper";
    if (QFile::exists(qApp->applicationDirPath() + "/helper"))
        return qApp->applicationDirPath() + "/helper";
    if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper")))
        return QString("%1/%2").arg(LIBEXECDIR).arg("helper");
#elif defined(__APPLE__)
    if (QFile::exists(qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper"))
        return qApp->applicationDirPath() + "/../../../../helper/mac/helper.app/Contents/MacOS/helper";
    if (QFile::exists(qApp->applicationDirPath() + "/helper"))
        return qApp->applicationDirPath() + "/helper";
#elif defined(_WIN32)
    if (QFile::exists(qApp->applicationDirPath() + "/helper.exe"))
        return qApp->applicationDirPath() + "/helper.exe";
    if (QFile::exists(qApp->applicationDirPath() + "/../helper.exe"))
        return qApp->applicationDirPath() + "/../helper.exe";
#endif
    return QString();
}

bool Drive::write(ReleaseVariant *data)
{
    m_image = data;
    m_image->setErrorString(QString());
    if (data && data->size() > 0 && size() > 0 && data->realSize() > size()) {
        m_image->setErrorString(tr("This drive is not large enough."));
        return false;
    }

    const QString path = helperPath();
    if (path.isEmpty()) {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        data->setStatus(ReleaseVariant::FAILED);
        return false;
    }

    if (m_image->status() != ReleaseVariant::DOWNLOADING && m_image->status() != ReleaseVariant::DOWNLOAD_VERIFYING)
        m_image->setStatus(ReleaseVariant::WRITING);

    QStringList args;
    args << "write";
    if (data->status() == ReleaseVariant::WRITING)
        args << data->iso();
    else
        args << data->temporaryPath();
    args << m_deviceIdentifier;

    m_process.reset(new QProcess());
    m_process->setProgram(path);
    m_process->setArguments(args);
    mDebug() << this->metaObject()->className() << "Helper command:" << path << args;

    connect(m_process.get(), &QProcess::readyRead, this, &Drive::onWriteReadyRead);
    connect(m_process.get(), &QProcess::finished, this, &Drive::onWriteFinished);
    connect(m_process.get(), &QProcess::errorOccurred, this, &Drive::onErrorOccurred);
    connect(qApp, &QCoreApplication::aboutToQuit, m_process.get(), &QProcess::terminate);

    m_process->start(QIODevice::ReadOnly);
    return true;
}

void Drive::cancel()
{
    m_error = QString();

    if (m_process && m_image) {
        if (m_image->status() == ReleaseVariant::WRITE_VERIFYING) {
            m_image->setStatus(ReleaseVariant::FINISHED);
        } else {
            m_image->setErrorString(tr("Stopped before writing has finished."));
            m_image->setStatus(ReleaseVariant::FAILED);
        }
        setRestoreStatus(CONTAINS_LIVE);
    } else {
        setRestoreStatus(CLEAN);
    }

    m_process.reset();
}

void Drive::restore()
{
    const QString path = helperPath();
    if (path.isEmpty()) {
        setRestoreStatus(RESTORE_ERROR);
        return;
    }

    setRestoreStatus(RESTORING);

    QStringList args;
    args << "restore" << m_deviceIdentifier;

    m_process.reset(new QProcess());
    m_process->setProgram(path);
    m_process->setArguments(args);
    mDebug() << this->metaObject()->className() << "Helper command:" << path << args;

    m_progress->setValue(0);
    m_progress->setTo(100);

    connect(m_process.get(), &QProcess::readyRead, this, &Drive::onRestoreReadyRead);
    connect(m_process.get(), &QProcess::finished, this, &Drive::onRestoreFinished);
    connect(qApp, &QCoreApplication::aboutToQuit, m_process.get(), &QProcess::terminate);

    m_process->start(QIODevice::ReadOnly);
}

bool Drive::operator==(const Drive &drive) const
{
    return name() == drive.name() && size() == drive.size() && serialNumber() == drive.serialNumber();
}

void Drive::onWriteReadyRead()
{
    if (!m_process) {
        return;
    }

    m_progress->setTo(m_image->size());
    m_progress->setValue(qQNaN());

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING) {
        m_image->setStatus(ReleaseVariant::WRITING);
    }

    while (m_process->bytesAvailable() > 0) {
        const QString line = m_process->readLine().trimmed();
        if (line == "CHECK") {
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITE_VERIFYING);
        } else if (line == "WRITE") {
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITING);
        } else if (line == "DONE") {
            m_progress->setValue(m_image->size());
            m_image->setStatus(ReleaseVariant::FINISHED);
            Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
        } else {
            bool ok;
            const qint64 bytes = line.toLongLong(&ok);
            if (ok) {
                if (bytes < 0) {
                    m_progress->setValue(qQNaN());
                } else {
                    m_progress->setValue(bytes);
                }
            }
        }
    }
}

void Drive::onWriteFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus)

    if (!m_process) {
        return;
    }

    mDebug() << this->metaObject()->className() << "Helper process finished with exit code" << exitCode;

    if (exitCode == 0) {
        if (m_image->status() != ReleaseVariant::FINISHED) {
            Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
            m_image->setStatus(ReleaseVariant::FINISHED);
        }
    } else {
        const QString errorMessage = m_process->readAllStandardError().trimmed();
        mWarning() << "Writing failed:" << errorMessage;
        Notifications::notify(tr("Error"), tr("Writing %1 failed").arg(m_image->fullName()));
        if (m_image->status() == ReleaseVariant::WRITING) {
            m_image->setErrorString(errorMessage);
            m_image->setStatus(ReleaseVariant::FAILED);
        }
    }

    m_process.reset();
    m_image = nullptr;
}

void Drive::onRestoreReadyRead()
{
    if (!m_process) {
        return;
    }

    while (m_process->bytesAvailable() > 0) {
        bool ok;
        const qint64 value = m_process->readLine().trimmed().toLongLong(&ok);
        if (ok && value >= 0) {
            m_progress->setValue(value);
        }
    }
}

void Drive::onRestoreFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    mDebug() << this->metaObject()->className() << "Helper process finished with exit code" << exitCode << exitStatus;

    if (exitCode != 0) {
        mWarning() << "Drive restoration failed:" << m_process->readAllStandardError();
    }

    setRestoreStatus(exitCode == 0 ? RESTORED : RESTORE_ERROR);
    m_process.reset();
}

void Drive::onErrorOccurred(QProcess::ProcessError error)
{
    Q_UNUSED(error)

    if (!m_process) {
        return;
    }

    const QString errorMessage = m_process->errorString();
    mWarning() << "Writing failed:" << errorMessage;
    if (m_image) {
        m_image->setErrorString(errorMessage);
        m_image->setStatus(ReleaseVariant::FAILED);
    }
    m_process.reset();
    m_image = nullptr;
}

void Drive::setRestoreStatus(Drive::RestoreStatus status)
{
    if (m_restoreStatus != status) {
        m_restoreStatus = status;
        Q_EMIT restoreStatusChanged();
    }
}
