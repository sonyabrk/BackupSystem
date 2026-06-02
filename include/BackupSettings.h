#pragma once
#include <QString>
#include <QStringList>

struct BackupSettings {
    QStringList sourcePaths;
    QString     destinationPath;
    bool        autoScheduleEnabled = false;
    int         scheduleIntervalDays = 1;
};
