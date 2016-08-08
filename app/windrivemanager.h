#ifndef WINDRIVEMANAGER_H
#define WINDRIVEMANAGER_H

#include "drivemanager.h"

class WinDriveProvider;
class WinDrive;

class WinDriveProvider : public DriveProvider
{
    Q_OBJECT
public:
    WinDriveProvider(DriveManager *parent);

public slots:
    void checkDrives();
private:
    WinDrive *describeDrive(int driveNumber);
};

class WinDrive : public Drive {
    Q_OBJECT
public:
    WinDrive(WinDriveProvider *parent, const QString &name, uint64_t size, bool containsLive = false);

    Q_INVOKABLE virtual void write(ReleaseVariant *data) override;
    Q_INVOKABLE virtual void restore() override;
};

#endif // WINDRIVEMANAGER_H
