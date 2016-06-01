#include "linuxdrivemanager.h"

#include <QtDBus/QtDBus>
#include <QDBusArgument>

typedef QHash<QDBusObjectPath, QHash<QString, QHash<QString, QVariant
             >                      >              > DBusIntrospection;

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent), m_objManager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus()) {
    //QTimer::singleShot(0, this, &LinuxDriveProvider::init);

    qDBusRegisterMetaType<QHash<QString, QVariant>>();
    qDBusRegisterMetaType<QHash<QString, QHash<QString, QVariant>>>();
    qDBusRegisterMetaType<DBusIntrospection>();

    QDBusPendingCall pcall = m_objManager.asyncCall("GetManagedObjects");
    QDBusPendingCallWatcher *w = new QDBusPendingCallWatcher(pcall, this);
    connect(w, &QDBusPendingCallWatcher::finished, this, &LinuxDriveProvider::init);
}

void LinuxDriveProvider::init(QDBusPendingCallWatcher *w) {
    QRegExp r("part[0-9]+$");
    QDBusPendingReply<DBusIntrospection> reply = *w;

    if (reply.isError()) {
        qWarning() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        return;
    }

    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices"))
            continue;

        QString deviceId = introspection[i]["org.freedesktop.UDisks2.Block"]["Id"].toString();
        QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(introspection[i]["org.freedesktop.UDisks2.Block"]["Drive"]);
        QString devicePath = introspection[i]["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();


        if (!deviceId.isEmpty() && r.indexIn(deviceId) < 0 && !driveId.path().isEmpty() && driveId.path() != "/") {
            bool portable = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Removable"].toBool();

            if (portable) {
                QString vendor = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Vendor"].toString();
                QString model = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Model"].toString();
                uint64_t size = introspection[driveId]["org.freedesktop.UDisks2.Drive"]["Size"].toULongLong();
                emit DriveProvider::driveConnected(new LinuxDrive(this, devicePath, QString("%1 %2").arg(vendor).arg(model), size));
            }
        }
    }
}

void LinuxDriveProvider::interfacesAdded() {

}

void LinuxDriveProvider::interfacesRemoved() {

}

LinuxDrive::LinuxDrive(LinuxDriveProvider *parent, QString device, QString name, uint64_t size)
    : Drive(parent), m_device(device), m_name(name), m_size(size) {
}

bool LinuxDrive::beingRestored() const {
    return false;
}

bool LinuxDrive::containsLive() const {
    return false;
}

QString LinuxDrive::name() const {
    QString sizeStr;
    if (m_size < (1000)) {
        sizeStr = QString(" (%1 B)").arg(m_size);
    }
    else if (m_size < (1000L*1000)) {
        sizeStr = QString(" (%1 KB)").arg(m_size / 1000.0, 0, 'f', 1);
    }
    else if (m_size < (1000L*1000*1000)) {
        sizeStr = QString(" (%1 MB)").arg(m_size / 1000000.0, 0, 'f', 1);
    }
    else if (m_size < (1000L*1000*1000*1000)) {
        sizeStr = QString(" (%1 GB)").arg(m_size / 1000000000.0, 0, 'f', 1);
    }
    else if (m_size < (1000L*1000*1000*1000*1000)) {
        sizeStr = QString(" (%1 TB)").arg(m_size / 1000000000000.0, 0, 'f', 1);
    }
    else { // better be ready for exabyte images and drives
        sizeStr = QString(" (%1 EB)").arg(m_size / 1000000000000000.0, 0, 'f', 1);
    }
    return m_name + sizeStr;
}

uint64_t LinuxDrive::size() const {
    return m_size;
}
