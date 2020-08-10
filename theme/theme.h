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

#ifndef ADWAITA_THEME_H
#define ADWAITA_THEME_H

#include <QColor>
#include <QObject>
#include <QPalette>
#include <QtQml>

#include <AdwaitaQt/adwaita.h>
#include <AdwaitaQt/adwaitacolors.h>

class AdwaitaTheme : public QObject
{
    Q_OBJECT
    // Button
    Q_PROPERTY(int buttonMarginHeight READ buttonMarginHeight CONSTANT)
    Q_PROPERTY(int buttonMarginWidth READ buttonMarginWidth CONSTANT)
    Q_PROPERTY(int buttonMinimumHeight READ buttonMinimumHeight CONSTANT)
    Q_PROPERTY(int buttonMinimumWidth READ buttonMinimumWidth CONSTANT)

    // Frame
    Q_PROPERTY(int frameRadius READ frameRadius CONSTANT)
    Q_PROPERTY(int frameWidth READ frameWidth CONSTANT)

    // Colors
    Q_PROPERTY(QColor baseColor READ baseColor CONSTANT)
    Q_PROPERTY(QColor buttonColor READ buttonColor CONSTANT)
    Q_PROPERTY(QColor buttonOutlineColor READ buttonOutlineColor CONSTANT)
    Q_PROPERTY(QColor disabledTextColor READ disabledTextColor CONSTANT)
    Q_PROPERTY(QColor highlightColor READ highlightColor CONSTANT)
    Q_PROPERTY(QColor highlightTextColor READ highlightTextColor CONSTANT)
    Q_PROPERTY(QColor linkColor READ linkColor CONSTANT)
    Q_PROPERTY(QColor textColor READ textColor CONSTANT)
    Q_PROPERTY(QColor windowColor READ windowColor CONSTANT)

    Q_PROPERTY(QColor darkMode READ darkMode CONSTANT)
public:
    explicit AdwaitaTheme(QObject *parent = nullptr);
    ~AdwaitaTheme() override = default;

    static AdwaitaTheme *qmlAttachedProperties(QObject *object);

    // Button
    int buttonMarginHeight() const { return Adwaita::Button_MarginHeight; }
    int buttonMarginWidth() const { return Adwaita::Button_MarginWidth; }
    int buttonMinimumHeight() const { return Adwaita::Button_MinHeight; }
    int buttonMinimumWidth() const { return Adwaita::Button_MinWidth; }

    // Frame
    int frameRadius() const { return Adwaita::Frame_FrameRadius; }
    int frameWidth() const { return Adwaita::Menu_FrameWidth; }

    // Base colors
    QColor baseColor() const { return m_palette.color(QPalette::Base); }
    QColor buttonColor() const { return m_palette.color(QPalette::Button); }
    QColor buttonOutlineColor() const;
    QColor disabledTextColor() const { return m_palette.color(QPalette::Disabled, QPalette::Text); }
    QColor highlightColor() const { return m_palette.color(QPalette::Highlight); }
    QColor highlightTextColor() const { return m_palette.color(QPalette::HighlightedText); }
    QColor linkColor() const { return m_palette.color(QPalette::Link); }
    QColor textColor() const { return m_palette.color(QPalette::Text); }
    QColor windowColor() const { return m_palette.color(QPalette::Window); }

    // Advanced colors
    Q_INVOKABLE QColor getButtonBottomColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed);
    Q_INVOKABLE QColor getButtonTopColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed);
    Q_INVOKABLE QColor getButtonOutlineColor(bool highlighted, bool destructiveAction, bool hovered, bool pressed);

    bool darkMode() const { return m_darkMode; }
private:
    bool m_darkMode;
    QPalette m_palette;
};

QML_DECLARE_TYPEINFO(AdwaitaTheme, QML_HAS_ATTACHED_PROPERTIES)

#endif // ADWAITA_THEME_H
