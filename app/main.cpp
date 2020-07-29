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
#include "releasemanager.h"
#include "versionchecker.h"

#ifdef __linux
#include <AdwaitaQt/adwaitacolors.h>
#endif

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
    MessageHandler::install();
    CrashHandler::install();

#ifdef __linux
    if (QGuiApplication::platformName() == QStringLiteral("xcb")) {
        if (qEnvironmentVariableIsEmpty("QSG_RENDER_LOOP"))
            qputenv("QSG_RENDER_LOOP", "threaded");
        qputenv("GDK_BACKEND", "x11");
    }
#endif

    QApplication::setOrganizationDomain("fedoraproject.org");
    QApplication::setOrganizationName("fedoraproject.org");
    QApplication::setApplicationName("MediaWriter");

#ifdef __linux
    // qt x11 scaling is broken
    if (QGuiApplication::platformName() == QStringLiteral("xcb"))
#endif
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication app(argc, argv);
    options.parse(app.arguments());

    // considering how often we hit driver issues, I have decided to force
    // the QML software renderer on Windows and Linux, since Qt 5.9
    if (qEnvironmentVariableIsEmpty("QMLSCENE_DEVICE"))
        qputenv("QMLSCENE_DEVICE", "softwarecontext");

    mDebug() << "Application constructed";

    QTranslator translator;
    translator.load(QLocale(QLocale().language(), QLocale().country()), QString(), QString(), ":/translations");
    app.installTranslator(&translator);

#ifdef __linux
    QQuickStyle::setStyle("org.fedoraproject.AdwaitaTheme");

    QPalette adwaitaPalette = Adwaita::Colors::palette();
    QGuiApplication::setPalette(adwaitaPalette);
#endif

    mDebug() << "Injecting QML context properties";
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("drives", DriveManager::instance());
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());
    engine.rootContext()->setContextProperty("downloadManager", DownloadManager::instance());
    engine.rootContext()->setContextProperty("mediawriterVersion", MEDIAWRITER_VERSION);
    engine.rootContext()->setContextProperty("versionChecker", new VersionChecker());
#if (defined(__linux) || defined(_WIN32))
    engine.rootContext()->setContextProperty("platformSupportsDelayedWriting", true);
#else
    engine.rootContext()->setContextProperty("platformSupportsDelayedWriting", false);
#endif
    mDebug() << "Loading the QML source code";

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    mDebug() << "Starting the application";
    int status = app.exec();
    mDebug() << "Quitting with status" << status;

    return status;
}
