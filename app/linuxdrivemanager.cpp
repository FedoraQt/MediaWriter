#include "linuxdrivemanager.h"

#include <QtDBus/QtDBus>
#include <QDBusArgument>

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent), m_objManager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus()) {
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();

    QDBusPendingCall pcall = m_objManager.asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);

    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager" ,"InterfacesAdded", this, SLOT(onInterfacesAdded(QDBusObjectPath,InterfacesAndProperties)));
    QDBusConnection::systemBus().connect("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager" ,"InterfacesRemoved", this, SLOT(onInterfacesRemoved(QDBusObjectPath,QStringList)));
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w) {
    QDBusPendingReply<DBusIntrospection> reply = *w;

    if (reply.isError()) {
        qWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            continue;

        if (introspection[i].contains("org.freedesktop.UDisks2.Partition"))
            continue;

        QString deviceId = introspection[i]["org.freedesktop.UDisks2.Block"]["Id"].toString();
        QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(introspection[i]["org.freedesktop.UDisks2.Block"]["Drive"]);
        QString devicePath = driveId.path();

        if (!deviceId.isEmpty() && !driveId.path().isEmpty() && driveId.path() != "/") {
            bool portable = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Removable"].toBool();

            if (portable) {
                QString vendor = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Vendor"].toString();
                QString model = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Model"].toString();
                uint64_t size = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Size"].toULongLong();
                bool isoLayout = introspection[i]["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

                LinuxDrive *d = new LinuxDrive(this, i.path(), QString("%1 %2").arg(vendor).arg(model), size, isoLayout);
                if (m_drives.contains(i)) {
                    LinuxDrive *tmp = m_drives[i];
                    emit DriveProvider::driveRemoved(tmp);
                }
                m_drives[i] = d;
                emit DriveProvider::driveConnected(d);
            }
        }
    }
}

void LinuxDriveProvider::onInterfacesAdded(QDBusObjectPath object_path, InterfacesAndProperties interfaces_and_properties) {
    QRegExp r("part[0-9]+$");

    if (interfaces_and_properties.keys().contains("org.freedesktop.UDisks2.Block")) {
        if (!m_drives.contains(object_path)) {
            QString deviceId = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Id"].toString();
            QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);
            QString devicePath = driveId.path();

            QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());

            if (!deviceId.isEmpty() && r.indexIn(deviceId) < 0 && !driveId.path().isEmpty() && driveId.path() != "/") {
                bool portable = driveInterface.property("Removable").toBool();

                if (portable) {
                    QString vendor = driveInterface.property("Vendor").toString();
                    QString model = driveInterface.property("Model").toString();
                    uint64_t size = driveInterface.property("Size").toULongLong();
                    bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

                    LinuxDrive *d = new LinuxDrive(this, object_path.path(), QString("%1 %2").arg(vendor).arg(model), size, isoLayout);
                    if (m_drives.contains(object_path)) {
                        LinuxDrive *tmp = m_drives[object_path];
                        emit DriveProvider::driveRemoved(tmp);
                    }
                    m_drives[object_path] = d;
                    emit DriveProvider::driveConnected(d);
                }
            }
        }
    }
}

void LinuxDriveProvider::onInterfacesRemoved(QDBusObjectPath object_path, QStringList interfaces) {
    if (interfaces.contains("org.freedesktop.UDisks2.Block")) {
        if (m_drives.contains(object_path)) {
            emit driveRemoved(m_drives[object_path]);
            m_drives[object_path]->deleteLater();
            m_drives.remove(object_path);
        }
    }
}


LinuxDrive::LinuxDrive(LinuxDriveProvider *parent, QString device, QString name, uint64_t size, bool isoLayout)
    : Drive(parent, name, size, isoLayout), m_device(device) {
}

bool LinuxDrive::beingWrittenTo() const {
    return m_process && m_process->state() == QProcess::Running;
}

void LinuxDrive::write(ReleaseVariant *data) {
    Drive::write(data);

    if (!m_process)
        m_process = new QProcess(this);

    m_beingWrittenTo = true;
    emit beingWrittenToChanged();

    m_process->setProgram("pkexec");
    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper"))
        args << qApp->applicationDirPath() + "/../helper/linux/helper";
    else
        args << QString("%1/%2").arg(LIBEXECDIR).arg("helper");
    args << "write";
    args << data->iso();
    args << m_device;
    m_process->setArguments(args);

    connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));

    m_progress->setTo(data->size());
    m_process->start(QIODevice::ReadOnly);
}

void LinuxDrive::restore() {
    qCritical() << "starting to restore";
    if (!m_process)
        m_process = new QProcess(this);

    m_restoreStatus = RESTORING;
    emit restoreStatusChanged();

    m_process->setProgram("pkexec");
    QStringList args;
    if (QFile::exists(qApp->applicationDirPath() + "/../helper/linux/helper"))
        args << qApp->applicationDirPath() + "/../helper/linux/helper";
    else
        args << QString("%1/%2").arg(LIBEXECDIR).arg("helper");
    args << "restore";
    args << m_device;
    m_process->setArguments(args);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);

    //connect(m_process, &QProcess::readyRead, this, &LinuxDrive::onReadyRead);
    connect(m_process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onRestoreFinished(int,QProcess::ExitStatus)));

    m_process->start(QIODevice::ReadOnly);
}

void LinuxDrive::onReadyRead() {
    while (m_process->bytesAvailable() > 0) {
        QString line = m_process->readLine().trimmed();
        //qCritical() << line;
        bool ok = false;
        qreal val = line.toULongLong(&ok);
        qCritical() << val << ok;
        if (ok && val > 0.0)
            m_progress->setValue(val);
    }
}

void LinuxDrive::onFinished(int exitCode, QProcess::ExitStatus status) {
    qCritical() << "Process finished" << exitCode << status;
    qCritical() << m_process->readAllStandardError();

    m_beingWrittenTo = false;
    emit beingWrittenToChanged();
}

void LinuxDrive::onRestoreFinished(int exitCode, QProcess::ExitStatus status) {
    qCritical() << "Process finished" << exitCode << status;
    qCritical() << m_process->readAllStandardError();

    m_restoreStatus = RESTORED;
    emit restoreStatusChanged();
}
