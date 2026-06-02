#pragma once
#include <QObject>
#include <QList>
#include "RestorePoint.h"

class BackupRepository : public QObject {
    Q_OBJECT
public:
    explicit BackupRepository(const QString &dbPath, QObject *parent = nullptr);
    bool initialize();
    void saveRestorePoint(const RestorePoint &point);
    QList<RestorePoint> getAllRestorePoints() const;
    RestorePoint getLastFullBackup() const;
    void deleteRestorePoint(int id);
private:
    QString m_dbPath;
};
