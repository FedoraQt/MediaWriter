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

#include <stdexcept>

#include <QCoreApplication>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QTranslator>

// Platform specific drive handler.
#include "drive.h"

int main(int argc, char *argv[]) {
    const QString action = argv[1];
    bool isRestore = argc == 3 && action == "restore";
    bool isWrite = argc == 4 && action == "write";
    QTextStream err(stderr);
    if (!isRestore && !isWrite) {
        err << "Helper: Wrong arguments entered\n";
        err.flush();
        return 1;
    }

    QCoreApplication app(argc, argv);
    QTranslator translator;
    translator.load(QLocale(), QString(), QString(), ":/translations");
    app.installTranslator(&translator);

    QString driveIdentifier = isRestore ? argv[2] : argv[3];
    QTimer::singleShot(0, [&]() {
        Drive *drive = new Drive(driveIdentifier);
        try {
            drive->init();
            if (isRestore) {
                drive->umount();
                drive->wipe();
            }
            else {
                drive->writeIso(argv[2]);
            }
            qApp->exit(0);
        } catch (std::runtime_error &error) {
            err << error.what() << '\n';
            err.flush();
            qApp->exit(1);
        }
    });
    return app.exec();
}
