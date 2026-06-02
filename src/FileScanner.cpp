#include "FileScanner.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>

FileScanner::FileScanner(QObject *parent) : QObject(parent) {}

QString FileScanner::computeChecksum(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return QString();

    QCryptographicHash hash(QCryptographicHash::Sha256);
    while (!file.atEnd()) {
        hash.addData(file.read(65536)); 
    }
    return hash.result().toHex();
}

QStringList FileScanner::getChangedFiles(const QStringList &sourcePaths,
                                          const QString &baseBackupPath) {
    QStringList changed;

    for (const QString &srcRoot : sourcePaths) {
        QDirIterator it(srcRoot, QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const QString srcFile = it.next();

            QString relative = QDir(srcRoot).relativeFilePath(srcFile);
            QString backupFile = baseBackupPath + "/" + relative;

            if (!QFileInfo::exists(backupFile)) {
                changed.append(srcFile);
            } else {
                if (computeChecksum(srcFile) != computeChecksum(backupFile))
                    changed.append(srcFile);
            }
        }
    }
    return changed;
}