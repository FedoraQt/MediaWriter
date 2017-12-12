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

#include "downloadmanager.h"

#include <QApplication>
#include <QAbstractEventDispatcher>
#include <QNetworkProxyFactory>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QSysInfo>
#include <QDir>
#include <QStringBuilder>

////////////////////////////////////////////////////////////////////////////////
DownloadManager *DownloadManager::_self = nullptr;
DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , m_manager(new AccessManager())
    , m_thread(new QThread(this))
{
    qCritical() << "DownloadManager:" << this->thread();
    m_manager->moveToThread(m_thread);
    m_thread->start();

    connect(m_manager, &AccessManager::downloadStarted, this, &DownloadManager::onDownloadStarted);
    //qCritical() << "Fetched:" << fetchSync("qrc:///releases.json");
}

DownloadManager *DownloadManager::instance() {
    if (!_self)
        _self = new DownloadManager();
    return _self;
}

QString DownloadManager::defaultDirectory() {
    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
}

QString DownloadManager::userAgent() {
    QString ret = QString("FedoraMediaWriter/%1 (").arg(MEDIAWRITER_VERSION);
#if QT_VERSION >= 0x050400
    ret.append(QString("%1").arg(QSysInfo::prettyProductName().replace(QRegExp("[()]"), "")));
    ret.append(QString("; %1").arg(QSysInfo::buildAbi()));
#else
    // TODO probably should follow the format of prettyProductName, however this will be a problem just on Debian it seems
# ifdef __linux__
    ret.append("linux");
# endif // __linux__
# ifdef __APPLE__
    ret.append("mac");
# endif // __APPLE__
# ifdef _WIN32
    ret.append("windows");
# endif // _WIN32
#endif
    ret.append(QString("; %1").arg(QLocale(QLocale().language()).name()));
#ifdef MEDIAWRITER_PLATFORM_DETAILS
    ret.append(QString("; %1").arg(MEDIAWRITER_PLATFORM_DETAILS));
#endif
    ret.append(")");

    return ret;
}

void DownloadManager::onDownloadStarted(int seq, DownloadWorker *worker) {
    m_downloads[seq]->injectWorker(worker);
    m_downloads.remove(seq);
}

Download *DownloadManager::download(const QString &url, const QString &folder) {
    QString file = QUrl(url).fileName();
    QString fullPath = folder % "/" % file;
    QString tempPath = fullPath % ".part";

    Download *d;
    if (QFile(fullPath).exists()) {
        d = new Download(this, fullPath, true);
    }
    else {
        d = new Download(this, tempPath);
    }

    m_downloads[m_sequence] = d;
    connect(d, &QObject::destroyed, [=](){
        m_downloads.remove(m_sequence);
    });

    if (!d->isFinished())
        QMetaObject::invokeMethod(m_manager, "download", Qt::QueuedConnection, Q_ARG(int, m_sequence), Q_ARG(QString, url), Q_ARG(QString, tempPath));

    m_sequence++;
    return d;
}

Download *DownloadManager::fetch(const QString &url) {
    Download *d = new Download(this);
    m_downloads[m_sequence] = d;
    connect(d, &QObject::destroyed, [=](){
        m_downloads.remove(m_sequence);
    });
    QMetaObject::invokeMethod(m_manager, "fetch", Qt::QueuedConnection, Q_ARG(int, m_sequence), Q_ARG(QString, url));
    m_sequence++;

    return d;
}

QString DownloadManager::fetchSync(const QString &url) {
    QString ret;
    QEventLoop loop;

    Download *d = fetch(url);
    if (!d)
        return ret;

    loop.connect(d, &Download::finished, [&]() {
        ret = d->data();
        loop.quit();
    });
    loop.connect(d, &Download::error, [&](QString reason) {
        qWarning() << "Fetching" << url << "failed:" << reason;
        loop.quit();
    });

    while (true) {
        if (d->isFinished()) {
            ret = d->data();
            break;
        }
        loop.processEvents(QEventLoop::AllEvents, 1000);
    }

    d->deleteLater();
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
AccessManager::AccessManager(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    m_manager->setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);
}

