#include "fakedrivemanager.h"

#include <QTimer>

FakeDriveProvider::FakeDriveProvider(DriveManager *parent)
    : DriveProvider(parent) {
    QTimer::singleShot(0, this, &FakeDriveProvider::connectDrives);
}

void FakeDriveProvider::createNewRestoreable() {
    qCritical() << "HYR";
    static uint64_t size = 4444444444;
    emit driveConnected(new FakeDrive(this, "Contains Live", size, true));
    size++;
}

void FakeDriveProvider::connectDrives() {
    emit driveConnected(new FakeDrive(this, "Okay", 1234567890, false));
    emit driveConnected(new FakeDrive(this, "Fails", 987654321, false));
    emit driveConnected(new FakeDrive(this, "Not Large Enough", 10000, false));
    QTimer::singleShot(2000, this, &FakeDriveProvider::createNewRestoreable);
}

FakeDrive::FakeDrive(FakeDriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, name, size, containsLive) {
    m_progress->setTo(size);
}

void FakeDrive::write(ReleaseVariant *data) {
    Drive::write(data);

    m_progress->setValue(0);
    m_writtenTo = true;
    emit beingWrittenToChanged();
    QTimer::singleShot(100, this, &FakeDrive::writingAdvanced);
}

void FakeDrive::restore() {
    m_restoreStatus = Drive::RESTORING;
    emit restoreStatusChanged();
    QTimer::singleShot(5000, this, &FakeDrive::restoringFinished);
}

void FakeDrive::writingAdvanced() {
    m_progress->setValue(m_progress->value() + 12345678);
    if (m_progress->value() >= m_size) {
        m_writtenTo = false;
        emit beingWrittenToChanged();
    }
    else if (m_name == "Fails" && m_progress->value() >= m_size / 2) {
        m_writtenTo = false;
        emit beingWrittenToChanged();
        m_error = "Some error string.";
        emit errorChanged();
        m_image->setStatus(ReleaseVariant::FAILED);
    }
    else {
        QTimer::singleShot(100, this, &FakeDrive::writingAdvanced);
    }
}

void FakeDrive::restoringFinished() {
    if (m_size % 2)
        m_restoreStatus = Drive::RESTORE_ERROR;
    else
        m_restoreStatus = Drive::RESTORED;
    emit restoreStatusChanged();

    QTimer::singleShot(5000, this, &FakeDrive::selfdestruct);
}

void FakeDrive::selfdestruct() {
    emit qobject_cast<FakeDriveProvider*>(parent())->driveRemoved(this);
    QTimer::singleShot(2000, qobject_cast<FakeDriveProvider*>(parent()), &FakeDriveProvider::createNewRestoreable);
    this->deleteLater();
}

bool FakeDrive::beingWrittenTo() const {
    return m_writtenTo;
}
