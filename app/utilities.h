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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QObject>
#include <QFile>

class Progress;
class DownloadReceiver;
class Download;
class DownloadManager;

class Progress : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal from READ from CONSTANT)
    Q_PROPERTY(qreal to READ to NOTIFY toChanged)
    Q_PROPERTY(qreal value READ value NOTIFY valueChanged)
    Q_PROPERTY(qreal ratio READ ratio NOTIFY valueChanged)
public:
    Progress(QObject *parent = nullptr, qreal from = 0.0, qreal to = 1.0);

    qreal from() const;
    qreal to() const;
    qreal value() const;
    qreal ratio() const;

    void setTo(qreal v);
    void setValue(qreal v);
    void setValue(qreal v, qreal to);

signals:
    void valueChanged();
    void toChanged();

public slots:
    void update(qreal value);
    void reset();

private:
    qreal m_from { 0.0 };
    qreal m_to { 1.0 };
    qreal m_value { 0.0 };
};


class DownloadReceiver {
public:
    virtual void onFileDownloaded(const QString &path) { Q_UNUSED(path) }
    virtual void onStringDownloaded(const QString &text) { Q_UNUSED(text) }
    virtual void onDownloadError(const QString &message) { Q_UNUSED(message) }
};

class Download : public QObject {
    Q_OBJECT

public:
    Download(DownloadManager *parent, QNetworkReply *reply, DownloadReceiver *receiver, const QString &filePath, Progress *progress = nullptr);
    DownloadManager *manager();

private slots:
    void onReadyRead();
    void onError(QNetworkReply::NetworkError code);
    void onSslErrors(const QList<QSslError> errors);
    void onFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    qint64 m_initialSize { 0 };
    QNetworkReply *m_reply;
    DownloadReceiver *m_receiver;
    QString m_path;
    Progress *m_progress;

    QFile *m_file;
    QByteArray m_buf;
};

class DownloadManager : public QObject, public DownloadReceiver {
    Q_OBJECT
public:
    static DownloadManager *instance();
    static QString dir();

    void downloadFile(DownloadReceiver *receiver, const QUrl &url, const QString &folder = dir(), Progress *progress = nullptr);
    void fetchPageAsync(DownloadReceiver *receiver, const QString &url);
    QString fetchPage(const QString &url);

    // DownloadReceiver interface
    virtual void onStringDownloaded(const QString &text);
    virtual void onDownloadError();

private:
    DownloadManager();
    static DownloadManager *_self;

    QNetworkAccessManager m_manager;
};

#endif // UTILITIES_H
