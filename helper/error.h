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

#ifndef HELPER_ERROR_H
#define HELPER_ERROR_H

#include <exception>

#include <QString>

enum class Error {
  DECOMPRESS_INIT,
  DECOMPRESS_MEM,
  DECOMPRESS_OPTIONS,
  DECOMPRESS_UNKNOWN,
  FILE_CORRUPT,
  FILE_READ,
  DRIVE_USE,
  DRIVE_WRITE,
  DRIVE_DAMAGED,
  UNEXPECTED
};

class HelperError : public std::exception {
public:
  HelperError(Error error, const QString& context = "", const QString& detail = "");
  const char *what() const noexcept;
  QString context() const noexcept;
  QString detail() const noexcept;
private:
  Error m_error;
  QString m_context;
  QString m_detail;
};

#endif // HELPER_ERROR_H
