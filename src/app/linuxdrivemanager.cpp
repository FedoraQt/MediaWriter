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

#include "linuxdrivemanager.h"

#include <QtDBus/QtDBus>
#include <QDBusArgument>


#include "notifications.h"

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent)
{
    mDebug() << this->metaObject()->className() << "construction";
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();

    m_initialized = false;

    QTimer::singleShot(0, this, SLOT(delayedConstruct()));
}

void LinuxDriveProvider::delayedConstruct() {
    m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());

    mDebug() << this->metaObject()->className() << "Calling GetManagedObjects over DBus";
    QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);

    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", nullptr, "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(onPropertiesChanged(QString,QVariantMap,QStringList)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesAdded", this, SLOT(onInterfacesAdded(QDBusObjectPath,InterfacesAndProperties)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", "InterfacesRemoved", this, SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList)));
}

QDBusObjectPath LinuxDriveProvider::handleObject(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegularExpression numberRE("[0-9]$");
    QRegularExpression mmcRE("[0-9]p[0-9]$");
    QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);

    QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

    if ((numberRE.match(object_path.path()).hasMatch() && !object_path.path().startsWith("/org/freedesktop/UDisks2/block_devices/mmcblk")) ||
            mmcRE.match(object_path.path()).hasMatch())
        return QDBusObjectPath();

    if (!driveId.path().isEmpty() && driveId.path() != "/") {
        bool portable = driveInterface.property("Removable").toBool();
        bool optical = driveInterface.property("Optical").toBool();
        bool containsMedia = driveInterface.property("MediaAvailable").toBool();
        QString connectionBus = driveInterface.property("ConnectionBus").toString().toLower();
        bool isValid = containsMedia && !optical && (portable || connectionBus == "usb");

        QString vendor = driveInterface.property("Vendor").toString();
        QString model = driveInterface.property("Model").toString();
        uint64_t size = driveInterface.property("Size").toULongLong();
        bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

        QString name;
        if (vendor.isEmpty())
            if (model.isEmpty())
                name = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();
            else
                name = model;
        else
            if (model.isEmpty())
                name = vendor;
            else
                name = QString("%1 %2").arg(vendor).arg(model);

        mDebug() << this->metaObject()->className() << "New drive" << driveId.path() << "-" << name << "(" << size << "bytes;" << (isValid ? "removable;" : "nonremovable;") << connectionBus << ")";

        if (isValid) {

            // TODO find out why do I do this
            if (m_drives.contains(object_path)) {
                LinuxDrive *tmp = m_drives[object_path];
                emit DriveProvider::driveRemoved(tmp);
            }
            LinuxDrive *d = new LinuxDrive(this, object_path.path(), name, size, isoLayout);
            m_drives[object_path] = d;
            emit DriveProvider::driveConnected(d);

            return object_path;
        }
    }
    return QDBusObjectPath();
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w) {
    mDebug() << this->metaObject()->className() << "Got a reply to GetManagedObjects, parsing";

    QDBusPendingReply<DBusIntrospection> reply = *w;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    const QList<QDBusObjectPath> paths = m_drives.keys();
    QSet<QDBusObjectPath> oldPaths(paths.begin(), paths.end());
#else
    QSet<QDBusObjectPath> oldPaths = m_drives.keys().toSet();
#endif
    QSet<QDBusObjectPath> newPaths;

    if (reply.isError()) {
        mWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        emit backendBroken(tr("UDisks2 seems to be unavailable or unaccessible on your system."));
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            continue;

        QDBusObjectPath path = handleObject(i, introspection[i]);
        if (!path.path().isEmpty())
            newPaths.insert(path);
    }

    for (auto i : oldPaths - newPaths) {
        emit driveRemoved(m_drives[i]);
        m_drives[i]->deleteLater();
        m_drives.remove(i);
    }

    m_initialized = true;
    emit initializedChanged();
}

void LinuxDriveProvider::onInterfacesAdded(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegularExpression numberRE("[0-9]$");

    if (interfaces_and_properties.keys().contains("org.freedesktop.UDisks2.Block")) {
        if (!m_drives.contains(object_path)) {
            handleObject(object_path, interfaces_and_properties);
        }
    }
}

