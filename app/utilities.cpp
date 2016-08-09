/*
 * Fedora Media Writer
 * Copyright (C) 2016 Martin Bříza <mbriza@redhat.com>
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

#include "utilities.h"

#include <QDebug>
#include <QStandardPaths>


Progress::Progress(QObject *parent, qreal from, qreal to)
    : QObject(parent), m_from(from), m_to(to), m_value(from) {
    connect(this, &Progress::toChanged, this, &Progress::valueChanged);
}

qreal Progress::from() const {
    return m_from;
}

qreal Progress::to() const {
    return m_to;
}

qreal Progress::value() const {
    return m_value;
}

qreal Progress::ratio() const {
    return (value() - from()) / (to() - from());
}

void Progress::setTo(qreal v) {
    if (m_to != v) {
        m_to = v;
        emit toChanged();
    }
}

void Progress::setValue(qreal v) {
    if (m_value != v) {
        m_value = v;
        emit valueChanged();
    }
}

void Progress::setValue(qreal v, qreal to) {
    qreal computedValue = v / to * (m_to - m_from) + m_from;
    if (computedValue != m_value) {
        m_value = computedValue;
        emit valueChanged();
    }
}

void Progress::update(qreal value) {
    if (m_value != value) {
        m_value = value;
        emit valueChanged();
    }
}

void Progress::reset() {
    update(from());
}


DownloadManager *DownloadManager::_self = nullptr;

DownloadManager *DownloadManager::instance() {
    if (!_self)
        _self = new DownloadManager();
    return _self;
}

QString DownloadManager::dir() {
    QString d = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);

    return d;
}

void DownloadManager::downloadFile(DownloadReceiver *receiver, const QUrl &url, const QString &folder, Progress *progress) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    QString bareFileName = QString("%1/%2").arg(folder).arg(url.fileName());
    QString partFileName = bareFileName + ".part";

    if (QFile::exists(bareFileName)) {
        receiver->onFileDownloaded(bareFileName);
        return;
    }
    else if (QFile::exists(partFileName)) {
        QFile partFile(partFileName);
        request.setRawHeader("Range", QString("bytes=%1-").arg(partFile.size()).toLocal8Bit());
    }

    auto reply = m_manager.get(request);
    auto download = new Download(this, reply, receiver, bareFileName, progress);
}

void DownloadManager::fetchPageAsync(DownloadReceiver *receiver, const QString &url) {

}

QString DownloadManager::fetchPage(const QString &url) {

}

void DownloadManager::onStringDownloaded(const QString &text) {

}

void DownloadManager::onDownloadError() {

}

DownloadManager::DownloadManager() {
    /*
    connect(networkReply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
        [=](QNetworkReply::NetworkError code){ }); */


    /*
    connect(&m_manager, &QNetworkAccessManager::encrypted, [=](QNetworkReply *){ qWarning() << "encrypted!";});
    connect(&m_manager, &QNetworkAccessManager::finished, [=](QNetworkReply *){ qWarning() << "finished!";});
    connect(&m_manager, &QNetworkAccessManager::networkAccessibleChanged, [=](QNetworkAccessManager::NetworkAccessibility){ qWarning() << "networkAccessibleChanged!";});
    connect(&m_manager, &QNetworkAccessManager::sslErrors, [=](QNetworkReply*, QList<QSslError>){ qWarning() << "sslErrors!";});
    */
}


Download::Download(DownloadManager *parent, QNetworkReply *reply, DownloadReceiver *receiver, const QString &filePath, Progress *progress)
    : QObject(parent)
    , m_reply(reply)
    , m_receiver(receiver)
    , m_path(filePath)
    , m_progress(progress)
{
    connect(reply, &QNetworkReply::readyRead, this, &Download::onReadyRead);
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Download::onError);
    connect(reply, &QNetworkReply::sslErrors, this, &Download::onSslErrors);
    connect(reply, &QNetworkReply::finished, this, &Download::onFinished);

    if (m_progress)
        connect(reply, &QNetworkReply::downloadProgress, this, &Download::onDownloadProgress);

    if (!m_path.isEmpty()) {
        m_file = new QFile(m_path + ".part");

        if (m_file->exists()) {
            m_initialSize = m_file->size();
            m_file->open(QIODevice::Append);
        }
        else {
            m_file->open(QIODevice::WriteOnly);
        }
    }

    if (reply->bytesAvailable() > 0)
        onReadyRead();
}

DownloadManager *Download::manager() {
    return qobject_cast<DownloadManager*>(parent());
}

void Download::onReadyRead() {
    while (m_reply->bytesAvailable()) {
        QByteArray buf = m_reply->read(1024*1024);
        if (m_file) {
            m_file->write(buf);
        }
        else {
            m_buf.append(buf);
        }
    }
}

void Download::onError(QNetworkReply::NetworkError code) {
    qWarning() << "Error" << code << "reading from" << m_reply->url() << ":" << m_reply->errorString();
    m_receiver->onDownloadError(m_reply->errorString());
}

void Download::onSslErrors(const QList<QSslError> errors) {
    qWarning() << "Error reading from" << m_reply->url() << ":" << m_reply->errorString();
    for (auto i : errors) {
        qWarning() << "SSL error" << i.errorString();
    }
    m_receiver->onDownloadError(m_reply->errorString());
}

void Download::onFinished() {
    if (m_reply->error() != 0) {
        if (m_file->size() == 0) {
            m_file->remove();
        }
    }
    else if (m_file) {
        m_file->rename(m_path);  // move the .part file to the final path
        m_receiver->onFileDownloaded(m_file->fileName());
    }
    else {
        m_receiver->onStringDownloaded(m_buf);
    }
}

void Download::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    Q_UNUSED(bytesTotal);
    m_progress->setValue(m_initialSize + bytesReceived);
}
