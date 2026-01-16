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
#include "linuxdrivemanager.h"
#endif // __linux__

#ifdef __APPLE__
#include "macdrivemanager.h"
#endif // __APPLE__

#ifdef _WIN32
#include "windrivemanager.h"
#endif // _WIN32

#include "fakedrivemanager.h"

#include <QtQml>

DriveManager *DriveManager::_self = nullptr;

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
    if (!_self)
        _self = new DriveManager();
    return _self;
}

QVariant DriveManager::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);

    if (role == Qt::UserRole + 1)
        return "drive";
    if (role == Qt::UserRole + 2)
        return "display";

    return QVariant();
}

QHash<int, QByteArray> DriveManager::roleNames() const
{
    QHash<int, QByteArray> ret;
    ret.insert(Qt::UserRole + 1, "drive");
    ret.insert(Qt::UserRole + 2, "display");
    return ret;
}

int DriveManager::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_drives.count();
}

QVariant DriveManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::UserRole + 1)
        return QVariant::fromValue(m_drives[index.row()]);

    if (role == Qt::UserRole + 2)
        return QVariant::fromValue(m_drives[index.row()]->name());

    return QVariant();
}

Drive *DriveManager::selected() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < m_drives.count())
        return m_drives[m_selectedIndex];
    return nullptr;
}

int DriveManager::selectedIndex() const
{
    return m_selectedIndex;
}

