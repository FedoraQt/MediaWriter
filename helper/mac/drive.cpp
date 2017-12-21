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

#include "drive.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QtGlobal>

Drive::Drive(const QString &identifier)
    : m_identifier(identifier), m_device(nullptr) {
}

Drive::~Drive() {
}

void Drive::init() {
    if (m_device != nullptr)
        return;
    m_device = new QFile("/dev/r" + m_identifier, this);
    m_device->open(QIODevice::ReadWrite);
}

bool Drive::diskutil(const QStringList &arguments) {
    QProcess process;
    process.setProgram("diskutil");
    process.setArguments(arguments);
    process.start();
    process.waitForFinished();
    return process.exitCode() == 0;
}

/**
 * Write buffer directly to drive.
 */
void Drive::write(const void *buffer, std::size_t size) {
    if (m_device->write(static_cast<const char *>(buffer), size) != static_cast<qint64>(size)) {
        throw HelperError("Destination drive is not writable.");
    }
}

/**
 * Grab file descriptor.
 */
int Drive::getDescriptor() const {
    return m_device->handle();
}

QString Drive::drive() const {
    return m_identifier;
}

void Drive::wipe() {
    if (!diskutil({"eraseDisk", m_identifier}) ||
            !diskutil({"partitionDisk", m_identifier, "1", "MBR", "fat32", "FLASHDISK" , "R"})) {
        throw HelperError(Error::DRIVE_USE);
    }
}

/**
 * Unmount all partitions of drive.
 */
void Drive::umount() {
    if (!diskutil({QStringList(), "unmountDisk", "force", m_identifier})) {
        throw HelperError(Error::DRIVE_USE);
    }
}
