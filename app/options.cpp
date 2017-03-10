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

#include "options.h"
#include <QTextStream>

Options options;

// this is slowly getting out of hand
// when adding an another option, please consider using a real argv parser

void Options::parse(QStringList argv) {
    int index;
    if (argv.contains("--testing"))
        testing = true;
    if (argv.contains("--verbose") || argv.contains("-v")) {
        verbose = true;
        logging = false;
    }
    if (argv.contains("--logging") || argv.contains("-l"))
        logging = true;
    if ((index = argv.indexOf("--releasesUrl")) >= 0) {
        if (index >= argv.length() - 1)
            printHelp();
        else
            releasesUrl = argv[index + 1];
    }
    if (argv.contains("--no-user-agent")) {
        noUserAgent = true;
    }
    if (argv.contains("--help")) {
        printHelp();
    }
}

void Options::printHelp() {
    QTextStream out(stdout);
    out << "mediawriter [--testing] [--no-user-agent] [--releasesUrl <url>]\n";
}
