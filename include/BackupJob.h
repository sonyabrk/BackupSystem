#pragma once
#include <QObject>
#include <QRunnable>
#include "BackupSettings.h"
#include "RestorePoint.h"

class BackupJob : public QObject, public QRunnable {
    Q_OBJECT
public:
    explicit BackupJob(const BackupSettings &settings, QObject *parent = nullptr);
    virtual ~BackupJob() = default;
    void run() override = 0;

signals:
    void progressChanged(int percent);
    void statusChanged(const QString &status);
    void errorOccurred(const QString &message);
    void finished(const RestorePoint &point);

protected:
    BackupSettings m_settings;
};