void AccessManager::download(int seq, const QString &url, const QString &fileName) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, DownloadManager::userAgent());

    if (QFile::exists(fileName)) {
        qCritical() << "Temp file exists";
        QFileInfo fi(fileName);
        request.setRawHeader("Range", QString("bytes=%1-").arg(fi.size()).toLatin1());
    }
    QNetworkReply *reply = m_manager->get(request);
    // have only a 4MB buffer in case the user is on a very fast network
    reply->setReadBufferSize(4*1024*1024);
    connect(reply, &QNetworkReply::redirected, [=](const QUrl& newUrl) {
        qCritical() << "Redirected to" << newUrl;
        emit reply->redirectAllowed();
    });

    emit downloadStarted(seq, new DownloadFile(fileName, reply));
}

void AccessManager::fetch(int seq, const QString &url) {
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, DownloadManager::userAgent());

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::redirected, [&](const QUrl& newUrl) {
        qCritical() << "Redirected to" << newUrl;
        emit reply->redirectAllowed();
    });

    emit downloadStarted(seq, new DownloadString(reply));
}

////////////////////////////////////////////////////////////////////////////////
DownloadWorker::DownloadWorker(QNetworkReply *reply)
    : QObject(nullptr)
{
    injectReply(reply);
}

void DownloadWorker::checkContentLength(qint64 received, qint64 total) {
    // total is sometimes (or usually with the mirrors I use) -1 and Content-Length contains a comma
    // do our own parsing of the header if that's the case
    // in the worst case, we'll be stuck with total being 0 which is okay (no download speed reporting)
    if (m_contentLength < 0 && total > 0)
        m_contentLength = total;
    if (m_contentLength < 0) {
        QString headerLength = reply()->rawHeader("Content-Length");
        if (headerLength.contains(','))
            m_contentLength = headerLength.split(',')[1].toLongLong();
        else
            m_contentLength = headerLength.toLongLong();
    }

    if (total < 0)
        total = m_contentLength;

    m_received = received;
}

QNetworkReply *DownloadWorker::reply() const {
    return qobject_cast<QNetworkReply*>(parent());
}

void DownloadWorker::injectReply(QNetworkReply *reply) {
    QObject *previous = parent();
    setParent(reply);
    if (previous || reply == previous)
        reply->deleteLater();
    if (!reply)
        return;

    connect(this, &DownloadWorker::destroyed, reply, &QNetworkReply::deleteLater);

    connect(reply, &QNetworkReply::readyRead, this, &DownloadWorker::onReadyRead);
    connect(reply, &QNetworkReply::downloadProgress, this, &DownloadWorker::onDownloadProgress);
    connect(reply, &QNetworkReply::finished, this, &DownloadWorker::finished);
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &DownloadWorker::onError);
    connect(reply, &QNetworkReply::sslErrors, this, &DownloadWorker::onSslErrors);
}

qint64 DownloadWorker::contentLength() const {
    return m_contentLength;
}

qint64 DownloadWorker::receivedBytes() const {
    return m_received;
}

bool DownloadWorker::isFinished() const {
    if (reply())
        return reply()->isFinished();
    return false;
}

void DownloadWorker::onDownloadProgress(qint64 received, qint64 total) {
    checkContentLength(received, total);

    emit progress(received, total);
}

void DownloadWorker::onError(QNetworkReply::NetworkError code) {
    qDebug() << "Download error:" << code;
}

void DownloadWorker::onSslErrors(const QList<QSslError> &errors) {
    qDebug() << "Download SSL errors:" << errors;
}

////////////////////////////////////////////////////////////////////////////////
DownloadString::DownloadString(QNetworkReply *reply)
    : DownloadWorker(reply)
{

}

QByteArray DownloadString::data() {
    return m_buffer;
}

void DownloadString::onReadyRead() {
    m_buffer.append(reply()->readAll());
}

////////////////////////////////////////////////////////////////////////////////
DownloadFile::DownloadFile(const QString &path, QNetworkReply *reply)
    : DownloadWorker(reply)
    , m_file(path)
    , m_hash(QCryptographicHash::Sha256)
{
    QTimer::singleShot(0, this, &DownloadFile::init);
}

QString DownloadFile::path() const {
    return m_file.fileName();
}

QString DownloadFile::hash() const {
    return m_hash.result().toHex();
}

