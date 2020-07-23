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

#ifndef ADWAITA_UNITS_H
#define ADWAITA_UNITS_H

#include <QObject>
#include <QtQml>

class AdwaitaUnits : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int gridUnit READ gridUnit NOTIFY gridUnitChanged)
    Q_PROPERTY(int smallSpacing READ smallSpacing NOTIFY spacingChanged)
    Q_PROPERTY(int largeSpacing READ largeSpacing NOTIFY spacingChanged)
public:
    explicit AdwaitaUnits(QObject *parent = nullptr);
    ~AdwaitaUnits() override = default;

    static AdwaitaUnits *qmlAttachedProperties(QObject *object);

    int gridUnit() const;
    int smallSpacing() const;
    int largeSpacing() const;
Q_SIGNALS:
    void gridUnitChanged();
    void spacingChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void update();

    int m_gridUnit;
    int m_smallSpacing;
    int m_largeSpacing;
};

QML_DECLARE_TYPEINFO(AdwaitaUnits, QML_HAS_ATTACHED_PROPERTIES)

#endif // ADWAITA_UNITS_H

