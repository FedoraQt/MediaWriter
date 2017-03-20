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

#import <Foundation/Foundation.h>
#import <DiskArbitration/DiskArbitration.h>

DASessionRef session = nil;

static void OnDiskAppeared(DADiskRef disk, void *__attribute__((__unused__)));
static void OnDiskDisappeared(DADiskRef disk, void *__attribute__((__unused__)));
static DADissenterRef OnMountApproval(DADiskRef disk, void *__attribute__((__unused__)));

void (*onAdded)(const char *bsdName, const char *vendor, const char *model, unsigned long long size, bool restoreable);
void (*onRemoved)(const char *bsdName);

void startArbiter(void (*addedCallback)(const char *bsdName, const char *vendor, const char *model, unsigned long long size, bool restoreable),
                  void (*removedCallback)(const char *bsdName)) {
    onAdded = addedCallback;
    onRemoved = removedCallback;

    session = DASessionCreate(kCFAllocatorDefault);
    DARegisterDiskAppearedCallback(session, NULL, OnDiskAppeared, NULL);
    DARegisterDiskDisappearedCallback(session, NULL, OnDiskDisappeared, NULL);
    DARegisterDiskMountApprovalCallback(session, NULL, OnMountApproval, NULL);
    DASessionScheduleWithRunLoop(session, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
}

void stopArbiter() {
    CFRelease(session);
}

DADissenterRef OnMountApproval(DADiskRef disk, void *context) {
    (void) disk; (void) context; // unused
    return DADissenterCreate(kCFAllocatorDefault, kDAReturnExclusiveAccess, CFSTR("Fedora Media Writer"));
}



static void OnDiskAppeared(DADiskRef disk, void *__attribute__((__unused__)) ctx) {
    static bool lastS1 = false;
    static NSString *lastPrefix = @"";
    if (disk) {
        NSDictionary *diskDescription = (NSDictionary*) DADiskCopyDescription(disk);

        NSNumber *isRemovable = nil;
        NSNumber *isInternal = nil;
        NSNumber *isWhole = nil;
        NSString *bsdName = nil;
        NSString *diskVendor = nil;
        NSString *diskModel = nil;
        NSNumber *diskSize = nil;
        NSNumber *bsdNumber = nil;
        NSString *deviceProtocol = nil;

        diskSize = [diskDescription objectForKey:(NSString*)kDADiskDescriptionMediaSizeKey];
        diskVendor = [diskDescription objectForKey:(NSString*)kDADiskDescriptionDeviceVendorKey];
        diskModel = [diskDescription objectForKey:(NSString*)kDADiskDescriptionDeviceModelKey];
        isRemovable = [diskDescription objectForKey:(NSNumber*)kDADiskDescriptionMediaRemovableKey];
        bsdName = [diskDescription objectForKey:(NSString*)kDADiskDescriptionMediaBSDNameKey];
        isWhole = [diskDescription objectForKey:(NSNumber*)kDADiskDescriptionMediaWholeKey];
        bsdNumber = [diskDescription objectForKey:(NSNumber*)kDADiskDescriptionMediaBSDUnitKey];
        isInternal = [diskDescription objectForKey:(NSNumber*)kDADiskDescriptionDeviceInternalKey];
        deviceProtocol = [diskDescription objectForKey:(NSString*)kDADiskDescriptionDeviceProtocolKey];

        //NSLog(@"%@\n", bsdName);

        if ([bsdName hasSuffix:@"s1"]) {
            lastPrefix = [bsdName substringToIndex:[bsdName length] - 2];
            if ([[diskDescription objectForKey:(NSNumber*)kDADiskDescriptionMediaSizeKey] integerValue] == 8192)
                lastS1 = true;
        }

        if (isWhole != nil && [isWhole integerValue] != 0) {
            if ((isInternal != nil && [isInternal integerValue] == 0 && isRemovable != nil && [isRemovable integerValue] != 0)
                || (deviceProtocol != nil && [deviceProtocol isEqual:@"USB"])) {
                    bool isRestoreable = lastS1 & [bsdName isEqualToString:lastPrefix];
                    onAdded([bsdName UTF8String], [diskVendor UTF8String], [diskModel UTF8String], [diskSize integerValue], isRestoreable);
            }
        }

        [diskDescription autorelease];
    }
}

static void OnDiskDisappeared(DADiskRef disk, void *__attribute__((__unused__)) ctx) {
    if (disk) {
        NSDictionary *diskDescription = (NSDictionary*) DADiskCopyDescription(disk);

        NSNumber *isWhole = nil;
        NSString *bsdName = nil;

        bsdName = [diskDescription objectForKey:(NSString*)kDADiskDescriptionMediaBSDNameKey];
        isWhole = [diskDescription objectForKey:(NSNumber*)kDADiskDescriptionMediaWholeKey];

        // let`s warn about every disk removed regardless of it being removable or not
        if (isWhole != nil && [isWhole integerValue] != 0) {
            onRemoved([bsdName UTF8String]);
        }

        [diskDescription autorelease];
    }
}
