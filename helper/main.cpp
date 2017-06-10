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

#include <QString>
#include <QTextStream>
#include <QTranslator>

// Platform specific drive handler.
#include "drive.h"
#include "write.h"

void restore(Drive *const drive) {
    drive->umount();
    drive->wipe();
    drive->addPartition();
}

int main(int argc, char *argv[]) {
    const QString action = argv[1];
    bool isRestore = argc == 3 && action == "restore";
    bool persistentStorage = argc == 5;
    bool isWrite = (argc == 4 || persistentStorage) && action == "write";
    QTextStream err(stderr);
    if (!isRestore && !isWrite) {
        err << "Helper: Wrong arguments entered\n";
        err.flush();
        return 1;
    }

    QTranslator translator;
    translator.load(QLocale(), QString(), QString(), ":/translations");

    QString driveIdentifier = isRestore ? argv[2] : argv[3];
    Drive drive(driveIdentifier);
    try {
        if (isRestore) {
            restore(&drive);
        }
        else {
            write(argv[2], &drive, persistentStorage);
        }
    } catch (std::runtime_error &error) {
        auto errorMessage = error.what();
        QString translatedMessage = translator.translate(nullptr, errorMessage);
        if (translatedMessage.isNull()) {
            err << errorMessage << '\n';
        }
        else {
            err << translatedMessage << '\n';
        }
        err.flush();
        return 1;
    }
}
