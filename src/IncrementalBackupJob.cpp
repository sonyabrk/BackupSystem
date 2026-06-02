#include "IncrementalBackupJob.h"
#include "FileScanner.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QDirIterator>

IncrementalBackupJob::IncrementalBackupJob(const BackupSettings &settings,
                                             const RestorePoint &basePoint,
                                             QObject *parent)
    : BackupJob(settings, parent), m_basePoint(basePoint) {}

void IncrementalBackupJob::run() {
    emit statusChanged("Выполняется инкрементальное копирование...");

    FileScanner scanner;
    QStringList changedFiles = scanner.getChangedFiles(m_settings.sourcePaths,
                                                        m_basePoint.path);

    QString destDir = m_settings.destinationPath + "/incremental_"
                      + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QDir().mkpath(destDir);

    int total = changedFiles.size();
    int done  = 0;

    for (const QString &srcFile : changedFiles) {
        for (const QString &srcRoot : m_settings.sourcePaths) {
            if (srcFile.startsWith(srcRoot)) {
                QString relative = QDir(srcRoot).relativeFilePath(srcFile);
                QString destFile = destDir + "/" + relative;
                QDir().mkpath(QFileInfo(destFile).absolutePath());
                QFile::copy(srcFile, destFile);
                break;
            }
        }
        ++done;
        emit progressChanged(total > 0 ? (done * 100 / total) : 100);
    }

    qint64 totalSize = 0;
    QDirIterator sizeIt(destDir, QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
    while (sizeIt.hasNext()) {
        sizeIt.next();
        totalSize += sizeIt.fileInfo().size();
    }

    RestorePoint point;
    point.type      = "incremental";
    point.timestamp = QDateTime::currentDateTime();
    point.path      = destDir;
    point.sizeBytes = totalSize;

    emit progressChanged(100);
    emit statusChanged("Завершено");
    emit finished(point);
}