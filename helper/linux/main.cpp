#include <QCoreApplication>
#include <QTextStream>

#include "restorejob.h"
#include "writejob.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    if (app.arguments().count() == 3 && app.arguments()[1] == "restore") {
        new RestoreJob(app.arguments()[2]);
    }
    else if (app.arguments().count() == 4 && app.arguments()[1] == "write") {
        new WriteJob(app.arguments()[2], app.arguments()[3]);
    }
    else {
        QTextStream err(stderr);
        err << "Helper: Wrong arguments entered";
        return 1;
    }
    return app.exec();
}
