#include "fakedrivemanager.h"

#include <QTimer>

FakeDriveProvider::FakeDriveProvider(DriveManager *parent)
    : DriveProvider(parent) {
    QTimer::singleShot(0, this, &FakeDriveProvider::connectDrives);
}

void FakeDriveProvider::connectDrives() {
    emit driveConnected(new FakeDrive(this, "Okay", 1234567890, false));
    emit driveConnected(new FakeDrive(this, "Fails", 9876543210, false));
    emit driveConnected(new FakeDrive(this, "Contains Live", 4444444444, true));
}

FakeDrive::FakeDrive(FakeDriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, name, size, containsLive) {
    m_progress->setTo(size);
}

void FakeDrive::write(ReleaseVariant *data) {
    Drive::write(data);

    m_writtenTo = true;
    emit beingWrittenToChanged();
    QTimer::singleShot(100, this, &FakeDrive::writingAdvanced);
}

void FakeDrive::restore() {
    m_restoreStatus = Drive::RESTORING;
    emit restoreStatusChanged();
    QTimer::singleShot(10000, this, &FakeDrive::restoringFinished);
}

void FakeDrive::writingAdvanced() {
    m_progress->setValue(m_progress->value() + 12345678);
    if (m_name != "Fails" && m_progress->value() >= m_size) {
        m_writtenTo = false;
        emit beingWrittenToChanged();
    }
    else if (m_name != "Fails")
        QTimer::singleShot(100, this, &FakeDrive::writingAdvanced);
    else
        m_writtenTo = false;
        emit beingWrittenToChanged();
}

void FakeDrive::restoringFinished() {
    m_restoreStatus = Drive::RESTORED;
    emit restoreStatusChanged();
}

bool FakeDrive::beingWrittenTo() const {
    return m_writtenTo;
}
