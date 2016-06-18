#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>

#include <QDebug>

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where), dd(new QProcess(this))
{
    dd->setProgram(qApp->applicationDirPath() + "/dd.exe");

    QStringList args;
    args << QString("if=%1").arg("/dev/zero");
    args << QString("of=%1").arg("/dev/null");
    args << QString("bs=1M");
    args << QString("count=100000");
    dd->setArguments(args);

    connect(dd, &QProcess::readyRead, this, &WriteJob::onReadyRead);
}

void WriteJob::onReadyRead() {
    while (dd->bytesAvailable() > 0) {
        qDebug() << dd->readLine();
    }
}
