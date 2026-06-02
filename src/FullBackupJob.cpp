#include "FullBackupJob.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>

FullBackupJob::FullBackupJob(const BackupSettings &settings, QObject *parent)
    : BackupJob(settings, parent) {}

void FullBackupJob::run() {
    emit statusChanged("Выполняется полное копирование...");

    QString destDir = m_settings.destinationPath + "/full_"
                      + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QDir().mkpath(destDir);

    QStringList allFiles;
    for (const QString &srcRoot : m_settings.sourcePaths) {
        QDirIterator it(srcRoot, QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext())
            allFiles.append(it.next());
    }

    int total = allFiles.size();
    int done  = 0;

    for (const QString &srcFile : allFiles) {
        for (const QString &srcRoot : m_settings.sourcePaths) {
            if (srcFile.startsWith(srcRoot)) {
                QString relative  = QDir(srcRoot).relativeFilePath(srcFile);
                QString destFile  = destDir + "/" + relative;
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
    point.type      = "full";
    point.timestamp = QDateTime::currentDateTime();
    point.path      = destDir;
    point.sizeBytes = totalSize;

    emit progressChanged(100);
    emit statusChanged("Завершено");
    emit finished(point);
}