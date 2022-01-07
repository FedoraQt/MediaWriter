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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QLoggingCategory>
#include <QTranslator>
#include <QDebug>
#include <QScreen>
#include <QtPlugin>
#include <QElapsedTimer>
#include <QStandardPaths>
#include <QQuickStyle>

#include "crashhandler.h"
#include "drivemanager.h"
#include "icon.h"
#include "portalfiledialog.h"
#include "releasemanager.h"
#include "units.h"
#include "versionchecker.h"

#include <AdwaitaQt/adwaitacolors.h>

#ifdef QT_STATIC
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);

Q_IMPORT_PLUGIN(QtQuick2Plugin);
Q_IMPORT_PLUGIN(QtQuick2WindowPlugin);
Q_IMPORT_PLUGIN(QtQuick2DialogsPlugin);
Q_IMPORT_PLUGIN(QtQuick2DialogsPrivatePlugin);
Q_IMPORT_PLUGIN(QtQuickControls1Plugin);
Q_IMPORT_PLUGIN(QtQuickLayoutsPlugin);
Q_IMPORT_PLUGIN(QmlFolderListModelPlugin);
Q_IMPORT_PLUGIN(QmlSettingsPlugin);
#endif

int main(int argc, char **argv)
{
    //MessageHandler::install();
    CrashHandler::install();

#ifdef __linux
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        qputenv("GDK_BACKEND", "x11");
    }
    // For some reason threaded renderer makes all the animations slow for me
    // so as a fallback force non-threaded renderer
    if (qEnvironmentVariableIsEmpty("QSG_RENDER_LOOP")) {
        qputenv("QSG_RENDER_LOOP", "basic");
    }
#endif

    QApplication::setOrganizationDomain("fedoraproject.org");
    QApplication::setOrganizationName("fedoraproject.org");
    QApplication::setApplicationName("MediaWriter");

    QQuickStyle::setStyle("QtQuick.Controls.org.fedoraproject.AdwaitaTheme");
    QApplication app(argc, argv);
    options.parse(app.arguments());

    mDebug() << "Application constructed";

    QTranslator translator;
    if (translator.load(QLocale(QLocale().language(), QLocale().country()), QLatin1String(), QLatin1String(), ":/translations"))
        app.installTranslator(&translator);

    QPalette adwaitaPalette = Adwaita::Colors::palette();
    QGuiApplication::setPalette(adwaitaPalette);
    QGuiApplication::setDesktopFileName("org.fedoraproject.MediaWriter.desktop");

    mDebug() << "Injecting QML context properties";
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("downloadManager", DownloadManager::instance());
    engine.rootContext()->setContextProperty("drives", DriveManager::instance());
    engine.rootContext()->setContextProperty("portalFileDialog", new PortalFileDialog(&app));
    engine.rootContext()->setContextProperty("mediawriterVersion", MEDIAWRITER_VERSION);
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());
    engine.rootContext()->setContextProperty("units", Units::instance());
    engine.rootContext()->setContextProperty("versionChecker", new VersionChecker());
#if (defined(__linux) || defined(_WIN32))
    engine.rootContext()->setContextProperty("platformSupportsDelayedWriting", true);
#else
    engine.rootContext()->setContextProperty("platformSupportsDelayedWriting", false);
#endif

    qmlRegisterType<Icon>("MediaWriter", 1, 0, "Icon");

    mDebug() << "Loading the QML source code";

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    mDebug() << "Starting the application";
    int status = app.exec();
    mDebug() << "Quitting with status" << status;

    return status;
}
