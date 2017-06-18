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

Drive::Drive(DriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : QObject(parent),
      m_progress(new Progress(this)),
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

void Drive::persistentStorage(bool enabled) {
    m_persistentStorage = enabled;
}

bool Drive::write(ReleaseVariant *data) {
    m_image = data;
    m_image->setErrorString(QString());

    if (data && data->size() > 0 && size() > 0 && data->realSize() > size()) {
        m_image->setErrorString(tr("This drive is not large enough."));
        return false;
    }

    return true;
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
