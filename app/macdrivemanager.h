#ifndef MACDRIVEMANAGER_H
#define MACDRIVEMANAGER_H
#ifdef __APPLE__

#include "drivemanager.h"

class MacDriveProvider : public DriveProvider
{
public:
    MacDriveProvider(DriveManager *parent);
};

#endif // __APPLE__
#endif // MACDRIVEMANAGER_H
