/*
 * AcreetionOS Media Writer
 * Copyright (C) 2026 Natalie Spiva <natalie@acreetionos.org>
 *
 * Based on Fedora Media Writer by the Fedora Project
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

#ifndef VENTOYINSTALLJOB_H
#define VENTOYINSTALLJOB_H

#include "job.h"

#include <QProcess>
#include <QTemporaryDir>

/*
 * VentoyInstallJob — installs Ventoy to a USB drive
 *
 * Ventoy is an open-source tool that creates a bootable USB drive
 * where you just drop ISO files onto a partition. This job:
 *   1. Downloads the latest Ventoy release from GitHub
 *   2. Extracts it to a temp directory
 *   3. Runs Ventoy2Disk.sh to install the bootloader
 *   4. Optionally copies the target ISO to the Ventoy data partition
 *
 * Usage: helper ventoy-install <device-identifier> [<image-path>]
 *
 * Ventoy project: https://github.com/ventoy/Ventoy
 */

class VentoyInstallJob : public Job
{
    Q_OBJECT
public:
    explicit VentoyInstallJob(const QString &where, bool useGpt = true);
    VentoyInstallJob(const QString &what, const QString &where, bool useGpt = true);

public slots:
    void work() override;

private:
    QString m_isoPath;                          // optional ISO to copy after install
    bool m_useGpt;                              // true = GPT (UEFI), false = MBR (BIOS)
    QTemporaryDir *m_tempDir{nullptr};          // holds extracted Ventoy files

    static const QString VENTOY_URL;
    static const QString VENTOY_VERSION;
};

#endif // VENTOYINSTALLJOB_H
