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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QLoggingCategory>
#include <QObject>

class Progress;
class Options;
class MessageHandler;

/**
 * @brief The Progress class
 *
 * Reports the percentual/ratio progress of some activity to the user
 *
 * @property from the minimum value of the reported process
 * @property to the maximum value of the reported process
 * @property value the current value of the process
 * @property ratio the ratio of the value, is in the range [0.0, 1.0]
 */
class Progress : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal from READ from CONSTANT)
    Q_PROPERTY(qreal to READ to NOTIFY toChanged)
    Q_PROPERTY(qreal value READ value NOTIFY valueChanged)
    Q_PROPERTY(qreal ratio READ ratio NOTIFY valueChanged)
public:
    Progress(QObject *parent = nullptr, qreal from = 0.0, qreal to = 1.0);

    qreal from() const;
    qreal to() const;
    qreal value() const;
    qreal ratio() const;

    void setTo(qreal v);
    void setValue(qreal v);
    void setValue(qreal v, qreal to);

signals:
    void valueChanged();
    void toChanged();

public slots:
    void update(qreal value);
    void reset();

private:
    qreal m_from{0.0};
    qreal m_to{1.0};
    qreal m_value{0.0};
};

class Options
{
public:
    void parse(QStringList argv);
    void printHelp();

    bool testing{false};
    bool verbose{false};
#if defined(QT_NO_DEBUG) && !defined(_WIN32)
    bool logging{false};
#else
    bool logging{true};
#endif
    // Point to GitHub Pages hosted releases.json - auto-updated daily
    QString releasesUrl{"https://raw.githubusercontent.com/xXJSONDeruloXx/MediaWriter/gh-pages/releases.json"};
    bool noUserAgent{false}; // disables sending the custom Fedora Media Writer user agent header
};

class MessageHandler
{
public:
    static void install();
    static QLoggingCategory category;
};

#define mInfo() qCInfo(MessageHandler::category)
#define mDebug() qCDebug(MessageHandler::category)
#define mWarning() qCWarning(MessageHandler::category)
#define mCritical() qCCritical(MessageHandler::category)
#define mFatal() qCFatal(MessageHandler::category)

extern Options options;

#endif // UTILITIES_H
