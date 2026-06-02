#pragma once
#include <QObject>
#include <QStringList>

class FileScanner : public QObject {
    Q_OBJECT
public:
    explicit FileScanner(QObject *parent = nullptr);
    QStringList getChangedFiles(const QStringList &sourcePaths,
                                const QString &baseBackupPath);
    static QString computeChecksum(const QString &filePath);
};
