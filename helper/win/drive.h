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

#ifndef DRIVE_H
#define DRIVE_H

#include <type_traits>
#include <stdexcept>
#include <utility>

#include <QObject>
#include <QProcess>
#include <QString>
#include <QtGlobal>

#include <io.h>
#include <windows.h>

#include "error.h"
#include "genericdrive.h"

class Drive : public GenericDrive {
    Q_OBJECT
private:
    static void throwError(Error error, const QString &drive = "unknown");
    static HANDLE openBlockDevice(const QString &device);
    template <class OutBuffer, class InBuffer = std::nullptr_t>
    static DWORD controlCodeOf();
    template <class T>
    static T *addressOf(T &variable);
    template <class T>
    static std::size_t sizeOf();
    template <class OutBuffer = std::nullptr_t, class InBuffer = std::nullptr_t>
    static OutBuffer deviceIoControl(HANDLE handle, InBuffer *inbuffer = nullptr, Error error = Error::DRIVE_USE);

    template <class OutBuffer = std::nullptr_t, class InBuffer = std::nullptr_t>
    OutBuffer deviceIoControl(InBuffer *inbuffer = nullptr, Error error = Error::DRIVE_USE) const;
    bool deviceIoControlCode(DWORD controlCode) const;
    void lock();
    void unlock();
    bool hasDriveLetter(const char driveLetter) const;

public:
    /**
     * Shared public interface across platforms.
     */
    explicit Drive(const QString &driveIdentifier);
    ~Drive();
    void init();
    void write(const void *buffer, std::size_t size);
    int getDescriptor() const;
    QString drive() const;
    void wipe();
    void umount();

private:
    HANDLE m_handle;
    DWORD m_driveNumber;
    DISK_GEOMETRY_EX m_geometry;
};

template <class T>
std::size_t Drive::sizeOf() {
    if (std::is_same<T, std::nullptr_t>::value)
        return 0;
    return sizeof(T);
}

template <class T>
T *Drive::addressOf(T &variable) {
    if (std::is_same<T, std::nullptr_t>::value)
        return nullptr;
    return &variable;
}

/**
 * Usage:
 * ```
 * // The control code is determined automatically based on the return type and
 * // the type of the input.
 * auto out = deviceIoControl<RETURN_TYPE>();
 * auto out = deviceIoControl<RETURN_TYPE>(in);
 * deviceIoControl(in);
 * // Not part of the interface below. Use it when there's no input or output
 * // provided with the specified control cod.
 * deviceIoControlCode(CONTROL_CODE);
 * ```
 * To add a new control code see the end of win/drive.cpp.
 *
 *
 * The design of the DeviceIoControl is evil partially because it does too many
 * things.
 * This wrapper does not fix that but it makes the interface easier to use
 * since the user does not need to specify the control code or provide unused
 * arguments when calling the wrapper.
 */
template <class OutBuffer, class InBuffer>
OutBuffer Drive::deviceIoControl(HANDLE handle, InBuffer *inbuffer, Error error) {
    OutBuffer outbuffer;
    DWORD bytesReturned;
    if (!::DeviceIoControl(handle, controlCodeOf<OutBuffer, InBuffer>(), inbuffer, sizeOf<InBuffer>(), addressOf(outbuffer), sizeOf<OutBuffer>(), &bytesReturned, nullptr)) {
        // Could get drive identifier via DISK_GEOMETRY.
        throwError(error);
    }
    return outbuffer;
}

template <class OutBuffer, class InBuffer>
OutBuffer Drive::deviceIoControl(InBuffer *inbuffer, Error error) const {
    // Could rethrow to add drive identifier.
    return deviceIoControl<OutBuffer, InBuffer>(m_handle, inbuffer, error);
}

#endif // DRIVE_H
