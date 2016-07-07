#ifndef RESTOREJOB_H
#define RESTOREJOB_H

#include <QObject>

class RestoreJob : public QObject
{
    Q_OBJECT
public:
    explicit RestoreJob(const QString &where);

signals:

public slots:
};

#endif // RESTOREJOB_H
