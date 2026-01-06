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

#ifndef DRIVEMANAGER_H
#define DRIVEMANAGER_H

#include <QAbstractListModel>
#include <QDebug>
#include <QSortFilterProxyModel>

#include "releasemanager.h"

class DriveManager;
class DriveProvider;
class Drive;
class UdisksDrive;
class RestoreableDriveManager;

/**
 * @brief The DriveManager class
 *
 * The class providing the list of all available portable drives to the UI
 *
 * Platform-independent
 *
 * @property length count of the drives
 * @property selected the selected drive
 * @property selectedIndex the index of the selected drive
 * @property lastRestoreable the most recently connected restoreable drive
 */
class DriveManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ length NOTIFY drivesChanged)
    Q_PROPERTY(Drive *selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedChanged)
    Q_PROPERTY(bool isBroken READ isBackendBroken NOTIFY isBackendBrokenChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY isBackendBrokenChanged)

    Q_PROPERTY(Drive *lastRestoreable READ lastRestoreable WRITE setLastRestoreable NOTIFY restoreableDriveChanged)
public:
    static DriveManager *instance();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Drive *selected() const;
    int selectedIndex() const;
    void setSelectedIndex(int o);

    int length() const;

    Drive *lastRestoreable();

    bool isBackendBroken();
    QString errorString();

    void setLastRestoreable(Drive *d);

private slots:
    void onDriveConnected(Drive *d);
    void onDriveRemoved(Drive *d);
    void onBackendBroken(const QString &message);

signals:
    void drivesChanged();
    void selectedChanged();
    void restoreableDriveChanged();
    void isBackendBrokenChanged();

private:
    explicit DriveManager(QObject *parent = 0);

    static DriveManager *_self;
    QList<Drive *> m_drives{};
    int m_selectedIndex{0};
    Drive *m_lastRestoreable{nullptr};
    DriveProvider *m_provider{nullptr};
    QString m_errorString{};
};

/**
 * @brief The DriveProvider class
 *
 * An "abstract" class providing a signal interface to the @ref DriveManager class.
 *
 * Should be reimplemented for every platform and the appropriate child class constructor should be added to the @ref create method.
 *
 * It reports coming and going drives through the @ref driveConnected and @ref driveRemoved signals.
 */
class DriveProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool initialized READ initialized NOTIFY initializedChanged)
public:
    static DriveProvider *create(DriveManager *parent);

    bool initialized() const;

signals:
    void driveConnected(Drive *d);
    void driveRemoved(Drive *d);
    void backendBroken(const QString &message);

    void initializedChanged();

protected:
    DriveProvider(DriveManager *parent);

    bool m_initialized{true};
};

/**
 * @brief The Drive class
 *
 * Contains a non-drive implementation. The methods can be reimplemented if necessary. Usually the child classes will just need to modify the values of the properties of the class.
 *
 * Should be reimplemented for every platform. The child instances should be created and handled by the @ref DriveProvider class.
 *
 * @property progress the @ref Progress object reporting the progress of writing the image
 * @property name name of the drive, should be human-readable, in ideal case the model of the drive and its size
 * @property size the size of the drive, in bytes
 * @property restoreStatus the status of restoring the drive
 */
class Drive : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Progress *progress READ progress CONSTANT)

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString readableSize READ readableSize CONSTANT)
    Q_PROPERTY(qreal size READ size CONSTANT)
    Q_PROPERTY(RestoreStatus restoreStatus READ restoreStatus NOTIFY restoreStatusChanged)
    Q_PROPERTY(bool delayedWrite READ delayedWrite WRITE setDelayedWrite NOTIFY delayedWriteChanged)
public:
    enum RestoreStatus {
        CLEAN = 0,
        CONTAINS_LIVE,
        RESTORING,
        RESTORE_ERROR,
        RESTORED,
    };
    Q_ENUMS(RestoreStatus)

    Drive(DriveProvider *parent, const QString &name, uint64_t size, bool containsLive = false);

    Progress *progress() const;

    virtual void updateDrive(const QString &name, uint64_t size, bool containsLive = false);

    virtual QString name() const;
    virtual QString readableSize() const;
    virtual qreal size() const;
    virtual RestoreStatus restoreStatus();
    virtual bool delayedWrite() const;

    virtual void setDelayedWrite(const bool &o);

    Q_INVOKABLE virtual void setImage(ReleaseVariant *data);
    Q_INVOKABLE virtual bool write(ReleaseVariant *data);
    Q_INVOKABLE virtual void cancel();
    Q_INVOKABLE virtual void restore() = 0;

    bool operator==(const Drive &o) const;

public slots:
    void setRestoreStatus(RestoreStatus o);

signals:
    void restoreStatusChanged();
    void delayedWriteChanged();

protected:
    ReleaseVariant *m_image{nullptr};
    Progress *m_progress{nullptr};
    QString m_name{};
    uint64_t m_size{0};
    RestoreStatus m_restoreStatus{CLEAN};
    QString m_error{};
    bool m_delayedWrite{false};
};

/**
 * @brief The RestoreableDriveManager class
 *
 * A proxy model that filters drives to only show those containing live USB systems.
 *
 * @property selected the currently selected drive
 * @property selectedIndex the index of the currently selected drive
 * @property length count of the filtered (restoreable) drives
 */
class RestoreableDriveManager : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(Drive *selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedChanged)
    Q_PROPERTY(int length READ length NOTIFY lengthChanged)
public:
    explicit RestoreableDriveManager(QObject *parent = nullptr);

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    QHash<int, QByteArray> roleNames() const override;
    Q_INVOKABLE QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Drive *selected() const;
    int selectedIndex() const;
    void setSelectedIndex(int index);

    int length() const;

signals:
    void selectedChanged();
    void lengthChanged();

private slots:
    void onSourceModelChanged();
    void onDriveRestoreStatusChanged();

private:
    void connectToDrives();

    int m_selectedIndex{0};
};

#endif // DRIVEMANAGER_H
