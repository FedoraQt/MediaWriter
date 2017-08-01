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

#include "genericdrive.h"

#include <string>

#include <QString>

#include <libimplantisomd5.h>

#include "blockdevice.h"
#include "write.h"

void GenericDrive::writeFile(const QString &source) {
    if (source.endsWith(".xz"))
        ::writeCompressed(source, this);
    else
        ::writePlain(source, this);
}

void GenericDrive::checkChecksum() {
    ::check(getDescriptor());
}

void GenericDrive::implantChecksum() {
    char *errstr;
    if (::implantISOFD(getDescriptor(), false, true, true, &errstr) != 0) {
        throw std::runtime_error(std::string(errstr));
    }
}

void GenericDrive::addOverlay(quint64 offset, quint64 size) {
    BlockDevice device(getDescriptor());
    device.read();
    device.addPartition(offset, size);
    device.formatOverlayPartition(offset, size);
}
