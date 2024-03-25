/*
 * AOSC Media Writer
 * Copyright (C) 2021 Jan Grulich <jgrulich@redhat.com>
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

#include "portalfiledialog.h"
#include "utilities.h"

#include <QFileDialog>
#include <QGuiApplication>
#include <QRandomGenerator>
#include <QStandardPaths>

#ifdef __linux
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QtDBus/QtDBus>

// Copied from PortalFileDialog
QDBusArgument &operator<<(QDBusArgument &arg, const PortalFileDialog::FilterCondition &filterCondition)
{
    arg.beginStructure();
    arg << filterCondition.type << filterCondition.pattern;
    arg.endStructure();
    return arg;
}
const QDBusArgument &operator>>(const QDBusArgument &arg, PortalFileDialog::FilterCondition &filterCondition)
{
    uint type;
    QString filterPattern;
    arg.beginStructure();
    arg >> type >> filterPattern;
    filterCondition.type = (PortalFileDialog::ConditionType)type;
    filterCondition.pattern = filterPattern;
    arg.endStructure();
    return arg;
}
QDBusArgument &operator<<(QDBusArgument &arg, const PortalFileDialog::Filter filter)
{
    arg.beginStructure();
    arg << filter.name << filter.filterConditions;
    arg.endStructure();
    return arg;
}
const QDBusArgument &operator>>(const QDBusArgument &arg, PortalFileDialog::Filter &filter)
{
    QString name;
    PortalFileDialog::FilterConditionList filterConditions;
    arg.beginStructure();
    arg >> name >> filterConditions;
    filter.name = name;
    filter.filterConditions = filterConditions;
    arg.endStructure();
    return arg;
}
#endif

static inline bool checkSandboxEnvironment()
{
    return !QStandardPaths::locate(QStandardPaths::RuntimeLocation, QStringLiteral("flatpak-info")).isEmpty() || qEnvironmentVariableIsSet("SNAP");
}

PortalFileDialog::PortalFileDialog(QObject *parent, WId winId)
    : QObject(parent)
    , m_winId(winId)
{
#ifdef __linux
    QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface();
    m_available = interface->isServiceRegistered(QStringLiteral("org.freedesktop.portal.Desktop"));

    // In case we are in sandbox we can use native dialog which will already use portals
    if (m_available) {
        m_available = !checkSandboxEnvironment();
    }
#endif
}

PortalFileDialog::~PortalFileDialog()
{
}

bool PortalFileDialog::isAvailable() const
{
#ifdef __linux
    return m_available;
#else
    return false;
#endif
}

void PortalFileDialog::open()
{
    if (!m_available) {
        return;
    }

#ifdef __linux
    QDBusMessage message =
        QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.portal.Desktop"), QStringLiteral("/org/freedesktop/portal/desktop"), QStringLiteral("org.freedesktop.portal.FileChooser"), QStringLiteral("OpenFile"));
    QVariantMap options;
    options.insert(QStringLiteral("modal"), true);
    options.insert(QStringLiteral("handle_token"), QStringLiteral("qt%1").arg(QRandomGenerator::global()->generate()));
    options.insert(QStringLiteral("multiple"), false);

    qDBusRegisterMetaType<FilterCondition>();
    qDBusRegisterMetaType<FilterConditionList>();
    qDBusRegisterMetaType<Filter>();
    qDBusRegisterMetaType<FilterList>();

    FilterList filterList;

    FilterConditionList filterConditions1;
    FilterCondition filterCondition1;
    filterCondition1.type = GlobalPattern;
    filterCondition1.pattern = QStringLiteral("*.iso");
    FilterCondition filterCondition2;
    filterCondition2.type = GlobalPattern;
    filterCondition2.pattern = QStringLiteral("*.raw");
    FilterCondition filterCondition3;
    filterCondition3.type = GlobalPattern;
    filterCondition3.pattern = QStringLiteral("*.xz");
    filterConditions1 << filterCondition1 << filterCondition2 << filterCondition3;

    Filter filter1;
    filter1.name = tr("Image files");
    filter1.filterConditions = filterConditions1;

    FilterConditionList filterConditions2;
    FilterCondition filterCondition4;
    filterCondition4.type = GlobalPattern;
    filterCondition4.pattern = QStringLiteral("*");
    filterConditions2 << filterCondition4;

    Filter filter2;
    filter2.name = tr("All files");
    filter2.filterConditions = filterConditions2;

    filterList << filter1 << filter2;

    options.insert(QLatin1String("filters"), QVariant::fromValue(filterList));

    const QString parentWindow = QGuiApplication::platformName() == QStringLiteral("xcb") ? QStringLiteral("x11:%1").arg(QString::number(m_winId)) : QStringLiteral("wayland:%1").arg(QString::number(m_winId));

    message << parentWindow << tr("Open File") << options;

    QDBusPendingCall pendingCall = QDBusConnection::sessionBus().asyncCall(message);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendingCall);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QDBusObjectPath> reply = *watcher;
        if (!reply.isError()) {
            QDBusConnection::sessionBus().connect(nullptr, reply.value().path(), QStringLiteral("org.freedesktop.portal.Request"), QStringLiteral("Response"), this, SLOT(gotResponse(uint, QVariantMap)));
        } else {
            mWarning() << "Failed to open portal file dialog: " << reply.error().message();
        }
    });
#endif
}

void PortalFileDialog::gotResponse(uint response, const QVariantMap &results)
{
    if (!response) {
        if (results.contains(QLatin1String("uris"))) {
            const QStringList selectedFiles = results.value(QLatin1String("uris")).toStringList();
            Q_EMIT fileSelected(selectedFiles.first());
        }
    } else {
        mWarning() << "Portal file dialog cancelled or failed to open";
    }
}
