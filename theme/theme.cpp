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
#include <QCheckBox>

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

QColor AdwaitaTheme::getButtonBottomColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed)
{
    // Give destructive action higher priority and leave "highlighted" as fallback
    if (destructiveAction) {
        if (pressed) {
            return m_darkMode ? QColor("#8a1116") : QColor("#a0131a");
        } else if (hovered) {
            // FIXME: this is not 1:1 from Adwaita, but we need to make it
            // lighter to highlight the hover
            return m_darkMode ? Adwaita::Colors::lighten(QColor("#ae151c"), 0.01) : QColor("#e01b24");
        } else {
            return m_darkMode ? QColor("#ae151c") : QColor("#ce1921");
        }
    }

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

QColor AdwaitaTheme::getButtonTopColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed)
{
    if (destructiveAction) {
        if (pressed) {
            return m_darkMode ? QColor("#8a1116") : QColor("#a0131a");
        } else if (hovered) {
            // FIXME: this is not 1:1 from Adwaita, but we need to make it
            // lighter to highlight the hover
            return m_darkMode ? Adwaita::Colors::lighten(QColor("#b2161d"), 0.01) : QColor("#e41c26");
        } else {
            return m_darkMode ? QColor("#b2161d") : QColor("#e01b24");
        }
    }

    if (highlighted) {
        if (pressed) {
            return m_darkMode ? QColor("#103e75") : QColor("#1961b9");
        } else if (hovered) {
            return m_darkMode ? QColor("#1655a2") : QColor("#3987e5");
        } else {
            return m_darkMode ? QColor("#15539e") : QColor("#3483e4");
        }
    }

    const QColor color = getButtonBottomColor(highlighted, destructiveAction, hovered, pressed);

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

QColor AdwaitaTheme::getButtonOutlineColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed)
{
    if (destructiveAction) {
        return m_darkMode ? QColor("#851015") : QColor("#b2161d");
    }

    if (highlighted) {
        return m_darkMode ? QColor("#0f3b71") : QColor("#1b6acb");
    }

    Adwaita::StyleOptions styleOptions(m_palette);
    styleOptions.setMouseOver(hovered);
    styleOptions.setSunken(pressed);
    return Adwaita::Colors::buttonOutlineColor(styleOptions);
}

QColor AdwaitaTheme::getCheckBoxBottomColor(bool hovered, bool pressed, bool checked)
{
    Adwaita::StyleOptions styleOptions(m_palette);
    styleOptions.setMouseOver(hovered);
    styleOptions.setSunken(pressed);
    styleOptions.setCheckboxState(checked ? Adwaita::CheckBoxState::CheckOn : Adwaita::CheckBoxState::CheckOff);

    const QColor color = Adwaita::Colors::indicatorBackgroundColor(styleOptions);

    // TODO: separate this from Adwaita::Helper::renderCheckBoxFrame()
    if (!checked) {
        if (pressed) {
            // Pressed button in normal and dark mode is not a gradient, but just an image consting from same $color
            return color;
        } else if (hovered) {
            QColor baseColor = m_darkMode ? color : Adwaita::Colors::darken(color, 0.09);
            return m_darkMode ? Adwaita::Colors::darken(baseColor, 0.04) : color;
        } else {
            QColor baseColor = m_darkMode ? Adwaita::Colors::lighten(color, 0.03) : Adwaita::Colors::darken(color, 0.05);
            return m_darkMode ? Adwaita::Colors::darken(baseColor, 0.06) : baseColor;
        }
    } else {
        return Adwaita::Colors::lighten(color, 0.04);
    }
}

QColor AdwaitaTheme::getCheckBoxTopColor(bool hovered, bool pressed, bool checked)
{
    const QColor color = getCheckBoxBottomColor(hovered, pressed, checked);

    // TODO: separate this from Adwaita::Helper::renderCheckBoxFrame()
    if (!checked) {
        if (pressed) {
            // Pressed button in normal and dark mode is not a gradient, but just an image consting from same $color
            return color;
        } else if (hovered) {
            QColor baseColor = m_darkMode ? color : Adwaita::Colors::darken(color, 0.09);
            return m_darkMode ? color : Adwaita::Colors::lighten(baseColor, 0.04);
        } else {
            return m_darkMode ? color : color;
        }
    } else {
        return Adwaita::Colors::lighten(color, 0.04);
    }
}

QColor AdwaitaTheme::getCheckBoxOutlineColor(bool hovered, bool pressed, bool checked)
{
    Adwaita::StyleOptions styleOptions(m_palette);
    styleOptions.setMouseOver(hovered);
    styleOptions.setSunken(pressed);
    styleOptions.setCheckboxState(checked ? Adwaita::CheckBoxState::CheckOn : Adwaita::CheckBoxState::CheckOff);
    return Adwaita::Colors::indicatorOutlineColor(styleOptions);
}

