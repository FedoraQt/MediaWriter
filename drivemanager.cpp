#include "drivemanager.h"
#include "linuxdrivemanager.h"

#include <QtQml>

DriveManager::DriveManager(QObject *parent)
    : QAbstractListModel(parent), m_provider(DriveProvider::create(this))
{
    qmlRegisterUncreatableType<Drive>("MediaWriter", 1, 0, "Drive", "");

    connect(m_provider, &DriveProvider::driveConnected, this, &DriveManager::onDriveConnected);
    connect(m_provider, &DriveProvider::driveRemoved, this, &DriveManager::onDriveRemoved);
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

void DriveManager::onDriveConnected(Drive *d) {
    beginInsertRows(QModelIndex(), m_drives.count(), m_drives.count());
    m_drives.append(d);
    endInsertRows();
    emit drivesChanged();
}

void DriveManager::onDriveRemoved(Drive *d) {
    int i = m_drives.indexOf(d);
    if (i >= 0) {
        beginRemoveRows(QModelIndex(), i, i);
        m_drives.removeAt(i);
        endRemoveRows();
        emit drivesChanged();
    }
}

DriveProvider *DriveProvider::create(DriveManager *parent)  {
#ifdef __APPLE__
# warning Mac OS X support is not implemented yet
#endif // APPLE

#ifdef __MINGW32__
# warning Windows support is not implemented yet
#endif // MINGW32

#ifdef __linux__
    return new LinuxDriveProvider(parent);
#endif // linux
}

DriveProvider::DriveProvider(DriveManager *parent)
    : QObject(parent) {

}

