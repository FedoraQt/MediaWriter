#ifndef LINUXDRIVEMANAGER_H
#define LINUXDRIVEMANAGER_H
#ifdef __linux__

#include "drivemanager.h"

#include <QDBusInterface>
#include <QDBusObjectPath>

class LinuxDriveProvider : public DriveProvider {
    Q_OBJECT
public:
    LinuxDriveProvider(DriveManager *parent);

private slots:
    void init();
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
