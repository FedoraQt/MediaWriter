#ifndef DRIVEMANAGER_H
#define DRIVEMANAGER_H

#include <QDebug>
#include <QAbstractListModel>

#include "releasemanager.h"

class DriveManager;
class Drive;
class UdisksDrive;

class DriveManager : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ length NOTIFY drivesChanged)
    Q_PROPERTY(Drive* selected READ selected NOTIFY selectedChanged)
    Q_PROPERTY(int selectedIndex READ selectedIndex WRITE setSelectedIndex NOTIFY selectedChanged)

    Q_PROPERTY(Drive* lastRestoreable READ lastRestoreable NOTIFY restoreableDriveChanged)
public:
    explicit DriveManager(QObject *parent = 0);

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Drive *selected() const;
    int selectedIndex() const;
    void setSelectedIndex(int o);

    int length() const;

    Drive *lastRestoreable();

signals:
    void drivesChanged();
    void selectedChanged();
    void restoreableDriveChanged();

private:
    QList<Drive*> m_drives {};
    int m_selectedIndex {};
    Drive *m_lastRestoreable;
};

/**
 * @brief The Drive class
 *
 * Contains a fake drive implementation
 */
class Drive : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(uint64_t size READ size CONSTANT)
    Q_PROPERTY(bool containsLive READ containsLive NOTIFY containsLiveChanged)
    Q_PROPERTY(bool beingRestored READ beingRestored NOTIFY beingRestoredChanged)
public:
    virtual bool beingRestored() const {
        return false;
    }
    virtual bool containsLive() const {
        return true;
    }
    virtual QString name() const {
        return QStringLiteral("SanDisk Cruzer (8.0 GB)");
    }
    virtual uint64_t size() const {
        return 0;
    }

    Q_INVOKABLE virtual bool write(ReleaseVariant *data) {
        qDebug() << "Fake drive writing:" << data->release()->name() << data->releaseVersion()->number() << data->arch()->abbreviation().first() << "contained in" << data->iso();
        return false;
    }
signals:
    void beingRestoredChanged();
    void containsLiveChanged();
};

class UdisksDrive : public Drive {
    Q_OBJECT
public:

};

#endif // DRIVEMANAGER_H
