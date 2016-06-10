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

    const int BUFFER_SIZE { 1024 * 16 };
public slots:
    void work();
private:
    QString what;
    QString where;
    QTextStream out { stdout };
    QTextStream err { stderr };

    QProcess *dd { nullptr };
};

#endif // WRITEJOB_H
