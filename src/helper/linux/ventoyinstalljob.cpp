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

#include "ventoyinstalljob.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

const QString VentoyInstallJob::VENTOY_VERSION = "1.0.99";
const QString VentoyInstallJob::VENTOY_URL =
    QStringLiteral("https://github.com/ventoy/Ventoy/releases/download/v%1/ventoy-%1-linux.tar.gz")
        .arg(VENTOY_VERSION);

VentoyInstallJob::VentoyInstallJob(const QString &where, bool useGpt)
    : Job(where)
    , m_useGpt(useGpt)
{
}

VentoyInstallJob::VentoyInstallJob(const QString &what, const QString &where, bool useGpt)
    : Job(what, where)
    , m_isoPath(what)
    , m_useGpt(useGpt)
{
}

void VentoyInstallJob::work()
{
    out << "VENTOY_START" << Qt::endl;

    // Get the raw block device descriptor via UDisks2 (same as WriteJob)
    fd = getDescriptor();
    if (!fd.isValid()) {
        err << "Could not open device" << Qt::endl;
        QCoreApplication::exit(2);
        return;
    }

    // Resolve the block device path from the D-Bus identifier
    QString blockDevice = where;
    // The UDisks2 path looks like /org/freedesktop/UDisks2/block_devices/sdb
    // We want /dev/sdb
    QString devName = blockDevice.section('/', -1);
    QString devicePath = QStringLiteral("/dev/%1").arg(devName);

    out << "VENTOY_DOWNLOAD" << Qt::endl;

    // Download Ventoy tarball from GitHub
    m_tempDir = new QTemporaryDir();
    if (!m_tempDir->isValid()) {
        err << "Could not create temp directory" << Qt::endl;
        QCoreApplication::exit(3);
        return;
    }

    // Use QProcess to download with curl since it's simpler than QNAM in the helper
    QString tarballPath = m_tempDir->path() + "/ventoy.tar.gz";
    QProcess dlProc;
    dlProc.setProgram("curl");
    dlProc.setArguments({"-sL", "-o", tarballPath, VENTOY_URL});
    dlProc.start();
    dlProc.waitForFinished(-1);

    if (dlProc.exitCode() != 0) {
        err << "Failed to download Ventoy" << Qt::endl;
        QCoreApplication::exit(4);
        return;
    }

    // Extract the tarball using tar (simpler than linking libarchive)
    QProcess extractProc;
    extractProc.setProgram("tar");
    extractProc.setArguments({"-xzf", tarballPath, "-C", m_tempDir->path()});
    extractProc.start();
    extractProc.waitForFinished(-1);

    if (extractProc.exitCode() != 0) {
        err << "Failed to extract Ventoy" << Qt::endl;
        QCoreApplication::exit(5);
        return;
    }

    // Find the Ventoy2Disk.sh script
    QString ventoyDir = m_tempDir->path() + "/ventoy-" + VENTOY_VERSION;
    QString ventoyScript = ventoyDir + "/Ventoy2Disk.sh";
    if (!QFile::exists(ventoyScript)) {
        // Try alternate directory structure
        QDir dir(m_tempDir->path());
        QStringList candidates = dir.entryList(QStringList{"*ventoy*"}, QDir::Dirs);
        if (!candidates.isEmpty()) {
            ventoyScript = dir.absoluteFilePath(candidates.first()) + "/Ventoy2Disk.sh";
        }
    }

    if (!QFile::exists(ventoyScript)) {
        err << "Could not find Ventoy2Disk.sh" << Qt::endl;
        QCoreApplication::exit(6);
        return;
    }

    // Make it executable
    QFile::setPermissions(ventoyScript, QFile::permissions(ventoyScript) | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);

    // Unmount any mounted partitions on the device first
    QProcess umountProc;
    umountProc.setProgram("umount");
    umountProc.setArguments({devicePath + "*"});
    umountProc.start();
    umountProc.waitForFinished(10000);

    // Run Ventoy2Disk.sh to install Ventoy (non-interactive)
    // -g flag = GPT partition table (UEFI), omitted = MBR (BIOS/legacy)
    out << "VENTOY_INSTALLING" << Qt::endl;
    out << "VENTOY_MODE_" << (m_useGpt ? "GPT" : "MBR") << Qt::endl;

    QStringList ventoyArgs = {ventoyScript, "-i"};
    if (m_useGpt) {
        ventoyArgs << "-g"; // GPT for UEFI
    }
    ventoyArgs << devicePath;

    QProcess ventoyProc;
    ventoyProc.setProgram("bash");
    ventoyProc.setArguments(ventoyArgs);
    ventoyProc.setProcessChannelMode(QProcess::ForwardedChannels);
    ventoyProc.start();
    ventoyProc.waitForFinished(-1);

    if (ventoyProc.exitCode() != 0) {
        err << "Ventoy installation failed" << Qt::endl;
        QCoreApplication::exit(7);
        return;
    }

    // If an ISO path was provided, copy it to the Ventoy data partition
    if (!m_isoPath.isEmpty()) {
        out << "VENTOY_COPYING_ISO" << Qt::endl;

        // Ventoy creates a data partition (usually the last partition, ExFAT or NTFS)
        // Find it by looking for the Ventoy partition label
        QProcess findPartProc;
        findPartProc.setProgram("lsblk");
        findPartProc.setArguments({"-o", "NAME,LABEL,FSTYPE", "-l", "-n", devicePath});
        findPartProc.start();
        findPartProc.waitForFinished(10000);

        QString partOutput = QString::fromUtf8(findPartProc.readAllStandardOutput());
        QStringList partLines = partOutput.split('\n', Qt::SkipEmptyParts);

        QString ventoyPartition;
        for (const QString &line : partLines) {
            // Look for the partition that doesn't have "VTOYEFI" label
            if (!line.contains("VTOYEFI") && !line.contains(devName + "1")) {
                QStringList cols = line.split(' ', Qt::SkipEmptyParts);
                if (cols.size() >= 1) {
                    QString partName = cols[0].trimmed();
                    if (partName.startsWith("├─") || partName.startsWith("└─"))
                        partName = partName.mid(1).trimmed();
                    if (!partName.isEmpty() && partName != devName) {
                        ventoyPartition = "/dev/" + partName;
                    }
                }
            }
        }

        // Fallback: just try the last partition
        if (ventoyPartition.isEmpty()) {
            ventoyPartition = devicePath + "2"; // Ventoy typically uses partition 2 for data
        }

        // Mount the Ventoy data partition
        QString mountPoint = m_tempDir->path() + "/mnt";
        QDir().mkpath(mountPoint);

        QProcess mountProc;
        mountProc.setProgram("mount");
        mountProc.setArguments({ventoyPartition, mountPoint});
        mountProc.start();
        mountProc.waitForFinished(10000);

        if (mountProc.exitCode() == 0) {
            // Copy the ISO to the Ventoy data partition
            QString isoFileName = QFileInfo(m_isoPath).fileName();
            QString destPath = mountPoint + "/" + isoFileName;

            QFile::remove(destPath); // Remove any existing file with same name
            if (QFile::copy(m_isoPath, destPath)) {
                out << "VENTOY_ISO_COPIED" << Qt::endl;
            } else {
                // Fallback: try with dd
                QProcess ddProc;
                ddProc.setProgram("dd");
                ddProc.setArguments({"if=" + m_isoPath, "of=" + destPath, "bs=1M", "status=progress"});
                ddProc.start();
                ddProc.waitForFinished(-1);
            }

            // Unmount
            QProcess umountDataProc;
            umountDataProc.setProgram("umount");
            umountDataProc.setArguments({mountPoint});
            umountDataProc.start();
            umountDataProc.waitForFinished(5000);
        }
    }

    // Cleanup
    delete m_tempDir;
    m_tempDir = nullptr;

    out << "VENTOY_DONE" << Qt::endl;
    QCoreApplication::exit(0);
}


