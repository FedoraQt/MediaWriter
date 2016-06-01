#include "linuxdrivemanager.h"

#include <QtDBus/QtDBus>
#include <QDBusArgument>

typedef QHash<QString, QHash<QString, QHash<QString, QVariant
             >              >              > DBusIntrospection;

LinuxDriveProvider::LinuxDriveProvider(DriveManager *parent)
    : DriveProvider(parent) {
    QTimer::singleShot(0, this, &LinuxDriveProvider::init);
}

void LinuxDriveProvider::init() {
    QDBusInterface interface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage msg = interface.call("GetManagedObjects");
    QRegExp r("part[0-9]+$");

    for (auto a : msg.arguments()) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(a);
        DBusIntrospection introspection;
        arg >> introspection;
        for (auto i : introspection.keys()) {
            if (!i.startsWith("/org/freedesktop/UDisks2/block_devices"))
                continue;

            QString deviceId = introspection[i]["org.freedesktop.UDisks2.Block"]["Id"].toString();
            QString driveId = qvariant_cast<QDBusObjectPath>(introspection[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
            QString devicePath = introspection[i]["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();


            if (!deviceId.isEmpty() && r.indexIn(deviceId) < 0 && !driveId.isEmpty() && driveId != "/") {
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
