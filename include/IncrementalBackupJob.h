#pragma once
#include "BackupJob.h"
#include "RestorePoint.h"

class IncrementalBackupJob : public BackupJob {
    Q_OBJECT
public:
    explicit IncrementalBackupJob(const BackupSettings &settings,
                                  const RestorePoint &basePoint,
                                  QObject *parent = nullptr);
    void run() override;
private:
    RestorePoint m_basePoint;
};
