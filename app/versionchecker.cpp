/*
 * Fedora Media Writer
 * Copyright (C) 2017 Martin Bříza <mbriza@redhat.com>
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

#include "versionchecker.h"

#include <QJsonDocument>
#include <QJsonObject>

VersionChecker::VersionChecker(QObject *parent)
    : QObject(parent) {
#if defined(__APPLE__) || defined(_WIN32)
    qDebug() << this->metaObject()->className() << "Asking for new FMW version information";
    DownloadManager::instance()->fetchPageAsync(this, "https://getfedora.org/static/fmw-version.json");
#else
    qDebug() << this->metaObject()->className() << "This platform doesn't need to ask about new FMW versions";
#endif
}
/*
void VersionChecker::onStringDownloaded(const QString &text) {
    auto doc = QJsonDocument::fromJson(text.toUtf8());
    QJsonObject obj = doc.object();
#if defined(__APPLE__)
    const char *platform = "osx";
#elif defined(_WIN32)
    const char *platform = "win32";
#else
    const char *platform = nullptr;
#endif
    qDebug() << this->metaObject()->className() <<"Got new FMW version information";
    if (platform) {
        QJsonValueRef versionObject = obj[platform].toObject()["version"];
        if (!versionObject.isNull() && !versionObject.isUndefined()) {
            QString currentVersion(MEDIAWRITER_VERSION);
            QString newVersion = versionObject.toString();
            QString urlStr = obj[platform].toObject()["url"].toString();
            QUrl url(urlStr);
            if (isVersionHigher(currentVersion, newVersion) && url.isValid()) {
                qDebug() << this->metaObject()->className() << "New FMW version is" << newVersion << "- we're running on" << currentVersion << "which is older.";
                m_newerVersion = newVersion;
                m_url = url;
                emit newerVersionChanged();
            }
            else {
                qDebug() << this->metaObject()->className() << "New FMW version is" << newVersion << "- we're running on" << currentVersion << "which is newer or the same.";
            }
        }
        else {
            qWarning() << this->metaObject()->className() << "New FMW version information was empty for this platform";
        }
    }
    else {
        qWarning() << this->metaObject()->className() << "Got an answer to query about new versions despite the fact this platform shouldn't support user updates.";
    }
}

void VersionChecker::onDownloadError(const QString &message) {
    qWarning() << this->metaObject()->className() << "It was impossible to fetch info about a new FMW version:" << message;
}
*/

QString VersionChecker::newerVersion() const {
    return m_newerVersion;
}

QUrl VersionChecker::url() const {
    return m_url;
}

bool VersionChecker::isVersionHigher(QString currentVersion, QString newVersion) {
    QStringList currentSplit = currentVersion.split(".");
    QStringList newSplit = newVersion.split(".");
    for (int i = 0; i < newSplit.count(); i++) {
        if (currentSplit.count() <= i)
            return true;
        if (currentSplit[i] < newSplit[i])
            return true;
        if (currentSplit[i] > newSplit[i])
            return false;
    }
    return false;
}
