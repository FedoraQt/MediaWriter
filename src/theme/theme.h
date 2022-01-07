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

#include <AdwaitaQt6/adwaita.h>
#include <AdwaitaQt6/adwaitacolors.h>

class AdwaitaTheme : public QObject
{
    Q_OBJECT
    // Button
    Q_PROPERTY(int buttonMarginHeight READ buttonMarginHeight CONSTANT)
    Q_PROPERTY(int buttonMarginWidth READ buttonMarginWidth CONSTANT)
    Q_PROPERTY(int buttonMinimumHeight READ buttonMinimumHeight CONSTANT)
    Q_PROPERTY(int buttonMinimumWidth READ buttonMinimumWidth CONSTANT)
    Q_PROPERTY(int buttonItemSpacing READ buttonItemSpacing CONSTANT)

    // Checkbox
    Q_PROPERTY(int checkboxItemSpacing READ checkboxItemSpacing CONSTANT)
    Q_PROPERTY(int checkboxFocusMarginWidth READ checkboxFocusMarginWidth CONSTANT)
    Q_PROPERTY(int checkboxSize READ checkboxSize CONSTANT)

    // ComboBox
    Q_PROPERTY(int comboBoxMarginHeight READ comboBoxMarginHeight CONSTANT)
    Q_PROPERTY(int comboBoxMarginWidth READ comboBoxMarginWidth CONSTANT)
    Q_PROPERTY(int comboBoxMinimumHeight READ comboBoxMinimumHeight CONSTANT)
    Q_PROPERTY(int comboBoxMinimumWidth READ comboBoxMinimumWidth CONSTANT)

    // Frame
    Q_PROPERTY(int frameRadius READ frameRadius CONSTANT)
    Q_PROPERTY(int frameWidth READ frameWidth CONSTANT)

    // LineEdit
    Q_PROPERTY(int lineEditFrameWidth READ lineEditFrameWidth CONSTANT)
    Q_PROPERTY(int lineEditMarginHeight READ lineEditMarginHeight CONSTANT)
    Q_PROPERTY(int lineEditMarginWidth READ lineEditMarginWidth CONSTANT)
    Q_PROPERTY(int lineEditMinimumHeight READ lineEditMinimumHeight CONSTANT)
    Q_PROPERTY(int lineEditMinimumWidth READ lineEditMinimumWidth CONSTANT)

    // MenuItem
    Q_PROPERTY(int menuItemSpacing READ menuItemSpacing CONSTANT)
    Q_PROPERTY(int menuItemMarginWidth READ menuItemMarginWidth CONSTANT)

    // Progressbar
    Q_PROPERTY(int progressBarBusyIndicatorSize READ progressBarBusyIndicatorSize CONSTANT)
    Q_PROPERTY(int progressBarThickness READ progressBarThickness CONSTANT)

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
    int buttonItemSpacing() const { return Adwaita::Button_ItemSpacing; }

    // Checkbox
    int checkboxItemSpacing() const { return Adwaita::CheckBox_ItemSpacing; }
    int checkboxFocusMarginWidth() const { return Adwaita::CheckBox_FocusMarginWidth; }
    int checkboxSize() const { return Adwaita::CheckBox_Size; }

    // ComboBox
    int comboBoxMarginHeight() const { return Adwaita::ComboBox_MarginHeight; }
    int comboBoxMarginWidth() const { return Adwaita::ComboBox_MarginWidth; }
    int comboBoxMinimumHeight() const { return Adwaita::ComboBox_MinHeight; }
    int comboBoxMinimumWidth() const { return Adwaita::ComboBox_MinWidth; }

    // Frame
    int frameRadius() const { return Adwaita::Frame_FrameRadius; }
    int frameWidth() const { return Adwaita::Menu_FrameWidth; }

    // LineEdit
    int lineEditFrameWidth() const { return Adwaita::LineEdit_FrameWidth; }
    int lineEditMarginHeight() const { return Adwaita::LineEdit_MarginHeight; }
    int lineEditMarginWidth() const { return Adwaita::LineEdit_MarginWidth; }
    int lineEditMinimumHeight() const { return Adwaita::LineEdit_MinHeight; }
    int lineEditMinimumWidth() const { return Adwaita::LineEdit_MinWidth; }

    // MenuItem
    int menuItemSpacing() const { return Adwaita::MenuItem_ItemSpacing; }
    int menuItemMarginWidth() const { return Adwaita::MenuItem_MarginWidth; }

    // Progressbar
    int progressBarBusyIndicatorSize() const { return Adwaita::ProgressBar_BusyIndicatorSize; }
    int progressBarThickness() const { return Adwaita::ProgressBar_Thickness; }

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

    Q_INVOKABLE QColor getCheckBoxBottomColor(bool hovered, bool pressed, bool checked);
    Q_INVOKABLE QColor getCheckBoxTopColor(bool hovered, bool pressed, bool checked);
    Q_INVOKABLE QColor getCheckBoxOutlineColor(bool hovered, bool pressed, bool checked);

    Q_INVOKABLE QColor getProgressBarColor();
    Q_INVOKABLE QColor getProgressBarOutlineColor();

    Q_INVOKABLE QColor getScrollBarGrooveColor();
    Q_INVOKABLE QColor getScrollBarHandleColor(bool hovered, bool pressed);

    bool darkMode() const { return m_darkMode; }
private:
    bool m_darkMode;
    QPalette m_palette;
};

QML_DECLARE_TYPEINFO(AdwaitaTheme, QML_HAS_ATTACHED_PROPERTIES)

#endif // ADWAITA_THEME_H
