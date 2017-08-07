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

#include "drivemanager.h"

#ifdef __linux__
# include "linuxdrivemanager.h"
#endif // __linux__

#ifdef __APPLE__
# include "macdrivemanager.h"
#endif // __APPLE__

#ifdef _WIN32
# include "windrivemanager.h"
#endif // _WIN32

#include "fakedrivemanager.h"

#include <QtQml>

#include "notifications.h"

DriveManager *DriveManager::_self = nullptr;

DriveManager::DriveManager(QObject *parent)
    : QAbstractListModel(parent), m_provider(DriveProvider::create(this))
{
    qDebug() << this->metaObject()->className() << "construction";
    qmlRegisterUncreatableType<Drive>("MediaWriter", 1, 0, "Drive", "");

    connect(m_provider, &DriveProvider::driveConnected, this, &DriveManager::onDriveConnected);
    connect(m_provider, &DriveProvider::driveRemoved, this, &DriveManager::onDriveRemoved);
    connect(m_provider, &DriveProvider::backendBroken, this, &DriveManager::onBackendBroken);
}

DriveManager *DriveManager::instance() {
    if (!_self)
        _self = new DriveManager();
    return _self;
}

QVariant DriveManager::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section); Q_UNUSED(orientation);

    if (role == Qt::UserRole + 1)
        return "drive";
    if (role == Qt::UserRole + 2)
        return "display";

    return QVariant();
}

QHash<int, QByteArray> DriveManager::roleNames() const {
    QHash<int, QByteArray> ret;
    ret.insert(Qt::UserRole + 1, "drive");
    ret.insert(Qt::UserRole + 2, "display");
    return ret;
}

int DriveManager::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_drives.count();
}

QVariant DriveManager::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(m_drives[index.row()]);

    if (role == Qt::UserRole + 2)
        return QVariant::fromValue(m_drives[index.row()]->name());

    return QVariant();
}

Drive *DriveManager::selected() const {
    if (m_selectedIndex >= 0 && m_selectedIndex < m_drives.count())
        return m_drives[m_selectedIndex];
    return nullptr;
}

int DriveManager::selectedIndex() const {
    return m_selectedIndex;
}

void DriveManager::setSelectedIndex(int o) {
    if (m_selectedIndex != o && o < m_drives.count() && o >= 0) {
        m_selectedIndex = o;
        emit selectedChanged();
    }
}

int DriveManager::length() const {
    return m_drives.count();
}

Drive *DriveManager::lastRestoreable() {
    return m_lastRestoreable;
}

bool DriveManager::isBackendBroken() {
    return !m_errorString.isEmpty();
}

QString DriveManager::errorString() {
    return m_errorString;
}

void DriveManager::setLastRestoreable(Drive *d) {
    if (m_lastRestoreable != d) {
        m_lastRestoreable = d;
        emit restoreableDriveChanged();
    }
}

void DriveManager::onDriveConnected(Drive *d) {
    beginInsertRows(QModelIndex(), m_drives.count(), m_drives.count());
    m_drives.append(d);
    endInsertRows();
    emit drivesChanged();

    if (d->restoreStatus() == Drive::CONTAINS_LIVE) {
        setLastRestoreable(d);
    }
}

void DriveManager::onDriveRemoved(Drive *d) {
    int i = m_drives.indexOf(d);
    if (i >= 0) {
        beginRemoveRows(QModelIndex(), i, i);
        m_drives.removeAt(i);
        endRemoveRows();
        emit drivesChanged();
        if (i == m_selectedIndex) {
            m_selectedIndex = 0;
        }
        emit selectedChanged();

        if (d == m_lastRestoreable) {
            setLastRestoreable(nullptr);
        }
    }
}

void DriveManager::onBackendBroken(const QString &message) {
    m_errorString = message;
    emit isBackendBrokenChanged();
}

