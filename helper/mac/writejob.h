#ifndef WRITEJOB_H
#define WRITEJOB_H

#include <QObject>
#include <QTextStream>
#include <QProcess>

class WriteJob : public QObject
{
    Q_OBJECT
public:
    explicit WriteJob(const QString &what, const QString &where);
private:
    QString what;
    QString where;

    QProcess *dd;
};

#endif // WRITEJOB_H