void LinuxDriveProvider::onInterfacesRemoved(const QDBusObjectPath &object_path, const QStringList &interfaces) {
    if (interfaces.contains("org.freedesktop.UDisks2.Block")) {
        if (m_drives.contains(object_path)) {
            mDebug() << this->metaObject()->className() << "Drive at" << object_path.path() << "removed";
            emit driveRemoved(m_drives[object_path]);
            m_drives[object_path]->deleteLater();
            m_drives.remove(object_path);
        }
    }
}

void LinuxDriveProvider::onPropertiesChanged(const QString &interface_name, const QVariantMap &changed_properties, const QStringList &invalidated_properties) {
    Q_UNUSED(interface_name)
    const QSet<QString> watchedProperties = { "MediaAvailable", "Size" };
    const QList<QString> changedPropertyKeys = changed_properties.keys();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QSet<QString> changedPropertiesSet(changedPropertyKeys.begin(), changedPropertyKeys.end());
    QSet<QString> invalidatedPropertiesSet(invalidated_properties.begin(), invalidated_properties.end());
    // not ideal but it works alright without a huge lot of code
    if (!changedPropertiesSet.intersect(watchedProperties).isEmpty() ||
            !invalidatedPropertiesSet.intersect(watchedProperties).isEmpty()) {
#else
    // not ideal but it works alright without a huge lot of code
    if (!changed_properties.keys().toSet().intersect(watchedProperties).isEmpty() ||
            !invalidated_properties.toSet().intersect(watchedProperties).isEmpty()) {
#endif
        QDBusPendingCall pcall = m_objManager->asyncCall("GetManagedObjects");
        QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

        connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
    }
}

LinuxDrive::LinuxDrive(LinuxDriveProvider *parent, QString device, QString name, uint64_t size, bool isoLayout)
    : Drive(parent, name, size, isoLayout), m_device(device) {
}

LinuxDrive::~LinuxDrive() {
    if (m_image && m_image->status() == ReleaseVariant::WRITING) {
        m_image->setErrorString(tr("The drive was removed while it was written to."));
        m_image->setStatus(ReleaseVariant::FAILED);
    }
}

bool LinuxDrive::write(ReleaseVariant *data) {
    mDebug() << this->metaObject()->className() << "Will now write" << data->iso() << "to" << this->m_device;

    if (!Drive::write(data))
        return false;

    if (m_image->status() == ReleaseVariant::READY || m_image->status() == ReleaseVariant::FAILED ||
            m_image->status() == ReleaseVariant::FAILED_VERIFICATION || m_image->status() == ReleaseVariant::FINISHED)
        m_image->setStatus(ReleaseVariant::WRITING);

    if (!m_process)
        m_process = new QProcess(this);

    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper/linux/helper");
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper");
    }
    else if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper"))) {
        m_process->setProgram(QString("%1/%2").arg(LIBEXECDIR).arg("helper"));
    }
    else {
        data->setErrorString(tr("Could not find the helper binary. Check your installation."));
        data->setStatus(ReleaseVariant::FAILED);
        setDelayedWrite(false);
        return false;
    }
    args << "write";
    if (data->status() == ReleaseVariant::WRITING)
        args << data->iso();
    else
        args << data->temporaryPath();
    args << m_device;
    mDebug() << this->metaObject()->className() << "Helper command will be" << args;
    m_process->setArguments(args);

    connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));
    // TODO check if this is actually necessary - it should work just fine even without it
    connect(m_process, &QProcess::errorOccurred, this, &LinuxDrive::onErrorOccurred);

    m_process->start(QIODevice::ReadOnly);

    return true;
}

void LinuxDrive::cancel() {
    Drive::cancel();
    static bool beingCancelled = false;
    if (m_process != nullptr && !beingCancelled) {
        beingCancelled = true;
        if (m_image) {
            if (m_image->status() == ReleaseVariant::WRITE_VERIFYING) {
                m_image->setStatus(ReleaseVariant::FINISHED);
            }
            else if (m_image->status() != ReleaseVariant::DOWNLOADING &&
                     m_image->status() != ReleaseVariant::DOWNLOAD_VERIFYING) {
                m_image->setErrorString(tr("Stopped before writing has finished."));
                m_image->setStatus(ReleaseVariant::FAILED);
            }
        }
        m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
        beingCancelled = false;
    }
}

