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

#include "utilities.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QStandardPaths>

Options options;

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
static QElapsedTimer timer;
static FILE *debugFile;

// TODO: everything Q_UNUSED

Progress::Progress(QObject *parent, qreal from, qreal to)
    : QObject(parent)
    , m_from(from)
    , m_to(to)
    , m_value(from)
{
    connect(this, &Progress::toChanged, this, &Progress::valueChanged);
}

qreal Progress::from() const
{
    return m_from;
}

qreal Progress::to() const
{
    return m_to;
}

qreal Progress::value() const
{
    return m_value;
}

qreal Progress::ratio() const
{
    return (value() - from()) / (to() - from());
}

void Progress::setTo(qreal v)
{
    if (m_to != v) {
        m_to = v;
        emit toChanged();
    }
}

void Progress::setValue(qreal v)
{
    if (m_value != v) {
        m_value = v;
        emit valueChanged();
    }
}

void Progress::setValue(qreal v, qreal to)
{
    qreal computedValue = v / to * (m_to - m_from) + m_from;
    if (computedValue != m_value) {
        m_value = computedValue;
        emit valueChanged();
    }
}

void Progress::update(qreal value)
{
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}

void Progress::reset()
{
    update(from());
}

// this is slowly getting out of hand
// when adding an another option, please consider using a real argv parser

void Options::parse(QStringList argv)
{
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

    if (options.logging) {
        QString debugFileName = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BazziteMediaWriter.log";
        debugFile = fopen(debugFileName.toStdString().c_str(), "w");
        if (!debugFile) {
            debugFile = stderr;
        }
    }
}

void Options::printHelp()
{
    QTextStream out(stdout);
    out << "mediawriter [--testing] [--no-user-agent] [--releasesUrl <url>]\n";
}

static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        if (options.verbose || options.logging)
            fprintf(debugFile, "D");
        break;
    case QtInfoMsg:
        fprintf(debugFile, "I");
        break;
    case QtWarningMsg:
        fprintf(debugFile, "W");
        break;
    case QtCriticalMsg:
        fprintf(debugFile, "C");
        break;
    case QtFatalMsg:
        fprintf(debugFile, "F");
    }
    if ((type == QtDebugMsg && (options.verbose || options.logging)) || type != QtDebugMsg) {
        if (context.line > 0)
            fprintf(debugFile, "@%lldms: %s (%s:%u)\n", timer.elapsed(), localMsg.constData(), context.file, context.line);
        else
            fprintf(debugFile, "@%lldms: %s\n", timer.elapsed(), localMsg.constData());
        fflush(debugFile);
    }
    if (type == QtFatalMsg)
        exit(1);
}

void MessageHandler::install()
{
    timer.start();
    debugFile = stderr;
    qInstallMessageHandler(myMessageOutput); // Install the handler
}

QLoggingCategory MessageHandler::category{"org.bazzite.MediaWriter"};
