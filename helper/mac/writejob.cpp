#include "writejob.h"

#include <QCoreApplication>
#include <QTimer>
#include <QTextStream>
#include <QProcess>

#include <QDebug>

WriteJob::WriteJob(const QString &what, const QString &where)
    : QObject(nullptr), what(what), where(where), dd(new QProcess(this))
{
}