DriveProvider *DriveProvider::create(DriveManager *parent)  {
    if (options.testing)
        return new FakeDriveProvider(parent);

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


DriveProvider::DriveProvider(DriveManager *parent)
    : QObject(parent) {

}

Drive::Drive(DriveProvider *parent, const QString &device, const QString &name, uint64_t size, bool containsLive)
    : QObject(parent),
      m_progress(new Progress(this)),
      m_device(device),
      m_name(name),
      m_size(size),
      m_restoreStatus(containsLive ? CONTAINS_LIVE : CLEAN)
{

}

Progress *Drive::progress() const {
    return m_progress;
}

QString Drive::name() const {
    QString sizeStr;
    if (m_size < (1000UL)) {
        sizeStr = QString(" (%1 B)").arg(m_size);
    }
    else if (m_size < (1000000UL)) {
        sizeStr = QString(" (%1 KB)").arg(m_size / 1000.0, 0, 'f', 1);
    }
    else if (m_size < (1000000000UL)) {
        sizeStr = QString(" (%1 MB)").arg(m_size / 1000000.0, 0, 'f', 1);
    }
    else if (m_size < (1000000000000UL)) {
        sizeStr = QString(" (%1 GB)").arg(m_size / 1000000000.0, 0, 'f', 1);
    }
    else if (m_size < (1000000000000000UL)) {
        sizeStr = QString(" (%1 TB)").arg(m_size / 1000000000000.0, 0, 'f', 1);
    }
    else { // better be ready for exabyte images and drives
        sizeStr = QString(" (%1 EB)").arg(m_size / 1000000000000000.0, 0, 'f', 1);
    }
    return m_name + sizeStr;
}

uint64_t Drive::size() const {
    return m_size;
}

Drive::RestoreStatus Drive::restoreStatus() {
    return m_restoreStatus;
}

QString Drive::helperBinary() {
    if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        return qApp->applicationDirPath() + "/helper";
    }
    return "";
}

void Drive::prepareProcess(const QString &binary, const QStringList &arguments) {
    m_process->setProgram(binary);
    m_process->setArguments(arguments);
}

void Drive::prepareHelper(const QString &binary, const QStringList &arguments) {
    if (m_process) {
        // TODO some handling of an already present process
        m_process->deleteLater();
    }
    m_process = new QProcess(this);

    prepareProcess(binary, arguments);

    qDebug() << metaObject()->className() << "Helper command will be" << m_process->program() << m_process->arguments();

    connect(m_process, &QProcess::readyRead, this, &Drive::onReadyRead);
#if QT_VERSION >= 0x050600
    // TODO check if this is actually necessary - it should work just fine even without it
    connect(m_process, &QProcess::errorOccurred, this, &Drive::onErrorOccurred);
#endif
}

bool Drive::write(ReleaseVariant *data) {
    m_image = data;
    m_image->setErrorString(QString());

    if (data && data->size() > 0 && size() > 0 && data->realSize() > size()) {
        m_image->setErrorString(tr("This drive is not large enough."));
        return false;
    }

    QString binary = helperBinary();
    if (binary == "") {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        data->setStatus(ReleaseVariant::FAILED);
        return false;
    }
    prepareHelper(binary, writeArgs(*data));
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    m_process->start(QIODevice::ReadOnly);
    return true;
}

void Drive::restore() {
    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    QString binary = helperBinary();
    if (binary == "") {
        qWarning() << "Couldn't find the helper binary.";
        setRestoreStatus(RESTORE_ERROR);
        return;
    }
    prepareHelper(binary, restoreArgs());
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int,QProcess::ExitStatus)));
    m_process->start(QIODevice::ReadOnly);
}

void Drive::cancel() {
    if (m_process) {
        if (!m_persistentStorage && m_image->status() == ReleaseVariant::WRITE_VERIFYING) {
            m_image->setStatus(ReleaseVariant::FINISHED);
        }
        else {
            m_image->setErrorString(tr("Stopped before writing has finished."));
            m_image->setStatus(ReleaseVariant::FAILED);
        }
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
    }
}

