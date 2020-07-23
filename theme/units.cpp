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

AdwaitaUnits::AdwaitaUnits(QObject *parent)
    : QObject(parent)
    , m_gridUnit(-1)
    , m_smallSpacing(-1)
    , m_largeSpacing(-1)
{
    update();
}

AdwaitaUnits *AdwaitaUnits::qmlAttachedProperties(QObject *object)
{
    return new AdwaitaUnits(object);
}

int AdwaitaUnits::gridUnit() const
{
    return m_gridUnit;
}

int AdwaitaUnits::smallSpacing() const
{
    return m_smallSpacing;
}

int AdwaitaUnits::largeSpacing() const
{
    return m_largeSpacing;
}

bool AdwaitaUnits::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == QCoreApplication::instance()) {
        if (event->type() == QEvent::ApplicationFontChange) {
            update();
        }
    }

    return QObject::eventFilter(watched, event);
}

void AdwaitaUnits::update()
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
        m_largeSpacing = gridUnit; // msize.height
        Q_EMIT spacingChanged();
    }
}
