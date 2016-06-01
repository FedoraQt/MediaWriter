#ifndef LINUXDRIVEMANAGER_H
#define LINUXDRIVEMANAGER_H
#ifdef __linux__

#include "drivemanager.h"

#include <QDBusInterface>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusArgument>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

class LinuxDriveProvider;
class LinuxDrive;

class LinuxDriveProvider : public DriveProvider {
    Q_OBJECT
public:
    LinuxDriveProvider(DriveManager *parent);

private slots:
    void init(QDBusPendingCallWatcher *w);
    void onInterfacesAdded(QDBusObjectPath object_path, InterfacesAndProperties interfaces_and_properties);
    void onInterfacesRemoved(QDBusObjectPath object_path, QStringList interfaces);

private:
    QDBusInterface m_objManager;
    QHash<QDBusObjectPath, LinuxDrive*> m_drives;
};

class LinuxDrive : public Drive {
    Q_OBJECT
public:
    LinuxDrive(LinuxDriveProvider *parent, QString device, QString name, uint64_t size);

    virtual bool beingRestored() const override;
    virtual bool containsLive() const override;
    virtual QString name() const override;
    virtual uint64_t size() const override;

private:
    QString m_device;
    QString m_name;
    uint64_t m_size;
};

#endif // linux
#endif // LINUXDRIVEMANAGER_H