void DownloadFile::onReadyRead() {
    while (m_file.isWritable() && reply()->bytesAvailable()) {
        qint64 wasAvailable = reply()->bytesAvailable();
        QByteArray chunk = reply()->read(4096);
        if ((wasAvailable < 4096 && chunk.size() != wasAvailable) && chunk.size() != 4096) {
            emit error(QString("CHYBA! %1 : %2").arg(chunk.size()).arg(wasAvailable));
        }
        m_hash.addData(chunk);
        qint64 written = m_file.write(chunk);
        if (written != chunk.size()) {
            emit error("CHYBA!!");
        }
    }
}

void DownloadFile::onDownloadProgress(qint64 received, qint64 total) {
    checkContentLength(received, total);
    emit progress(m_originalSize + received, m_contentLength + m_originalSize);
}

void DownloadFile::init() {
    qCritical() << "Constructed" << reply()->url() << m_file.fileName();
    if (m_file.exists()) {
        m_file.open(QIODevice::ReadOnly);
        m_originalSize = m_file.size();
        qint64 total = 0;
        while (!m_file.atEnd()) {
            QByteArray buffer = m_file.read(4*1024*1024);
            m_hash.addData(buffer);
            total += buffer.size();
            emit progress(-total, m_file.size());
        }
        m_file.close();
    }
    qCritical() << "Hash precomputation finished";
    m_file.open(QIODevice::WriteOnly | QIODevice::Append);
}

////////////////////////////////////////////////////////////////////////////////
Download::Download(QObject *parent, const QString &path, bool finished)
    : QObject(parent)
    , m_path(path)
{
    if (finished) {
        m_status = FINISHED;
    }
}

void Download::cancel() {
    if (m_status == RUNNING) {
        // TODO something
    }
    m_status = CANCELLED;
    emit statusChanged();
}

void Download::injectWorker(DownloadWorker *worker) {
    if (!worker)
        return;

    m_worker = worker;
    if (m_worker->isFinished())
        m_status = FINISHED;
    else
        m_status = RUNNING;
    m_timer.start();
    emit statusChanged();

    connect(this, &Download::destroyed, m_worker, &DownloadWorker::deleteLater);

    connect(m_worker, &DownloadWorker::error, this, &Download::error);
    connect(m_worker, &DownloadWorker::error, this, &Download::onError);
    connect(m_worker, &DownloadWorker::finished, this, &Download::onFinished);
    connect(m_worker, &DownloadWorker::finished, this, &Download::finished);
    connect(m_worker, &DownloadWorker::progress, this, &Download::onProgress);
}

void Download::onFinished() {
    m_status = FINISHED;
    emit statusChanged();
}

void Download::onError(QString reason) {
    Q_UNUSED(reason);
    m_status = ERROR;
    emit statusChanged();
}

void Download::onProgress(qint64 current, qint64 total) {
    if (current < -1 && m_status != Download::PRECOMPUTING) {
        m_status = Download::PRECOMPUTING;
        emit statusChanged();
    }
    if (current >= 0 && m_status != Download::RUNNING) {
        m_status = Download::RUNNING;
        emit statusChanged();
    }
    emit progress(abs(current), total);
    qCritical() << bytesPerSecond() / 1024.0 << "KB/s";
    qCritical() << secondsRemaining() << "s remaining";
}

Download::Status Download::status() const {
    return m_status;
}

QByteArray Download::data() const {
    if (isFinished()) {
        auto w = qobject_cast<DownloadString*>(m_worker);
        if (w)
            return w->data();
    }
    return QByteArray();
}

QString Download::path() const {
    if (isFinished()) {
        auto w = qobject_cast<DownloadFile*>(m_worker);
        if (w)
            return w->path();
    }
    return QString();
}

QString Download::hash() const {
    if (isFinished()) {
        auto w = qobject_cast<DownloadFile*>(m_worker);
        if (w)
            return w->hash();
    }
    return QString();
}

bool Download::isFinished() const {
    qCritical() << "Status:" << m_status;
    return m_status == FINISHED;
}

qreal Download::secondsRemaining() const {
    if (!m_worker)
        return 0.0/0.0;
    qreal total = m_worker->contentLength();
    qreal current = m_worker->receivedBytes();
    qreal s = m_timer.elapsed() / 1000.0;
    return (total - current) / (current / s);
}

qreal Download::bytesPerSecond() const {
    if (!m_worker)
        return 0.0/0.0;
    qreal current = m_worker->receivedBytes();
    qreal s = m_timer.elapsed() / 1000.0;
    return current / s;
}
