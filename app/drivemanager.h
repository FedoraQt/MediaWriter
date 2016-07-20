#ifndef DRIVEMANAGER_H
#define DRIVEMANAGER_H

#include <QDebug>
#include <QAbstractListModel>

#include "releasemanager.h"

class DriveManager;
class DriveProvider;
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

private slots:
    void onDriveConnected(Drive *d);
    void onDriveRemoved(Drive *d);

signals:
    void drivesChanged();
    void selectedChanged();
    void restoreableDriveChanged();

private:
    QList<Drive*> m_drives {};
    int m_selectedIndex {};
    Drive *m_lastRestoreable;
    DriveProvider *m_provider;
};

class DriveProvider : public QObject {
    Q_OBJECT
public:
    static DriveProvider *create(DriveManager *parent);

signals:
    void driveConnected(Drive *d);
    void driveRemoved(Drive *d);

protected:
    DriveProvider(DriveManager *parent);
};

/**
 * @brief The Drive class
 *
 * Contains a fake drive implementation
 */
class Drive : public QObject {
    Q_OBJECT
    Q_PROPERTY(Progress* progress READ progress CONSTANT)

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(uint64_t size READ size CONSTANT)
    Q_PROPERTY(bool beingWrittenTo READ beingWrittenTo NOTIFY beingWrittenToChanged)
    Q_PROPERTY(RestoreStatus restoreStatus READ restoreStatus NOTIFY restoreStatusChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)
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

    virtual bool beingWrittenTo() const = 0;
    virtual QString name() const;
    virtual uint64_t size();
    virtual RestoreStatus restoreStatus();
    virtual QString error();

    Q_INVOKABLE virtual void write(ReleaseVariant *data);
    Q_INVOKABLE virtual void restore() = 0;

protected slots:
    void setRestoreStatusChanged(RestoreStatus o);
    void setError(const QString &o);

signals:
    void beingWrittenToChanged();
    void restoreStatusChanged();
    void errorChanged();

protected:
    ReleaseVariant *m_image { nullptr };
    Progress *m_progress { nullptr };
    QString m_name { };
    uint64_t m_size { 0 };
    RestoreStatus m_restoreStatus { CLEAN };
    QString m_error { };
};

#endif // DRIVEMANAGER_H
