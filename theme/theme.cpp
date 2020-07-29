/*
 * Fedora Media Writer
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

#include "theme.h"

#include <QGuiApplication>

AdwaitaTheme::AdwaitaTheme(QObject *parent)
    : QObject(parent)
    , m_darkMode(false)
{
    // Just guess dark mode for now based on text color
    const QColor textColor = QGuiApplication::palette().color(QPalette::Text);
    if ((textColor.redF() * 0.299 + textColor.greenF() * 0.587 + textColor.blueF() * 0.114) <= 186) {
        m_darkMode = true;
    }

    m_palette = Adwaita::Colors::palette();
}

AdwaitaTheme *AdwaitaTheme::qmlAttachedProperties(QObject *object)
{
    return new AdwaitaTheme(object);
}

QColor AdwaitaTheme::buttonOutlineColor() const
{
    return Adwaita::Colors::buttonOutlineColor(Adwaita::StyleOptions(m_palette));
}

QColor AdwaitaTheme::getButtonBottomColor(bool highlighted, bool hovered, bool pressed)
{
    if (highlighted) {
        if (pressed) {
            return m_darkMode ? QColor("#103e75") : QColor("#1961b9");
        } else if (hovered) {
            return m_darkMode ? QColor("#155099") : QColor("#3584e4");
        } else {
            return m_darkMode ? QColor("#155099") : QColor("#2379e2");
        }
    }

    Adwaita::StyleOptions styleOptions(m_palette);
    styleOptions.setMouseOver(hovered);
    styleOptions.setSunken(pressed);

    const QColor color = Adwaita::Colors::buttonBackgroundColor(styleOptions);

    // FIXME
    return hovered && m_darkMode ? Adwaita::Colors::lighten(color, 0.01) : color;
}

QColor AdwaitaTheme::getButtonTopColor(bool highlighted, bool hovered, bool pressed)
{
    if (highlighted) {
        if (pressed) {
            return m_darkMode ? QColor("#103e75") : QColor("#1961b9");
        } else if (hovered) {
            return m_darkMode ? QColor("#1655a2") : QColor("#3987e5");
        } else {
            return m_darkMode ? QColor("#15539e") : QColor("#3483e4");
        }
    }

    const QColor color = getButtonBottomColor(highlighted, hovered, pressed);

    // TODO: separate this from Adwaita::Helper::renderButtonFrame()
    if (pressed) {
        // Pressed button in normal and dark mode is not a gradient, but just an image consting from same $color
        return color;
    } else if (hovered) {
        return m_darkMode ? Adwaita::Colors::lighten(color, 0.02) : Adwaita::Colors::lighten(color, 0.01);
    } else {
        return m_darkMode ? Adwaita::Colors::lighten(color, 0.01) : Adwaita::Colors::lighten(color, 0.04);
    }
}
