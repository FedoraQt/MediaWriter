#ifndef WINDRIVEMANAGER_H
#define WINDRIVEMANAGER_H
#ifdef __MINGW32__

#include "drivemanager.h"

class WinDriveProvider : public DriveProvider
{
public:
    WinDriveProvider(DriveManager *parent);
};

#endif // __MINGW32__
#endif // WINDRIVEMANAGER_H
