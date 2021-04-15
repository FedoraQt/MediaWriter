/*
 * Fedora Media Writer
 * Copyright 2013 Marco Martin <mart@kde.org>
 * Copyright 2014 Sebastian Kügler <sebas@kde.org>
 * Copyright 2014 David Edmundson <davidedmunsdon@kde.org>
 * Copyright (C) 2020 Jan Grulich <jgrulich@redhat.com>
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

#include "units.h"

#include <QFontMetrics>
#include <QGuiApplication>
#include <QScreen>

Units *Units::_self = nullptr;

Units::Units(QObject *parent)
    : QObject(parent)
    , m_devicePixelRatio(-1)
    , m_gridUnit(-1)
    , m_smallSpacing(-1)
    , m_largeSpacing(-1)
{
    update();
    updateDevicePixelRatio();
}

Units *Units::instance()
{
    if (!_self)
        _self = new Units();
    return _self;
}

qreal Units::devicePixelRatio() const
{
    return m_devicePixelRatio;
}

int Units::gridUnit() const
{
    return m_gridUnit;
}

int Units::smallSpacing() const
{
    return m_smallSpacing;
}

int Units::largeSpacing() const
{
    return m_largeSpacing;
}

bool Units::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange) {
            update();
        }
    }

    return QObject::eventFilter(watched, event);
}

void Units::update()
{
    int gridUnit = QFontMetrics(QGuiApplication::font()).boundingRect(QStringLiteral("M")).height();

    if (gridUnit % 2 != 0) {
        gridUnit++;
    }

    if (gridUnit != m_gridUnit) {
        m_gridUnit = gridUnit;
        Q_EMIT gridUnitChanged();
    }

    if (gridUnit != m_largeSpacing) {
        m_smallSpacing = qMax(2, (int)(gridUnit / 4)); // 1/4 of gridUnit, at least 2
        m_largeSpacing = m_smallSpacing * 2;
        Q_EMIT spacingChanged();
    }
}

void Units::updateDevicePixelRatio()
{
    // Using QGuiApplication::devicePixelRatio() gives too coarse values,
    // i.e. it directly jumps from 1.0 to 2.0. We want tighter control on
    // sizing, so we compute the exact ratio and use that.
    // TODO: make it possible to adapt to the dpi for the current screen dpi
    //  instead of assuming that all of them use the same dpi which applies for
    //  X11 but not for other systems.
    QScreen *primary = QGuiApplication::primaryScreen();
    if (!primary) {
        return;
    }
    const qreal dpi = primary->logicalDotsPerInchX();
    // Usual "default" is 96 dpi
    // that magic ratio follows the definition of "device independent pixel" by Microsoft
    m_devicePixelRatio = (qreal)dpi / (qreal)96;
    emit devicePixelRatioChanged();
}
