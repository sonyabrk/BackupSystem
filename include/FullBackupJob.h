#pragma once
#include "BackupJob.h"

class FullBackupJob : public BackupJob {
    Q_OBJECT
public:
    explicit FullBackupJob(const BackupSettings &settings, QObject *parent = nullptr);
    void run() override;
};
