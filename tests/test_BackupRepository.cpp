#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDateTime>
#include "BackupRepository.h"

class BackupRepositoryTest : public ::testing::Test {
protected:
    QTemporaryDir tmpDir;
    BackupRepository *repo = nullptr;

    void SetUp() override {
        repo = new BackupRepository(tmpDir.filePath("test.db"));
        ASSERT_TRUE(repo->initialize());
    }
    void TearDown() override {
        delete repo;
    }
};

// после инициализации список точек восстановления пуст
TEST_F(BackupRepositoryTest, EmptyAfterInit) {
    EXPECT_TRUE(repo->getAllRestorePoints().isEmpty());
}

// getLastFullBackup() возвращает пустую точку, если нет полных копий
TEST_F(BackupRepositoryTest, GetLastFullBackupReturnsEmptyWhenNone) {
    RestorePoint rp = repo->getLastFullBackup();
    EXPECT_TRUE(rp.path.isEmpty());
}

// saveRestorePoint() увеличивает размер списка на 1
TEST_F(BackupRepositoryTest, SaveIncreasesCount) {
    RestorePoint rp;
    rp.type      = "full";
    rp.timestamp = QDateTime::currentDateTime();
    rp.sizeBytes = 1024;
    rp.path      = "/tmp/full_20250101";
    repo->saveRestorePoint(rp);
    EXPECT_EQ(repo->getAllRestorePoints().size(), 1);
}

// saveRestorePoint() корректно сохраняет тип и путь
TEST_F(BackupRepositoryTest, SavePreservesTypeAndPath) {
    RestorePoint rp;
    rp.type      = "incremental";
    rp.timestamp = QDateTime::currentDateTime();
    rp.sizeBytes = 512;
    rp.path      = "/tmp/inc_20250102";
    repo->saveRestorePoint(rp);
    QList<RestorePoint> list = repo->getAllRestorePoints();
    ASSERT_EQ(list.size(), 1);
    EXPECT_EQ(list[0].type, "incremental");
    EXPECT_EQ(list[0].path, "/tmp/inc_20250102");
}

// getLastFullBackup() возвращает последнюю полную копию
TEST_F(BackupRepositoryTest, GetLastFullBackupReturnsLatest) {
    RestorePoint rp1;
    rp1.type      = "full";
    rp1.timestamp = QDateTime::fromString("2025-01-01T00:00:00", Qt::ISODate);
    rp1.path      = "/tmp/full_old";
    repo->saveRestorePoint(rp1);

    RestorePoint rp2;
    rp2.type      = "full";
    rp2.timestamp = QDateTime::fromString("2025-06-01T00:00:00", Qt::ISODate);
    rp2.path      = "/tmp/full_new";
    repo->saveRestorePoint(rp2);

    RestorePoint last = repo->getLastFullBackup();
    EXPECT_EQ(last.path, "/tmp/full_new");
}

// deleteRestorePoint() удаляет запись по id
TEST_F(BackupRepositoryTest, DeleteRemovesRecord) {
    RestorePoint rp;
    rp.type      = "full";
    rp.timestamp = QDateTime::currentDateTime();
    rp.path      = "/tmp/to_delete";
    repo->saveRestorePoint(rp);

    QList<RestorePoint> list = repo->getAllRestorePoints();
    ASSERT_EQ(list.size(), 1);
    int id = list[0].id;

    repo->deleteRestorePoint(id);
    EXPECT_TRUE(repo->getAllRestorePoints().isEmpty());
}

// getAllRestorePoints() возвращает несколько записей в порядке убывания
TEST_F(BackupRepositoryTest, GetAllReturnsSortedByTimestampDesc) {
    for (int i = 1; i <= 3; ++i) {
        RestorePoint rp;
        rp.type      = "incremental";
        rp.timestamp = QDateTime::fromString(
            QString("2025-0%1-01T00:00:00").arg(i), Qt::ISODate);
        rp.path      = QString("/tmp/inc_%1").arg(i);
        repo->saveRestorePoint(rp);
    }
    QList<RestorePoint> list = repo->getAllRestorePoints();
    ASSERT_EQ(list.size(), 3);
    // первый элемент должен быть самым поздним
    EXPECT_GT(list[0].timestamp, list[1].timestamp);
    EXPECT_GT(list[1].timestamp, list[2].timestamp);
}