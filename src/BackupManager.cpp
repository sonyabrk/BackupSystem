#include "BackupManager.h"
#include "BackupRepository.h"
#include "Logger.h"
#include "FullBackupJob.h"
#include "IncrementalBackupJob.h"
#include <QThreadPool>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QFileInfo>

BackupManager::BackupManager(BackupRepository *repo, Logger *logger, QObject *parent)
    : QObject(parent), m_repo(repo), m_logger(logger) {}

void BackupManager::startBackup(const BackupSettings &settings) {
    m_logger->info("Запуск бэкапа...");

    RestorePoint lastFull = m_repo->getLastFullBackup();
    BackupJob *job = nullptr;

    if (lastFull.path.isEmpty()) {
        m_logger->info("Полный бэкап не найден — создаём полную копию.");
        job = new FullBackupJob(settings);
    } else {
        m_logger->info("Найден полный бэкап: " + lastFull.path + " — создаём инкрементальную копию.");
        job = new IncrementalBackupJob(settings, lastFull);
    }

    connect(job, &BackupJob::progressChanged, this, &BackupManager::onJobProgress);
    connect(job, &BackupJob::statusChanged,   this, &BackupManager::onJobStatus);
    connect(job, &BackupJob::errorOccurred,   this, &BackupManager::onJobError);
    connect(job, &BackupJob::finished,        this, &BackupManager::onJobFinished);

    QThreadPool::globalInstance()->start(job);
}

void BackupManager::pauseBackup()  { m_paused = true;  emit statusChanged("Приостановлено"); }
void BackupManager::resumeBackup() { m_paused = false; emit statusChanged("Выполняется"); }

void BackupManager::restoreFromPoint(const RestorePoint &point, const QString &targetPath) {
    m_logger->info("Восстановление из: " + point.path + " в: " + targetPath);
    QDir().mkpath(targetPath);

    QDirIterator it(point.path, QDir::Files | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString srcFile  = it.next();
        const QString relative = QDir(point.path).relativeFilePath(srcFile);
        const QString dstFile  = targetPath + "/" + relative;
        QDir().mkpath(QFileInfo(dstFile).absolutePath());
        if (!QFileInfo::exists(dstFile))
            QFile::copy(srcFile, dstFile);
    }
    m_logger->info("Восстановление завершено.");
}

void BackupManager::onJobFinished(const RestorePoint &point) {
    m_repo->saveRestorePoint(point);
    m_logger->info("Бэкап завершён и сохранён: " + point.path);
    emit backupFinished(point);
}
void BackupManager::onJobProgress(int p)            { emit progressChanged(p); }
void BackupManager::onJobStatus(const QString &s)   { emit statusChanged(s); }
void BackupManager::onJobError(const QString &msg)  {
    m_logger->error(msg);
    emit errorOccurred(msg);
}