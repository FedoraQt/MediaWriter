#include "fakedrivemanager.h"

#include <QTimer>

FakeDriveProvider::FakeDriveProvider(DriveManager *parent)
    : DriveProvider(parent) {
    QTimer::singleShot(0, this, &FakeDriveProvider::connectDrives);
}

void FakeDriveProvider::createNewRestoreable() {
    static uint64_t size = 4444444444;
    emit driveConnected(new FakeDrive(this, "Contains Live", size, true));
    size++;
}

void FakeDriveProvider::createNewGetsDisconnected() {
    emit driveConnected(new FakeDrive(this, "Gets Disconnected", 1000000000, false));
}

void FakeDriveProvider::connectDrives() {
    emit driveConnected(new FakeDrive(this, "Okay", 12345678900, false));
    emit driveConnected(new FakeDrive(this, "Fails", 9876543210, false));
    emit driveConnected(new FakeDrive(this, "Not Large Enough", 10000, false));
    emit driveConnected(new FakeDrive(this, "Gets Disconnected", 10000000000, false));
    QTimer::singleShot(2000, this, &FakeDriveProvider::createNewRestoreable);
}

FakeDrive::FakeDrive(FakeDriveProvider *parent, const QString &name, uint64_t size, bool containsLive)
    : Drive(parent, name, size, containsLive) {
    m_progress->setTo(size);
}

void FakeDrive::write(ReleaseVariant *data) {
    Drive::write(data);

    m_progress->setValue(0);
    m_image->setStatus(ReleaseVariant::WRITING);
    QTimer::singleShot(100, this, &FakeDrive::writingAdvanced);
}

void FakeDrive::restore() {
    m_restoreStatus = Drive::RESTORING;
    emit restoreStatusChanged();
    QTimer::singleShot(5000, this, &FakeDrive::restoringFinished);
}

void FakeDrive::writingAdvanced() {
    m_progress->setValue(m_progress->value() + 123456789);
    if (m_progress->value() >= m_size) {
        m_image->setStatus(ReleaseVariant::FINISHED);
    }
    else if (m_name == "Fails" && m_progress->value() >= m_size / 2) {
        m_image->setStatus(ReleaseVariant::FAILED);
        m_image->setErrorString("Some error string.");
    }
    else if (m_name == "Gets Disconnected" && m_progress->value() >= m_size / 2) {
        emit qobject_cast<FakeDriveProvider*>(parent())->driveRemoved(this);
        QTimer::singleShot(5000, qobject_cast<FakeDriveProvider*>(parent()), &FakeDriveProvider::createNewGetsDisconnected);
        this->deleteLater();
        m_image->setStatus(ReleaseVariant::FAILED);
        m_image->setErrorString(QString("Drive %1 got disconnected.").arg(name()));
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
