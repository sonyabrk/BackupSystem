#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QSettings>
#include "AppSettings.h"

// очистка QSettings
class AppSettingsTest : public ::testing::Test {
protected:
    void SetUp() override {
        QSettings s("RTU-MIREA", "BackupSystem");
        s.clear();
        s.sync();
    }
    void TearDown() override {
        QSettings s("RTU-MIREA", "BackupSystem");
        s.clear();
        s.sync();
    }
};

// load() возвращает пустой destinationPath, если ничего не сохранено
TEST_F(AppSettingsTest, LoadReturnsEmptyDestinationByDefault) {
    AppSettings as;
    BackupSettings bs = as.load();
    EXPECT_TRUE(bs.destinationPath.isEmpty());
}

// load() возвращает пустой sourcePaths по умолчанию
TEST_F(AppSettingsTest, LoadReturnsEmptySourcePathsByDefault) {
    AppSettings as;
    BackupSettings bs = as.load();
    EXPECT_TRUE(bs.sourcePaths.isEmpty());
}

// autoScheduleEnabled по умолчанию false
TEST_F(AppSettingsTest, LoadReturnsAutoScheduleDisabledByDefault) {
    AppSettings as;
    BackupSettings bs = as.load();
    EXPECT_FALSE(bs.autoScheduleEnabled);
}

// scheduleIntervalDays по умолчанию 1
TEST_F(AppSettingsTest, LoadReturnsDefaultIntervalOne) {
    AppSettings as;
    BackupSettings bs = as.load();
    EXPECT_EQ(bs.scheduleIntervalDays, 1);
}

// save() + load() сохраняет и восстанавливает destinationPath
TEST_F(AppSettingsTest, SaveAndLoadDestinationPath) {
    AppSettings as;
    BackupSettings bs;
    bs.destinationPath = "/tmp/backup_dest";
    as.save(bs);
    BackupSettings loaded = as.load();
    EXPECT_EQ(loaded.destinationPath, "/tmp/backup_dest");
}

// save() + load() сохраняет список sourcePaths
TEST_F(AppSettingsTest, SaveAndLoadSourcePaths) {
    AppSettings as;
    BackupSettings bs;
    bs.sourcePaths = QStringList{"/home/user/docs", "/home/user/photos"};
    as.save(bs);
    BackupSettings loaded = as.load();
    EXPECT_EQ(loaded.sourcePaths.size(), 2);
    EXPECT_EQ(loaded.sourcePaths[0], "/home/user/docs");
    EXPECT_EQ(loaded.sourcePaths[1], "/home/user/photos");
}

// save() + load() сохраняет флаг autoScheduleEnabled = true
TEST_F(AppSettingsTest, SaveAndLoadAutoScheduleEnabled) {
    AppSettings as;
    BackupSettings bs;
    bs.autoScheduleEnabled  = true;
    bs.scheduleIntervalDays = 7;
    as.save(bs);
    BackupSettings loaded = as.load();
    EXPECT_TRUE(loaded.autoScheduleEnabled);
    EXPECT_EQ(loaded.scheduleIntervalDays, 7);
}