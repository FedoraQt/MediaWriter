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

#include "drivemanager.h"
#include "releasemanager.h"
#include "options.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    options.parse(app.arguments());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("drives", new DriveManager());
    engine.rootContext()->setContextProperty("releases", new ReleaseManager());
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
