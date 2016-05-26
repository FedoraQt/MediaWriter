#include "utilities.h"

#include <QDebug>


Progress::Progress(QObject *parent, qreal from, qreal to)
    : QObject(parent), m_from(from), m_to(to), m_value(from)
{

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

void DownloadManager::downloadFile(DownloadReceiver *receiver, const QString &url, const QString &folder, Progress *progress) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    //request.setHeader();
    auto reply = m_manager.get(request);
    qWarning() << "Get request for" << url << "created";
    auto download = new Download(this, reply, receiver, folder, progress);
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


    connect(&m_manager, &QNetworkAccessManager::encrypted, [=](QNetworkReply *){ qWarning() << "encrypted!";});
    connect(&m_manager, &QNetworkAccessManager::finished, [=](QNetworkReply *){ qWarning() << "finished!";});
    connect(&m_manager, &QNetworkAccessManager::networkAccessibleChanged, [=](QNetworkAccessManager::NetworkAccessibility){ qWarning() << "networkAccessibleChanged!";});
    connect(&m_manager, &QNetworkAccessManager::sslErrors, [=](QNetworkReply*, QList<QSslError>){ qWarning() << "sslErrors!";});
}


Download::Download(DownloadManager *parent, QNetworkReply *reply, DownloadReceiver *receiver, const QString &folder, Progress *progress)
    : QObject(parent)
    , m_reply(reply)
    , m_receiver(receiver)
    , m_folder(folder)
    , m_progress(progress)
{
    connect(reply, &QNetworkReply::readyRead, this, &Download::onReadyRead);
    connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, &Download::onError);
    connect(reply, &QNetworkReply::sslErrors, this, &Download::onSslErrors);
    connect(reply, &QNetworkReply::finished, this, &Download::onFinished);
    if (m_progress)
        connect(reply, &QNetworkReply::downloadProgress, this, &Download::onDownloadProgress);
    qWarning() << "Download: signals connected";

    if (!folder.isEmpty()) {
        m_file = new QFile(QString("%1/%2").arg(folder).arg(reply->url().fileName()));
        if (m_file->exists())
            m_file->open(QIODevice::Append);
        else
            m_file->open(QIODevice::WriteOnly);
    }
    qWarning() << "Download: file opened";

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
}

void Download::onSslErrors(const QList<QSslError> errors) {
    qWarning() << "Error reading from" << m_reply->url() << ":" << m_reply->errorString();
    for (auto i : errors) {
        qWarning() << "SSL error" << i.errorString();
    }
}

void Download::onFinished() {
    qWarning() << "Download of" << m_reply->url() << "finished with status" << m_reply->errorString();
    qWarning() << "There is" << m_reply->bytesAvailable() << "bytes left";
    m_progress->setValue(1.0);
    if (m_file)
        m_receiver->onFileDownloaded(m_file->fileName());
    else
        m_receiver->onStringDownloaded(m_buf);
}

void Download::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    m_progress->setValue(bytesReceived);
}
