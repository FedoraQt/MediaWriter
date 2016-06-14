#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>

#include <QtDBus>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>

#include <QDebug>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where)
{
    qDBusRegisterMetaType<Properties>();
    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();
    QTimer::singleShot(0, this, &WriteJob::work);
}

void WriteJob::work()
{
    out << -1 << '\n';
    out.flush();

    QDBusInterface device("org.freedesktop.UDisks2", where, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QString drivePath = qvariant_cast<QDBusObjectPath>(device.property("Drive")).path();
    QDBusInterface drive("org.freedesktop.UDisks2", drivePath, "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus(), this);
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");

    if (message.arguments().length() == 1) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
        DBusIntrospection objects;
        arg >> objects;
        for (auto i : objects.keys()) {
            if (objects[i].contains("org.freedesktop.UDisks2.Filesystem")) {
                QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
                if (currentDrivePath == drivePath) {
                    QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
                    message = partition.call("Unmount", Properties { {"force", true} });
                }
            }
        }
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = device.call("OpenForRestore", Properties());
    QDBusUnixFileDescriptor fd = reply.value();

    if (!fd.isValid()) {
        err << reply.error().message();
        qApp->exit(2);
    }

    QFile outFile;
    outFile.open(fd.fileDescriptor(), QIODevice::WriteOnly, QFileDevice::AutoCloseHandle);
    QFile inFile(what);
    inFile.open(QIODevice::ReadOnly);

    QByteArray buffer;
    buffer.resize(BUFFER_SIZE);
    qint64 total = 0;

    while(!inFile.atEnd()) {
        qint64 len = inFile.read(buffer.data(), BUFFER_SIZE);
        outFile.write(buffer.data(), len);
        total += len;
        out << total << '\n';
        out.flush();
    }
    err << inFile.errorString() << "\n";
    err << outFile.errorString() << "\n";
    err << "OK\n";
    err.flush();
    qApp->exit(0);
}