void DriveManager::setSelectedIndex(int o)
{
    if (m_selectedIndex != o && o < m_drives.count() && o >= 0) {
        m_selectedIndex = o;
        emit selectedChanged();
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

void DriveManager::setLastRestoreable(Drive *d)
{
    if (m_lastRestoreable != d) {
        m_lastRestoreable = d;
        emit restoreableDriveChanged();
    }
}

void DriveManager::onDriveConnected(Drive *d)
{
    int position = 0;
    for (auto i : m_drives) {
        if (d->size() < i->size())
            break;
        position++;
    }
    beginInsertRows(QModelIndex(), position, position);
    m_drives.insert(position, d);
    endInsertRows();
    emit drivesChanged();

    if (m_provider->initialized()) {
        m_selectedIndex = position;
        emit selectedChanged();
    } else {
        m_selectedIndex = 0;
        emit selectedChanged();
    }

    if (d->restoreStatus() == Drive::CONTAINS_LIVE) {
        setLastRestoreable(d);
        connect(d, &Drive::restoreStatusChanged, [=]() {
            if (d && d == m_lastRestoreable && d->restoreStatus() != Drive::CONTAINS_LIVE)
                setLastRestoreable(nullptr);
        });
    }
}

void DriveManager::onDriveRemoved(Drive *d)
{
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

void DriveManager::onBackendBroken(const QString &message)
{
    m_errorString = message;
    emit isBackendBrokenChanged();
}

DriveProvider *DriveProvider::create(DriveManager *parent)
{
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

bool DriveProvider::initialized() const
{
    return m_initialized;
}

DriveProvider::DriveProvider(DriveManager *parent)
    : QObject(parent)
{
}

Drive::Drive(DriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : QObject(parent)
    , m_progress(new Progress(this))
    , m_name(name)
    , m_size(size)
    , m_restoreStatus(containsLive ? CONTAINS_LIVE : CLEAN)
{
}

Progress *Drive::progress() const
{
    return m_progress;
}

void Drive::updateDrive(const QString &name, uint64_t size, bool containsLive)
{
    m_name = name;
    m_size = size;
    // Only update status when we are not in the "restoration" process
    if (m_restoreStatus == CONTAINS_LIVE || m_restoreStatus == CLEAN)
        setRestoreStatus(containsLive ? CONTAINS_LIVE : CLEAN);
}

QString Drive::name() const
{
    return QString("%1 (%2)").arg(m_name).arg(readableSize());
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

bool Drive::delayedWrite() const
{
    return m_delayedWrite;
}

void Drive::setDelayedWrite(const bool &o)
{
    if (m_delayedWrite != o) {
        m_delayedWrite = o;
        emit delayedWriteChanged();
        if (m_delayedWrite) {
            write(m_image);
        } else {
            cancel();
        }
    }
}

void Drive::setImage(ReleaseVariant *data)
{
    m_image = data;
    if (m_image)
        m_image->setErrorString(QString());
}

bool Drive::write(ReleaseVariant *data)
{
    m_image = data;
    m_image->setErrorString(QString());
    if (data && data->size() > 0 && size() > 0 && data->realSize() > size()) {
        m_image->setErrorString(tr("This drive is not large enough."));
        setDelayedWrite(false);
        return false;
    }

    return true;
}

void Drive::cancel()
{
    m_delayedWrite = false;
    emit delayedWriteChanged();
    m_error = QString();
    m_restoreStatus = CLEAN;
    emit restoreStatusChanged();
}

bool Drive::operator==(const Drive &o) const
{
    return name() == o.name() && size() == o.size();
}

void Drive::setRestoreStatus(Drive::RestoreStatus o)
{
    if (m_restoreStatus != o) {
        m_restoreStatus = o;
        emit restoreStatusChanged();
    }
}

RestoreableDriveManager::RestoreableDriveManager(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(DriveManager::instance());

    connect(DriveManager::instance(), &DriveManager::drivesChanged, this, &RestoreableDriveManager::onSourceModelChanged);

    // Connect to existing drives
    connectToDrives();

    if (rowCount() > 0) {
        m_selectedIndex = 0;
    }
}

void RestoreableDriveManager::connectToDrives()
{
    DriveManager *dm = DriveManager::instance();
    for (int i = 0; i < dm->rowCount(); i++) {
        QModelIndex idx = dm->index(i, 0);
        Drive *drive = qvariant_cast<Drive *>(dm->data(idx, Qt::UserRole + 1));
        if (drive) {
            // Use unique connection to avoid duplicates
            connect(drive, &Drive::restoreStatusChanged, this, &RestoreableDriveManager::onDriveRestoreStatusChanged, Qt::UniqueConnection);
        }
    }
}

void RestoreableDriveManager::onDriveRestoreStatusChanged()
{
    // Store the previously selected drive
    Drive *previouslySelected = selected();

    // Invalidate filter to update which drives are shown
    invalidateFilter();
    emit lengthChanged();

    // Check if the previously selected drive is still in the filtered list
    if (previouslySelected) {
        bool stillInList = false;
        for (int i = 0; i < rowCount(); i++) {
            QModelIndex idx = index(i, 0);
            Drive *drive = qvariant_cast<Drive *>(data(idx, Qt::UserRole + 1));
            if (drive == previouslySelected) {
                stillInList = true;
                if (m_selectedIndex != i) {
                    m_selectedIndex = i;
                    emit selectedChanged();
                }
                break;
            }
        }

        // If the previously selected drive is no longer in the list, select a new one
        if (!stillInList) {
            if (rowCount() > 0) {
                m_selectedIndex = 0;
            } else {
                m_selectedIndex = -1;
            }
            emit selectedChanged();
        }
    } else {
        // No previous selection - select first drive if available
        if (rowCount() > 0) {
            m_selectedIndex = 0;
            emit selectedChanged();
        }
    }
}

bool RestoreableDriveManager::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent)

    DriveManager *dm = DriveManager::instance();
    if (source_row < 0 || source_row >= dm->rowCount())
        return false;

    QModelIndex idx = dm->index(source_row, 0);
    Drive *drive = qvariant_cast<Drive *>(dm->data(idx, Qt::UserRole + 1));

    if (drive && drive->restoreStatus() == Drive::CONTAINS_LIVE)
        return true;

    return false;
}

QHash<int, QByteArray> RestoreableDriveManager::roleNames() const
{
    QHash<int, QByteArray> ret;
    ret.insert(Qt::UserRole + 1, "drive");
    ret.insert(Qt::UserRole + 2, "display");
    ret.insert(Qt::DisplayRole, "name");
    return ret;
}

QVariant RestoreableDriveManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QModelIndex sourceIndex = mapToSource(index);
    DriveManager *dm = DriveManager::instance();

    if (role == Qt::UserRole + 1)
        return dm->data(sourceIndex, Qt::UserRole + 1);
    else if (role == Qt::UserRole + 2 || role == Qt::DisplayRole) {
        Drive *drive = qvariant_cast<Drive *>(dm->data(sourceIndex, Qt::UserRole + 1));
        if (drive)
            return drive->name();
    }

    return QVariant();
}

Drive *RestoreableDriveManager::selected() const
{
    if (m_selectedIndex >= 0 && m_selectedIndex < rowCount()) {
        QModelIndex idx = index(m_selectedIndex, 0);
        return qvariant_cast<Drive *>(data(idx, Qt::UserRole + 1));
    }
    return nullptr;
}

int RestoreableDriveManager::selectedIndex() const
{
    return m_selectedIndex;
}

void RestoreableDriveManager::setSelectedIndex(int index)
{
    if (m_selectedIndex != index && index >= 0 && index < rowCount()) {
        m_selectedIndex = index;
        emit selectedChanged();
    }
}

int RestoreableDriveManager::length() const
{
    return rowCount();
}

void RestoreableDriveManager::onSourceModelChanged()
{
    // Connect to any new drives
    connectToDrives();

    // Remember previous state
    int previousCount = rowCount();
    Drive *previousSelected = selected();

    invalidateFilter();

    int newCount = rowCount();

    // Always emit length changed
    emit lengthChanged();

    // Reset selection if out of bounds
    if (m_selectedIndex >= newCount) {
        m_selectedIndex = newCount > 0 ? 0 : -1;
    }

    // If there are restoreable drives and nothing is selected, select the first one
    if (m_selectedIndex < 0 && newCount > 0) {
        m_selectedIndex = 0;
    }

    // Emit selectedChanged if the selected drive changed
    if (selected() != previousSelected) {
        emit selectedChanged();
    }
}
