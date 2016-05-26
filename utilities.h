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
    Q_PROPERTY(qreal to READ to CONSTANT)
    Q_PROPERTY(qreal value READ value NOTIFY valueChanged)
    Q_PROPERTY(qreal ratio READ ratio NOTIFY valueChanged)
public:
    Progress(QObject *parent = nullptr, qreal from = 0.0, qreal to = 1.0);

    qreal from() const;
    qreal to() const;
    qreal value() const;
    qreal ratio() const;

    void setValue(qreal v);
    void setValue(qreal v, qreal to);

signals:
    void valueChanged();

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
    virtual void onDownloadError() {}
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
