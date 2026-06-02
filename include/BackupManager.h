#pragma once
#include <QObject>
#include "BackupSettings.h"
#include "RestorePoint.h"

class BackupRepository;
class Logger;

class BackupManager : public QObject {
    Q_OBJECT
public:
    explicit BackupManager(BackupRepository *repo,
                           Logger *logger,
                           QObject *parent = nullptr);
    void startBackup(const BackupSettings &settings);
    void pauseBackup();
    void resumeBackup();
    void restoreFromPoint(const RestorePoint &point, const QString &targetPath);

signals:
    void progressChanged(int percent);
    void statusChanged(const QString &status);
    void errorOccurred(const QString &message);
    void backupFinished(const RestorePoint &point);

private slots:
    void onJobFinished(const RestorePoint &point);
    void onJobProgress(int percent);
    void onJobStatus(const QString &status);
    void onJobError(const QString &message);

private:
    BackupRepository *m_repo;
    Logger           *m_logger;
    bool              m_paused = false;
};