void Drive::onFinished(int exitCode, QProcess::ExitStatus status) {
    qDebug() << metaObject()->className() << "Helper process finished with status" << status;

    if (!m_process)
        return;

    if (exitCode != 0) {
        QString errorMessage = m_process->readAllStandardError();
        /* QRegExp re("^.+:.+: "); */
        /* QStringList lines = errorMessage.split('\n'); */
        /* if (lines.length() > 0) { */
        /*     QString line = lines.first().replace(re, ""); */
        /*     m_image->setErrorString(line); */
        /* } */
        qWarning() << "Writing failed:" << errorMessage;
        Notifications::notify(tr("Error"), tr("Writing %1 failed").arg(m_image->fullName()));
        if (m_image->status() == ReleaseVariant::WRITING) {
            m_image->setErrorString(errorMessage);
            m_image->setStatus(ReleaseVariant::FAILED);
        }
    }
    else {
        Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
        m_image->setStatus(ReleaseVariant::FINISHED);
    }
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
        m_image = nullptr;
    }
}

void Drive::onRestoreFinished(int exitCode, QProcess::ExitStatus status) {
    qDebug() << metaObject()->className() << "Helper process finished with status" << status;

    if (exitCode == 0) {
        m_restoreStatus = RESTORED;
    }
    else {
        if (m_process)
            qWarning() << "Drive restoration failed:" << m_process->readAllStandardError();
        else
            qWarning() << "Drive restoration failed";
        m_restoreStatus = RESTORE_ERROR;
    }
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    emit restoreStatusChanged();
}

void Drive::onErrorOccurred(QProcess::ProcessError e) {
    Q_UNUSED(e);
    if (!m_process)
        return;

    QString errorMessage = m_process->errorString();
    qWarning() << "Restoring failed:" << errorMessage;
    m_image->setErrorString(errorMessage);
    m_process->deleteLater();
    m_process = nullptr;
    m_image->setStatus(ReleaseVariant::FAILED);
    m_image = nullptr;
}

bool Drive::operator==(const Drive &o) const {
    return name() == o.name() && size() == o.size();
}

void Drive::setRestoreStatus(Drive::RestoreStatus o) {
    if (m_restoreStatus != o) {
        m_restoreStatus = o;
        emit restoreStatusChanged();
    }
}

QStringList Drive::writeArgs(const ReleaseVariant &releaseVariant) {
    QStringList args;
    args << "write" << releaseVariant.iso() << m_device;
    if (m_persistentStorage) {
        args << "true";
    }
    return args;
}

QStringList Drive::restoreArgs() {
    QStringList args;
    args << "restore" << m_device;
    return args;
}

void Drive::onReadyRead() {
    if (!m_process)
        return;

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING && m_image->status() != ReleaseVariant::WRITING_OVERLAY)
        m_image->setStatus(ReleaseVariant::WRITING);

    m_progress->setTo(10000);
    m_progress->setValue(0.0 / 0.0);
    while (m_process->bytesAvailable() > 0) {
        QString line = m_process->readLine().trimmed();
        if (line == "CHECK") {
            qDebug() << metaObject()->className() << "Written media check starting";
            m_progress->setValue(0.0 / 0.0);
            m_image->setStatus(ReleaseVariant::WRITE_VERIFYING);
        }
        else if (line == "OVERLAY") {
            qDebug() << metaObject()->className() << "Starting to create the overlay partition";
            m_progress->setValue(0.0 / 0.0);
            m_image->setStatus(ReleaseVariant::WRITING_OVERLAY);
        }
        else {
            bool ok;
            qreal percentage = line.toLongLong(&ok);
            if (!ok || percentage < 0)
                continue;
            if (percentage >= m_progress->to()) {
                m_progress->setValue(0.0 / 0.0);
            }
            else {
                m_progress->setValue(percentage);
            }
        }
    }
}
