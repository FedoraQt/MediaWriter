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

class AdwaitaTheme : public QObject
{
    Q_OBJECT
    // Button
    Q_PROPERTY(int buttonMarginHeight READ buttonMarginHeight CONSTANT)
    Q_PROPERTY(int buttonMarginWidth READ buttonMarginWidth CONSTANT)
    Q_PROPERTY(int buttonMinimumWidth READ buttonMinimumWidth CONSTANT)
    Q_PROPERTY(int buttonMinimumWidth READ buttonMinimumWidth CONSTANT)

    // Frame
    Q_PROPERTY(int frameRadius READ frameRadius CONSTANT)
    Q_PROPERTY(int frameWidth READ frameWidth CONSTANT)

    // Colors
    Q_PROPERTY(QColor buttonColor READ buttonColor CONSTANT)
    Q_PROPERTY(QColor buttonOutlineColor READ buttonOutlineColor CONSTANT)
    Q_PROPERTY(QColor highlightColor READ highlightColor CONSTANT)
    Q_PROPERTY(QColor highlightTextColor READ highlightTextColor CONSTANT)
    Q_PROPERTY(QColor disabledButtonColor READ disabledButtonColor CONSTANT)
    Q_PROPERTY(QColor linkColor READ linkColor CONSTANT)
    Q_PROPERTY(QColor disabledTextColor READ disabledTextColor CONSTANT)
    Q_PROPERTY(QColor textColor READ textColor CONSTANT)
    Q_PROPERTY(QColor windowColor READ windowColor CONSTANT)

    Q_PROPERTY(QColor darkMode READ darkMode CONSTANT)
public:
    explicit AdwaitaTheme(QObject *parent = nullptr);
    ~AdwaitaTheme() override = default;

    static AdwaitaTheme *qmlAttachedProperties(QObject *object);

    // Borrowed from the KColorUtils code
    Q_INVOKABLE static QColor mix(const QColor &c1, const QColor &c2, qreal bias = 0.5)
    {
        auto mixQreal = [](qreal a, qreal b, qreal bias) {
            return a + (b - a) * bias;
        };

        if (bias <= 0.0) {
            return c1;
        }
        if (bias >= 1.0) {
            return c2;
        }
        if (std::isnan(bias)) {
            return c1;
        }

        qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
        qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
        qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
        qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

        return QColor::fromRgbF(r, g, b, a);
    }

    Q_INVOKABLE static QColor lighten(const QColor &color, qreal amount = 0.1)
    {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal lightness = l + amount;
        if (lightness > 1) {
            lightness = 1;
        }
        return QColor::fromHslF(h, s, lightness, a);
    }

    Q_INVOKABLE static QColor darken(const QColor &color, qreal amount = 0.1)
    {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal lightness = l - amount;
        if (lightness < 0) {
            lightness = 0;
        }

        return QColor::fromHslF(h, s, lightness, a);
    }

    Q_INVOKABLE static QColor desaturate(const QColor &color, qreal amount = 0.1)
    {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal saturation = s - amount;
        if (saturation < 0) {
            saturation = 0;
        }
        return QColor::fromHslF(h, saturation, l, a);
    }

    Q_INVOKABLE static QColor transparentize(const QColor &color, qreal amount = 0.1)
    {
        qreal h, s, l, a;
        color.getHslF(&h, &s, &l, &a);

        qreal alpha = a - amount;
        if (alpha < 0)
            alpha = 0;
        return QColor::fromHslF(h, s, l, alpha);
    }

    // TODO: keep synced with Adwaita-qt or use Adwaita-qt library

    // Button
    int buttonMarginHeight() const { return 4; }
    int buttonMarginWidth() const { return 8; }
    int buttonMinimumHeight() const { return 36; }
    int buttonMinimumWidth() const { return 80; }

    // Frame
    int frameRadius() const { return 5; }
    int frameWidth() const { return 4; }

    // Colors
    QColor buttonColor() const { return m_palette.color(QPalette::Button); }
    QColor buttonOutlineColor() const;
    QColor highlightColor() const { return m_palette.color(QPalette::Highlight); }
    QColor highlightTextColor() const { return m_palette.color(QPalette::HighlightedText); }
    QColor disabledButtonColor() const { return m_palette.color(QPalette::Disabled, QPalette::Button); }
    QColor linkColor() const { return m_palette.color(QPalette::Link); }
    QColor disabledTextColor() const { return m_palette.color(QPalette::Disabled, QPalette::Text); }
    QColor textColor() const { return m_palette.color(QPalette::Text); }
    QColor windowColor() const { return m_palette.color(QPalette::Window); }

    bool darkMode() const { return m_darkMode; }
private:
    bool m_darkMode;
    QPalette m_palette;
};

QML_DECLARE_TYPEINFO(AdwaitaTheme, QML_HAS_ATTACHED_PROPERTIES)

#endif // ADWAITA_THEME_H
