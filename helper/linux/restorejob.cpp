#include "restorejob.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QThread>
#include <QTimer>

RestoreJob::RestoreJob(const QString &where)
    : QObject(nullptr)
{
    QTimer::singleShot(0, this, &RestoreJob::work);
}

void RestoreJob::work()
{
    QThread::sleep(10);
    qApp->exit(0);
}
