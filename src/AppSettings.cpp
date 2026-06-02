#include "AppSettings.h"
#include <QSettings>

AppSettings::AppSettings(QObject *parent) : QObject(parent) {}

BackupSettings AppSettings::load() const {
    QSettings s("RTU-MIREA", "BackupSystem");
    BackupSettings settings;
    settings.sourcePaths           = s.value("sourcePaths").toStringList();
    settings.destinationPath       = s.value("destinationPath").toString();
    settings.autoScheduleEnabled   = s.value("autoScheduleEnabled", false).toBool();
    settings.scheduleIntervalDays  = s.value("scheduleIntervalDays", 1).toInt();
    return settings;
}

void AppSettings::save(const BackupSettings &settings) {
    QSettings s("RTU-MIREA", "BackupSystem");
    s.setValue("sourcePaths",          settings.sourcePaths);
    s.setValue("destinationPath",      settings.destinationPath);
    s.setValue("autoScheduleEnabled",  settings.autoScheduleEnabled);
    s.setValue("scheduleIntervalDays", settings.scheduleIntervalDays);
    s.sync();
}