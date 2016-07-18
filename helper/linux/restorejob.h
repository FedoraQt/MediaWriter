#ifndef RESTOREJOB_H
#define RESTOREJOB_H

#include <QObject>
#include <QTextStream>

class RestoreJob : public QObject
{
    Q_OBJECT
public:
    explicit RestoreJob(const QString &where);
public slots:
    void work();
private:
    QTextStream out { stdout };
    QTextStream err { stderr };
};

#endif // RESTOREJOB_H
