/*
 * AOSC Media Writer
 * Copyright (C) 2021 Jan Grulich <jgrulich@redhat.com>
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

#ifndef PORTALPortalFileDialog_H
#define PORTALPortalFileDialog_H

#include <QObject>
#include <QWindow>

class PortalFileDialog : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAvailable READ isAvailable)
public:
    // Copied from QXdgDesktopPortalFileDialog
    enum ConditionType : uint { GlobalPattern = 0, MimeType = 1 };
    // Filters a(sa(us))
    // Example: [('Images', [(0, '*.ico'), (1, 'image/png')]), ('Text', [(0, '*.txt')])]
    struct FilterCondition {
        ConditionType type;
        QString pattern; // E.g. '*ico' or 'image/png'
    };
    typedef QVector<FilterCondition> FilterConditionList;
    struct Filter {
        QString name; // E.g. 'Images' or 'Text
        FilterConditionList filterConditions;
        ; // E.g. [(0, '*.ico'), (1, 'image/png')] or [(0, '*.txt')]
    };
    typedef QVector<Filter> FilterList;

    PortalFileDialog(QObject *parent = nullptr, WId winId = 0);
    virtual ~PortalFileDialog() override;

    bool isAvailable() const;

public Q_SLOTS:
    void open();

private Q_SLOTS:
    void gotResponse(uint response, const QVariantMap &results);

Q_SIGNALS:
    void fileSelected(const QUrl &fileName);

private:
    bool m_available;
    WId m_winId;
};

Q_DECLARE_METATYPE(PortalFileDialog::FilterCondition);
Q_DECLARE_METATYPE(PortalFileDialog::FilterConditionList);
Q_DECLARE_METATYPE(PortalFileDialog::Filter);
Q_DECLARE_METATYPE(PortalFileDialog::FilterList);

#endif // PORTALPortalFileDialog_H
