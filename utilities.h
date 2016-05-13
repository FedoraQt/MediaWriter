#ifndef UTILITIES_H
#define UTILITIES_H

#include <QObject>

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

class DownloadManager : public QObject {
    Q_OBJECT
};

class Download : public QObject {
    Q_OBJECT
};

#endif // UTILITIES_H