void LinuxDrive::restore() {
    mDebug() << this->metaObject()->className() << "Will now restore" << this->m_device;

    if (!m_process)
        m_process = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/../helper/linux/helper");
    }
    else if (QFile::exists(qApp->applicationDirPath() + "/helper")) {
        m_process->setProgram(qApp->applicationDirPath() + "/helper");
    }
    else if (QFile::exists(QString("%1/%2").arg(LIBEXECDIR).arg("helper"))) {
        m_process->setProgram(QString("%1/%2").arg(LIBEXECDIR).arg("helper"));
    }
    else {
        mWarning() << "Couldn't find the helper binary.";
        setRestoreStatus(RESTORE_ERROR);
        return;
    }
    args << "restore";
    args << m_device;
    mDebug() << this->metaObject()->className() << "Helper command will be" << args;
    m_process->setArguments(args);

    connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int,QProcess::ExitStatus)));

    m_process->start(QIODevice::ReadOnly);
}

void LinuxDrive::onReadyRead() {
    if (!m_process)
        return;

    m_progress->setTo(m_image->size());
    m_progress->setValue(0.0/0.0);

    if (m_image->status() != ReleaseVariant::WRITE_VERIFYING && m_image->status() != ReleaseVariant::WRITING)
        m_image->setStatus(ReleaseVariant::WRITING);

    while (m_process->bytesAvailable() > 0) {
        QString line = m_process->readLine().trimmed();
        if (line == "CHECK") {
            mDebug() << this->metaObject()->className() << "Helper finished writing, now it will check the written data";
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITE_VERIFYING);
        }
        else if (line == "WRITE") {
            m_progress->setValue(0);
            m_image->setStatus(ReleaseVariant::WRITING);
        }
        else if (line == "DONE") {
            m_progress->setValue(m_image->size());
            m_image->setStatus(ReleaseVariant::FINISHED);
            Notifications::notify(tr("Finished!"), tr("Writing %1 was successful").arg(m_image->fullName()));
        }
        else {
            bool ok = false;
            qreal val = line.toULongLong(&ok);
            if (ok && val > 0.0)
                m_progress->setValue(val);
        }
    }
}

void LinuxDrive::onFinished(int exitCode, QProcess::ExitStatus status) {
    mDebug() << this->metaObject()->className() << "Helper process finished with status" << status;

    setDelayedWrite(false);

    if (!m_process)
        return;

    if (exitCode != 0) {
        QString errorMessage = m_process->readAllStandardError();
        mWarning() << "Writing failed:" << errorMessage;
        Notifications::notify(tr("Error"), tr("Writing %1 failed").arg(m_image->fullName()));
        if (m_image->status() == ReleaseVariant::WRITING) {
            m_image->setErrorString(errorMessage);
            m_image->setStatus(ReleaseVariant::FAILED);
        }
    }

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
        m_image = nullptr;
    }
}

void LinuxDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus status) {
    mDebug() << this->metaObject()->className() << "Helper process finished with status" << status;

    if (exitCode != 0) {
        if (m_process)
            mWarning() << "Drive restoration failed:" << m_process->readAllStandardError();
        else
            mWarning() << "Drive restoration failed";
        m_restoreStatus = RESTORE_ERROR;
    }
    else {
        m_restoreStatus = RESTORED;
    }
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    emit restoreStatusChanged();
}

void LinuxDrive::onErrorOccurred(QProcess::ProcessError e) {
    Q_UNUSED(e);
    if (!m_process)
        return;

    QString errorMessage = m_process->errorString();
    mWarning() << "Restoring failed:" << errorMessage;
    m_image->setErrorString(errorMessage);
    m_process->deleteLater();
    m_process = nullptr;
    m_image->setStatus(ReleaseVariant::FAILED);
    m_image = nullptr;
}
