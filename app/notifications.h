#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>

class Notifications
{
public:
    static void notify(const QString &title, const QString &body);
};

#endif // NOTIFICATIONS_H
