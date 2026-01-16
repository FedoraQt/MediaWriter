/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulichredhat.com>
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
#include <QTranslator>

#include "crashhandler.h"
#include "drivemanager.h"
#include "portalfiledialog.h"
#include "releasemanager.h"

int main(int argc, char **argv)
{
    MessageHandler::install();
    CrashHandler::install();

#ifdef __linux
    // For some reason threaded renderer makes all the animations slow for me
    // so as a fallback force non-threaded renderer
    if (qEnvironmentVariableIsEmpty("QSG_RENDER_LOOP")) {
        qputenv("QSG_RENDER_LOOP", "basic");
    }
#endif

    QApplication::setOrganizationDomain("fedoraproject.org");
    QApplication::setOrganizationName("fedoraproject.org");
    QApplication::setApplicationName("MediaWriter");

    QApplication app(argc, argv);
    options.parse(app.arguments());

    mDebug() << "Application constructed";

    QTranslator translator;
    QLocale locale(QLocale::system().language(), QLocale::system().script(), QLocale::system().territory());
    if (translator.load(locale, QLatin1String(), QLatin1String(), ":/translations")) {
        mDebug() << "Localization " << locale;
        app.installTranslator(&translator);
    }

    QGuiApplication::setDesktopFileName("org.fedoraproject.MediaWriter");

    mDebug() << "Injecting QML context properties";
    QQmlApplicationEngine engine;

    engine.rootContext()->setContextProperty("downloadManager", DownloadManager::instance());
    engine.rootContext()->setContextProperty("drives", DriveManager::instance());
    engine.rootContext()->setContextProperty("restoreableDrives", new RestoreableDriveManager());
    engine.rootContext()->setContextProperty("portalFileDialog", new PortalFileDialog(&app));
    engine.rootContext()->setContextProperty("mediawriterVersion", MEDIAWRITER_VERSION);
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());

    mDebug() << "Loading the QML source code";

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    mDebug() << "Starting the application";
    int status = app.exec();
    mDebug() << "Quitting with status" << status;

    return status;
}
