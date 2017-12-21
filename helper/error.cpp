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

#include "error.h"

#include <QString>

HelperError::HelperError(Error error, const QString& context, const QString& detail) : m_error(error), m_context(context), m_detail(detail) {
}

const char* HelperError::what() const noexcept {
    switch (m_error) {
        case Error::DECOMPRESS_INIT:
            return QT_TR_NOOP("Couldn't initalize decompressor.");
        case Error::DECOMPRESS_MEM:
            return QT_TR_NOOP("Not enough memory to decompress the file \"%1\".");
        case Error::DECOMPRESS_OPTIONS:
            return QT_TR_NOOP("Unknown options passed to decompressor.");
        case Error::DECOMPRESS_UNKNOWN:
            return QT_TR_NOOP("Unknown error occurred while decompressing.");
        case Error::FILE_CORRUPT:
            return QT_TR_NOOP("The file \"%1\" is corrupted.");
        case Error::FILE_READ:
            return QT_TR_NOOP("Can't open \"%1\" for reading.");
        case Error::DRIVE_USE:
            return QT_TR_NOOP("Failed to use the drive \"%1\". Make sure the drive is not in use by another program.");
        case Error::DRIVE_WRITE:
            return QT_TR_NOOP("Failed to write to drive \"%1\". Make sure the drive is not in use by another program.");
        case Error::DRIVE_DAMAGED:
            return QT_TR_NOOP("The drive \"%1\" is probably damaged.");
        case Error::UNEXPECTED:
        default:
            return QT_TR_NOOP("Unknown error.");
    }
}

QString HelperError::context() const noexcept {
  return m_context;
}

QString HelperError::detail() const noexcept {
  return m_detail;
}
