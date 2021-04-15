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

#include "adwaitathemeplugin.h"
#include "icon.h"
#include "theme.h"

#include <QQmlContext>

void AdwaitaThemePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    QQmlContext *context = engine->rootContext();

    AdwaitaTheme *theme = new AdwaitaTheme(engine);
    context->setContextProperty(QStringLiteral("theme"), theme);
}

void AdwaitaThemePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.fedoraproject.AdwaitaTheme"));

    qmlRegisterUncreatableType<AdwaitaTheme>(uri, 2, 0, "Theme", QStringLiteral("It is not possible to instantiate Theme directly."));
    qmlRegisterType<Icon>(uri, 2, 0, "Icon");

}
