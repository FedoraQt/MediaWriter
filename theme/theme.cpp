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

    // TODO: keep synced with Adwaita-qt or use Adwaita-qt library
    if (!m_darkMode) {
        QColor base_color = QColor("white");
        QColor text_color = QColor("black");
        QColor bg_color = QColor("#f6f5f4");
        QColor fg_color = QColor("#2e3436");
        QColor selected_bg_color = QColor("#3584e4");
        QColor selected_fg_color = QColor("white");
        QColor osd_text_color = QColor("white");
        QColor osd_bg_color = QColor("black");
        QColor shadow = transparentize(QColor("black"), 0.9);

        QColor backdrop_fg_color = mix(fg_color, bg_color);
        QColor backdrop_base_color = darken(base_color, 0.01);
        QColor backdrop_selected_fg_color = backdrop_base_color;

        // This is the color we use as initial color for the gradient in normal state
        // Defined in _drawing.scss button(normal)
        QColor button_base_color = darken(bg_color, 0.04);

        QColor link_color = darken(selected_bg_color, 0.1);
        QColor link_visited_color = darken(selected_bg_color, 0.2);

        m_palette.setColor(QPalette::All,      QPalette::Window,          bg_color);
        m_palette.setColor(QPalette::All,      QPalette::WindowText,      fg_color);
        m_palette.setColor(QPalette::All,      QPalette::Base,            base_color);
        m_palette.setColor(QPalette::All,      QPalette::AlternateBase,   base_color);
        m_palette.setColor(QPalette::All,      QPalette::ToolTipBase,     osd_bg_color);
        m_palette.setColor(QPalette::All,      QPalette::ToolTipText,     osd_text_color);
        m_palette.setColor(QPalette::All,      QPalette::Text,            fg_color);
        m_palette.setColor(QPalette::All,      QPalette::Button,          button_base_color);
        m_palette.setColor(QPalette::All,      QPalette::ButtonText,      fg_color);
        m_palette.setColor(QPalette::All,      QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::All,      QPalette::Light,           lighten(button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Midlight,        mix(lighten(button_base_color), button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Mid,             mix(darken(button_base_color), button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Dark,            darken(button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::All,      QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::All,      QPalette::HighlightedText, selected_fg_color);

        m_palette.setColor(QPalette::All,      QPalette::Link,            link_color);
        m_palette.setColor(QPalette::All,      QPalette::LinkVisited,     link_visited_color);

        QColor insensitive_fg_color = mix(fg_color, bg_color);
        QColor insensitive_bg_color = mix(bg_color, base_color, 0.4);

        m_palette.setColor(QPalette::Disabled, QPalette::Window,          insensitive_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::WindowText,      insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Base,            base_color);
        m_palette.setColor(QPalette::Disabled, QPalette::AlternateBase,   base_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Text,            insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Button,          insensitive_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::ButtonText,      insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::Disabled, QPalette::Light,           lighten(insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Dark,            darken(insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::Disabled, QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::HighlightedText, selected_fg_color);

        m_palette.setColor(QPalette::Disabled, QPalette::Link,            link_color);
        m_palette.setColor(QPalette::Disabled, QPalette::LinkVisited,     link_visited_color);

        m_palette.setColor(QPalette::Inactive, QPalette::Window,          bg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::WindowText,      backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Base,            backdrop_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::AlternateBase,   backdrop_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Text,            backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Button,          button_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::ButtonText,      backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::Inactive, QPalette::Light,           lighten(insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Dark,            darken(insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::Inactive, QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::HighlightedText, backdrop_selected_fg_color);

        m_palette.setColor(QPalette::Inactive, QPalette::Link,            link_color);
        m_palette.setColor(QPalette::Inactive, QPalette::LinkVisited,     link_visited_color);
    } else {
        QColor base_color = lighten(desaturate(QColor("#241f31"), 1.0), 0.02);
        QColor text_color = QColor("white");
        QColor bg_color = darken(desaturate(QColor("#3d3846"), 1.0), 0.04);
        QColor fg_color = QColor("#eeeeec");
        QColor selected_bg_color = darken(QColor("#3584e4"), 0.2);
        QColor selected_fg_color = QColor("white");
        QColor osd_text_color = QColor("white");
        QColor osd_bg_color = QColor("black");
        QColor shadow = transparentize(QColor("black"), 0.9);

        QColor backdrop_fg_color = mix(fg_color, bg_color);
        QColor backdrop_base_color = lighten(base_color, 0.01);
        QColor backdrop_selected_fg_color = mix(text_color, backdrop_base_color, 0.2);

        // This is the color we use as initial color for the gradient in normal state
        // Defined in _drawing.scss button(normal)
        QColor button_base_color = darken(bg_color, 0.01);

        QColor link_color = lighten(selected_bg_color, 0.2);
        QColor link_visited_color = lighten(selected_bg_color, 0.1);

        m_palette.setColor(QPalette::All,      QPalette::Window,          bg_color);
        m_palette.setColor(QPalette::All,      QPalette::WindowText,      fg_color);
        m_palette.setColor(QPalette::All,      QPalette::Base,            base_color);
        m_palette.setColor(QPalette::All,      QPalette::AlternateBase,   base_color);
        m_palette.setColor(QPalette::All,      QPalette::ToolTipBase,     osd_bg_color);
        m_palette.setColor(QPalette::All,      QPalette::ToolTipText,     osd_text_color);
        m_palette.setColor(QPalette::All,      QPalette::Text,            fg_color);
        m_palette.setColor(QPalette::All,      QPalette::Button,          button_base_color);
        m_palette.setColor(QPalette::All,      QPalette::ButtonText,      fg_color);
        m_palette.setColor(QPalette::All,      QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::All,      QPalette::Light,           lighten(button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Midlight,        mix(lighten(button_base_color), button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Mid,             mix(darken(button_base_color), button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Dark,            darken(button_base_color));
        m_palette.setColor(QPalette::All,      QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::All,      QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::All,      QPalette::HighlightedText, selected_fg_color);

        m_palette.setColor(QPalette::All,      QPalette::Link,            link_color);
        m_palette.setColor(QPalette::All,      QPalette::LinkVisited,     link_visited_color);


        QColor insensitive_fg_color = mix(fg_color, bg_color);
        QColor insensitive_bg_color = mix(bg_color, base_color, 0.4);

        m_palette.setColor(QPalette::Disabled, QPalette::Window,          insensitive_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::WindowText,      insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Base,            base_color);
        m_palette.setColor(QPalette::Disabled, QPalette::AlternateBase,   base_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Text,            insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::Button,          insensitive_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::ButtonText,      insensitive_fg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::Disabled, QPalette::Light,           lighten(insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Dark,            darken(insensitive_bg_color));
        m_palette.setColor(QPalette::Disabled, QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::Disabled, QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::Disabled, QPalette::HighlightedText, selected_fg_color);

        m_palette.setColor(QPalette::Disabled, QPalette::Link,            link_color);
        m_palette.setColor(QPalette::Disabled, QPalette::LinkVisited,     link_visited_color);


        m_palette.setColor(QPalette::Inactive, QPalette::Window,          bg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::WindowText,      backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Base,            backdrop_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::AlternateBase,   backdrop_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Text,            backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::Button,          button_base_color);
        m_palette.setColor(QPalette::Inactive, QPalette::ButtonText,      backdrop_fg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::BrightText,      text_color);

        m_palette.setColor(QPalette::Inactive, QPalette::Light,           lighten(insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Midlight,        mix(lighten(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Mid,             mix(darken(insensitive_bg_color), insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Dark,            darken(insensitive_bg_color));
        m_palette.setColor(QPalette::Inactive, QPalette::Shadow,          shadow);

        m_palette.setColor(QPalette::Inactive, QPalette::Highlight,       selected_bg_color);
        m_palette.setColor(QPalette::Inactive, QPalette::HighlightedText, backdrop_selected_fg_color);

        m_palette.setColor(QPalette::Inactive, QPalette::Link,            link_color);
        m_palette.setColor(QPalette::Inactive, QPalette::LinkVisited,     link_visited_color);
    }
}

AdwaitaTheme *AdwaitaTheme::qmlAttachedProperties(QObject *object)
{
    return new AdwaitaTheme(object);
}

QColor AdwaitaTheme::buttonOutlineColor() const
{
    if (!m_darkMode) {
        return darken(m_palette.color(QPalette::Window), 0.18);
    } else {
       return darken(m_palette.color(QPalette::Window), 0.1);
    }
}
