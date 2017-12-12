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


#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>
#include <QElapsedTimer>
#include <QThread>

#include "utilities.h"

class Download;
class AccessManager;
class DownloadManager;

class DownloadWorker : public QObject {
    Q_OBJECT
public:
    void injectReply(QNetworkReply *reply);
    qint64 contentLength() const;
    qint64 receivedBytes() const;
    bool isFinished() const;

signals:
    void finished();
    void progress(qint64 current, qint64 total);
    void error(QString reason);

private slots:
    virtual void onReadyRead() = 0;
    virtual void onDownloadProgress(qint64 received, qint64 total);
    void onError(QNetworkReply::NetworkError code);
    void onSslErrors(const QList<QSslError> &errors);

protected:
    DownloadWorker(QNetworkReply *parent = nullptr);

    void checkContentLength(qint64 received, qint64 total);

    QNetworkReply *reply() const;
    qint64 m_contentLength { -1 };
    qint64 m_received { -1 };
};

class DownloadString : public DownloadWorker {
    Q_OBJECT
public:
    QByteArray data();

private slots:
    void onReadyRead() override;

private:
    friend class AccessManager;
    DownloadString(QNetworkReply *parent = nullptr);

    QByteArray m_buffer;
};

class DownloadFile : public DownloadWorker {
    Q_OBJECT
public:
    QString path() const;
    QString hash() const;

private slots:
    void onReadyRead() override;
    void onDownloadProgress(qint64 received, qint64 total) override;
    void init();

private:
    friend class AccessManager;
    DownloadFile(const QString &path = QString(), QNetworkReply *parent = nullptr);
    DownloadFile(const QString &directory, const QString &file);

    QFile m_file { nullptr };
    QByteArray m_buffer;
    QCryptographicHash m_hash;
    qint64 m_originalSize { 0 };
};


class Download : public QObject {
    Q_OBJECT
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
public:
    enum Status {
        CREATED = 0,
        PRECOMPUTING,
        RUNNING,
        FINISHED,
        CANCELLED,
        ERROR
    };
    Q_ENUMS(Status)

    Status status() const;

    QByteArray data() const;
    QString path() const;
    QString hash() const;

    bool isFinished() const;

    qreal secondsRemaining() const;
    qreal bytesPerSecond() const;

public slots:
    void cancel();

private slots:
    void injectWorker(DownloadWorker *worker);
    void onFinished();
    void onError(QString reason);
    void onProgress(qint64 current, qint64 total);

signals:
    void statusChanged();

    void finished();
    void error(QString reason);
    void progress(qreal current, qreal total);
    void timeRemaining(qreal seconds);

private:
    friend class DownloadManager;
    Download(QObject *parent = nullptr, const QString &path = QString(), bool finished = false);

    DownloadWorker *m_worker { nullptr };
    QString m_path;
    Status m_status { CREATED };
    QElapsedTimer m_timer;
};

/**
 * @brief The AccessManager class
 *
 * Not accessible from the outside.
 * Exists in a thread dedicated to downloads.
 * All downloads are running in parallel in this thread to not block UI.
 */
class AccessManager : public QObject {
    Q_OBJECT
public:
    AccessManager(QObject *parent = nullptr);

signals:
    void downloadStarted(int seq, DownloadWorker *w);

public slots:
    void fetch(int seq, const QString &url);
    void download(int seq, const QString &url, const QString &folder);

private:
    QNetworkAccessManager *m_manager { nullptr };
};

/**
 * @brief The DownloadManager class
 *
 * The only directly accessible class.
 * Lives in the main thread - all members all callable directly without any special measures.
 *
 */
class DownloadManager : public QObject {
    Q_OBJECT
public:
    DownloadManager(QObject *parent = nullptr);
    static DownloadManager *instance();

    Download *download(const QString &url, const QString &folder = defaultDirectory());
    Download *fetch(const QString &url);
    QString fetchSync(const QString &url);

    static QString defaultDirectory();
    static QString userAgent();

private slots:
    void onDownloadStarted(int seq, DownloadWorker *worker);

private:
    static DownloadManager *_self;

    AccessManager *m_manager;
    QThread *m_thread;

    int m_sequence { 0 };
    QMap<int, Download*> m_downloads;
};

#endif // DOWNLOADMANAGER_H
