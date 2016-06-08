#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>

#include <QDebug>

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where)
{
    QTimer::singleShot(0, this, &WriteJob::work);
}

void WriteJob::work()
{
    if (!dd)
        dd = new QProcess();
    dd->setProgram("dd");

    connect(dd, &QProcess::readyRead, this, &WriteJob::onReadyRead);
    connect(dd, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(onFinished(int,QProcess::ExitStatus)));

    QStringList args;
    args << QString("if=%1").arg(what);
    args << QString("of=%1").arg(where);
    args << "bs=1M";
    args << "iflag=direct";
    args << "oflag=direct";
    args << "conv=fdatasync";
    args << "status=progress";
    dd->setArguments(args);

    QProcessEnvironment env = dd->processEnvironment();
    env.insert("LC_ALL", "C");
    dd->setProcessEnvironment(env);
    dd->setProcessChannelMode(QProcess::MergedChannels);

    qCritical() << "Starting" << "dd" << args;

    dd->start(QIODevice::ReadOnly);
}

void WriteJob::onReadyRead() {
    QRegExp r("^([0-9]+)");
    while (dd->bytesAvailable() > 0) {
        QString line = dd->readLine().trimmed();
        if (r.indexIn(line) >= 0) {
            bool ok = false;
            uint64_t val = r.cap(0).toULongLong(&ok);
            if (val > 0 && ok) {
                out << val << '\n';
                out.flush();
            }
        }
    }
}

void WriteJob::onFinished(int exitCode, QProcess::ExitStatus status) {
    if (exitCode == 0) {
        err << "OK\n";
        err.flush();
        qApp->exit(0);
    }
    else {
        // TODO
        err << dd->readAllStandardError();
        err.flush();
        qApp->exit(2);
    }
}
