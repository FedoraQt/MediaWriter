/*
 * Fedora Media Writer
 * Copyright (C) 2024 Jan Grulich <jgrulich@redhat.com>
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

#ifndef MNEMONICFILTER_H
#define MNEMONICFILTER_H

#include <QObject>

class MnemonicFilter : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool altPressed READ altPressed NOTIFY altPressedChanged)

public:
    explicit MnemonicFilter(QObject *parent = nullptr);

    bool altPressed() const;

signals:
    void altPressedChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    bool m_altPressed = false;
};

#endif // MNEMONICFILTER_H
