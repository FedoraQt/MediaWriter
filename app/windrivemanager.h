#ifndef WINDRIVEMANAGER_H
#define WINDRIVEMANAGER_H

#include "drivemanager.h"

class WinDriveProvider : public DriveProvider
{
    Q_OBJECT
public:
    WinDriveProvider(DriveManager *parent);
};

#endif // WINDRIVEMANAGER_H
