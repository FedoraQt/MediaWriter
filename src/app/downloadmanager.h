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

#include "utilities.h"

class DownloadReceiver;
class Download;
class DownloadManager;

/*
 * How this (Downloader stuff) works:
 * not very well
 *
 * All of this should be rewritten at some point (and I admit it's possible it never will be - for
 *  reference, today is Nov 9 2016)
 *
 * There are three classes:
 * Download, DownloadManager and DownloadReceiver
 *
 * DownloadReceiver is just an abstract class you should inherit in a class that you want to receive
 *  notifications about finished downloads.
 * I should have probably done that using a signal. Nothing wrong with this so far though.
 *
 * DownloadManager is the entry point where you request a download and provide your
 *  DownloadReceiver implementation that will be notified about it having finished at some point in the future.
 * You're able to either request a string containing whatever it found at the URL
 *  or you can get a path to a file where all the data from the link will be stored.
 * Getting strings is pretty straightforward while downloading files is not easy at all.
 *
 * When downloading a file, the Manager asks the Fedora mirror service for a list of mirrors first.
 * At the same time, it creates a Download object (there can be only one at a time) and waits.
 * After getting the mirrosr, it inserts a new QNetworkReply inside the Download which starts
 *  crunching the incoming data. That means it stores it to a .part file while also computing a SHA256 hash.
 * If the mirror stops supplying data or the network breaks, it tries accessing another repo from the list.
 * If there is no list provided by the server, it tries to just download from the metadata URL directly.
 *
 * This all would work well if we didn't have to support download resuming.
 * If you have to resume a download, the newly-created Download object starts to compute the SHA256 hash
 *  while the Manager waits for the mirror list.
 * When the mirror list is delivered, a new QNetworkReply is created and inserted into the Download if
 *  (and only if) it's done computing the hash of the part already on the drive (hasCatchedUp() method).
 * Otherwise, it lets the Download finish with its business of computing the hash. It will ask for a new mirror
 *  when it's done. From this point, everything continues exactly as if it wasn't continuing.
 * For this reason I think there's a slight (really very small) possibility of getting stuck, forcing the user
 *  to reset the process.
 *
 */

/**
 * @brief The DownloadReceiver class
 *
 * A virtual class for getting the results of a download
 */
class DownloadReceiver {
public:
    virtual void onFileDownloaded(const QString &path, const QString &shaHash) { Q_UNUSED(path); Q_UNUSED(shaHash) }
    virtual void onStringDownloaded(const QString &text) { Q_UNUSED(text) }
    virtual void onDownloadError(const QString &message) = 0;
};


/**
 * @brief The Download class
 *
 * The download handler itself
 *
 * It processes the incoming data, computes its checksum and tracks the amount of data downloaded/required
 */
class Download : public QObject {
    Q_OBJECT

public:
    Download(DownloadManager *parent, DownloadReceiver *receiver, const QString &filePath, Progress *progress = nullptr);
    DownloadManager *manager();

    void handleNewReply(QNetworkReply *reply);
    qint64 bytesDownloaded();

    bool hasCatchedUp();

private slots:
    void start();
    void catchUp();

    void onReadyRead();
    void onError(QNetworkReply::NetworkError code);
    void onSslErrors(const QList<QSslError> errors);
    void onFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onTimedOut();

private:
    qint64 m_previousSize { 0 };
    qint64 m_bytesDownloaded { 0 };
    QNetworkReply *m_reply { nullptr };
    DownloadReceiver *m_receiver { nullptr };
    QString m_path { };
    Progress *m_progress { nullptr };
    QTimer m_timer { };
    bool m_catchingUp { false };

    QFile *m_file { nullptr };
    QByteArray m_buf { };
    QCryptographicHash m_hash { QCryptographicHash::Sha256 };
};

/**
 * @brief The DownloadManager class
 *
 * The class to use as an entry point to downloading some data
 *
 * You can either get a string of the data cointained at an URL or save the data as a file on your hard drive.
 *
 * For files hosted on Fedora servers, it also tries to get a list of mirrors to download the file from.
 */
class DownloadManager : public QObject, public DownloadReceiver {
    Q_OBJECT
public:
    static DownloadManager *instance();
    static QString dir();
    static QString userAgent();

    QString downloadFile(DownloadReceiver *receiver, const QUrl &url, const QString &folder = dir(), Progress *progress = nullptr);
    void fetchPageAsync(DownloadReceiver *receiver, const QString &url);
    QString fetchPage(const QString &url);

    QNetworkReply *tryAnotherMirror();

    Q_INVOKABLE void cancel();

    // DownloadReceiver interface
    virtual void onStringDownloaded(const QString &text) override;
    virtual void onDownloadError(const QString &message) override;

private:
    DownloadManager();
    static DownloadManager *_self;

    Download *m_current { nullptr };
    QStringList m_mirrorCache { };

    QNetworkAccessManager m_manager;
};

#endif // DOWNLOADMANAGER_H
