#ifndef FAKEDRIVEMANAGER_H
#define FAKEDRIVEMANAGER_H

#include <QObject>

#include "drivemanager.h"


class FakeDriveProvider : public DriveProvider {
    Q_OBJECT
public:
    FakeDriveProvider(DriveManager *parent);
public slots:
    void createNewRestoreable();
private slots:
    void connectDrives();
};

/**
 * @brief The Drive class
 *
 * Contains a fake drive implementation
 */
class FakeDrive : public Drive {
    Q_OBJECT
public:
    FakeDrive(FakeDriveProvider *parent, const QString &name, uint64_t size, bool containsLive = false);

    virtual bool beingWrittenTo() const override;

    Q_INVOKABLE virtual void write(ReleaseVariant *data);
    Q_INVOKABLE virtual void restore();

private slots:
    void writingAdvanced();
    void restoringFinished();
    void selfdestruct();

private:
    bool m_writtenTo { false };
};

#endif // FAKEDRIVEMANAGER_H
