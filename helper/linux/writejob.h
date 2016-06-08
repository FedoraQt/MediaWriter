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

public slots:
    void work();
    void onReadyRead();
    void onFinished(int exitCode, QProcess::ExitStatus status);
private:
    QString what;
    QString where;
    QTextStream out { stdout };
    QTextStream err { stderr };

    QProcess *dd { nullptr };
};

#endif // WRITEJOB_H
