#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QThreadPool>
#include "FullBackupJob.h"
#include "AppSettings.h"

static void writeFile(const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream(&f) << content;
}

class FullBackupJobTest : public ::testing::Test {
protected:
    QTemporaryDir srcDir, dstDir;
};

// синхронный запуск задачи через QThreadPool с ожиданием сигнала finished
static RestorePoint runJob(FullBackupJob *job) {
    RestorePoint result;
    QEventLoop loop;
    QObject::connect(job, &BackupJob::finished,
                     [&](const RestorePoint &rp){ result = rp; loop.quit(); });
    QThreadPool::globalInstance()->start(job);
    loop.exec();
    return result;
}

// после выполнения создаётся директория назначения
TEST_F(FullBackupJobTest, DestDirCreated) {
    writeFile(srcDir.filePath("a.txt"), "content a");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    EXPECT_TRUE(QDir(rp.path).exists());
}

// тип точки восстановления равен "full"
TEST_F(FullBackupJobTest, RestorePointTypeIsFull) {
    writeFile(srcDir.filePath("b.txt"), "content b");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    EXPECT_EQ(rp.type, "full");
}

// файл из источника скопирован в назначение
TEST_F(FullBackupJobTest, FilesCopiedToDestination) {
    writeFile(srcDir.filePath("c.txt"), "hello");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    EXPECT_TRUE(QFileInfo::exists(rp.path + "/c.txt"));
}

// содержимое скопированного файла совпадает с исходным
TEST_F(FullBackupJobTest, CopiedFileContentMatches) {
    writeFile(srcDir.filePath("d.txt"), "unique_content_xyz");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    QFile f(rp.path + "/d.txt");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    EXPECT_EQ(QString::fromUtf8(f.readAll()).trimmed(), "unique_content_xyz");
}

// точка восстановления содержит корректный timestamp (не null)
TEST_F(FullBackupJobTest, RestorePointTimestampIsValid) {
    writeFile(srcDir.filePath("e.txt"), "ts");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    EXPECT_TRUE(rp.timestamp.isValid());
}

// если источников несколько, файлы из обоих копируются
TEST_F(FullBackupJobTest, MultipleSourcePathsAllFilesCopied) {
    QTemporaryDir src2;
    writeFile(srcDir.filePath("f1.txt"), "from src1");
    writeFile(src2.filePath("f2.txt"),  "from src2");
    BackupSettings s;
    s.sourcePaths    = {srcDir.path(), src2.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    RestorePoint rp = runJob(job);
    EXPECT_TRUE(QFileInfo::exists(rp.path + "/f1.txt"));
    EXPECT_TRUE(QFileInfo::exists(rp.path + "/f2.txt"));
}

// пустой источник — задача завершается без ошибок
TEST_F(FullBackupJobTest, EmptySourceCompletesWithoutError) {
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    auto *job = new FullBackupJob(s);
    EXPECT_NO_THROW({ RestorePoint rp = runJob(job); });
}