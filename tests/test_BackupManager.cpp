#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QThread>
#include <QTimer>
#include <QCoreApplication>
#include "BackupManager.h"
#include "BackupRepository.h"
#include "Logger.h"
#include "AppSettings.h"

static void writeFile(const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream(&f) << content;
}

class BackupManagerTest : public ::testing::Test {
protected:
    QTemporaryDir tmpDir;
    Logger           *logger  = nullptr;
    BackupRepository *repo    = nullptr;
    BackupManager    *manager = nullptr;

    void SetUp() override {
        logger  = new Logger(tmpDir.filePath("test.log"));
        repo    = new BackupRepository(tmpDir.filePath("test.db"));
        repo->initialize();
        manager = new BackupManager(repo, logger);
    }
    void TearDown() override {
        delete manager;
        delete repo;
        delete logger;
    }

    void waitForDone(int timeoutMs = 5000) {
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        timer.setInterval(timeoutMs);

        QObject::connect(manager, &BackupManager::statusChanged,
                         &loop, [&loop](const QString &s){
                             if (s == "Завершено") loop.quit();
                         }, Qt::QueuedConnection);

        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start();
        loop.exec();

        QCoreApplication::processEvents();
    }
};

// первый бэкап создаёт полную точку восстановления
TEST_F(BackupManagerTest, FirstBackupCreatesFullRestorePoint) {
    QTemporaryDir src, dst;
    writeFile(src.filePath("a.txt"), "aaa");
    BackupSettings s;
    s.sourcePaths     = {src.path()};
    s.destinationPath = dst.path();

    manager->startBackup(s);
    waitForDone(8000);

    QCoreApplication::processEvents();
    QThread::msleep(100);
    QCoreApplication::processEvents();

    EXPECT_FALSE(repo->getLastFullBackup().path.isEmpty());
}

// второй бэкап создаёт инкрементальную точку
TEST_F(BackupManagerTest, SecondBackupCreatesIncrementalPoint) {
    QTemporaryDir src, dst;
    writeFile(src.filePath("b.txt"), "v1");
    BackupSettings s;
    s.sourcePaths     = {src.path()};
    s.destinationPath = dst.path();

    manager->startBackup(s);
    waitForDone();

    writeFile(src.filePath("b.txt"), "v2");
    manager->startBackup(s);
    waitForDone();

    QList<RestorePoint> all = repo->getAllRestorePoints();
    ASSERT_GE(all.size(), 2);
    bool hasIncremental = std::any_of(all.begin(), all.end(),
                                      [](const RestorePoint &rp){ return rp.type == "incremental"; });
    EXPECT_TRUE(hasIncremental);
}

// pauseBackup() меняет внутреннее состояние
TEST_F(BackupManagerTest, PauseDoesNotCrash) {
    EXPECT_NO_THROW(manager->pauseBackup());
}

// resumeBackup() после pause не падает
TEST_F(BackupManagerTest, ResumeAfterPauseDoesNotCrash) {
    manager->pauseBackup();
    EXPECT_NO_THROW(manager->resumeBackup());
}

// restoreFromPoint() копирует файлы в целевую директорию
TEST_F(BackupManagerTest, RestoreFromPointCopiesFiles) {
    QTemporaryDir backupDir, targetDir;
    writeFile(backupDir.filePath("restored.txt"), "restored content");
    RestorePoint rp;
    rp.path = backupDir.path();
    manager->restoreFromPoint(rp, targetDir.path());
    EXPECT_TRUE(QFileInfo::exists(targetDir.filePath("restored.txt")));
}

// restoreFromPoint() не перезаписывает существующий файл
TEST_F(BackupManagerTest, RestoreDoesNotOverwriteExistingFile) {
    QTemporaryDir backupDir, targetDir;
    writeFile(backupDir.filePath("file.txt"), "backup version");
    writeFile(targetDir.filePath("file.txt"), "local version");
    RestorePoint rp;
    rp.path = backupDir.path();
    manager->restoreFromPoint(rp, targetDir.path());
    QFile f(targetDir.filePath("file.txt"));
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    EXPECT_EQ(QString::fromUtf8(f.readAll()).trimmed(), "local version");
}

// после завершения бэкапа прогресс достигает 100
TEST_F(BackupManagerTest, ProgressReaches100OnCompletion) {
    QTemporaryDir src, dst;
    writeFile(src.filePath("c.txt"), "ccc");
    BackupSettings s;
    s.sourcePaths     = {src.path()};
    s.destinationPath = dst.path();

    int maxProgress = 0;
    QObject::connect(manager, &BackupManager::progressChanged,
                     [&](int p){ if (p > maxProgress) maxProgress = p; });

    manager->startBackup(s);
    waitForDone();
    EXPECT_EQ(maxProgress, 100);
}