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

#include "crashhandler.h"
#include "drivemanager.h"
#include "releasemanager.h"

#if QT_VERSION < 0x050300
# error "Minimum supported Qt version is 5.3.0"
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
    char *qsgLoop = getenv("QSG_RENDER_LOOP");
    if (!qsgLoop)
        setenv("QSG_RENDER_LOOP", "threaded", 1);
    setenv("GDK_BACKEND", "x11", 1);
#endif

    QApplication::setOrganizationDomain("fedoraproject.org");
    QApplication::setOrganizationName("fedoraproject.org");
    QApplication::setApplicationName("MediaWriter");
#ifndef __linux
    // qt x11 scaling is broken
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QApplication app(argc, argv);
    options.parse(app.arguments());

    qDebug() << "Application constructed";

    QTranslator translator;
    translator.load(QLocale(QLocale().language()), QString(), QString(), ":/translations");
    app.installTranslator(&translator);

    qDebug() << "Injecting QML context properties";
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("drives", DriveManager::instance());
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());
    engine.rootContext()->setContextProperty("downloadManager", DownloadManager::instance());
    engine.rootContext()->setContextProperty("mediawriterVersion", MEDIAWRITER_VERSION);
    qDebug() << "Loading the QML source code";
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    qDebug() << "Starting the application";
    int status = app.exec();
    qDebug() << "Quitting with status" << status;

    return status;
}
