#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QThreadPool>
#include <QDateTime>
#include "IncrementalBackupJob.h"
#include "AppSettings.h"
#include "BackupRepository.h"

static void writeFile(const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream(&f) << content;
}

static RestorePoint runIncJob(IncrementalBackupJob *job) {
    RestorePoint result;
    QEventLoop loop;
    QObject::connect(job, &BackupJob::finished,
                     [&](const RestorePoint &rp){ result = rp; loop.quit(); });
    QThreadPool::globalInstance()->start(job);
    loop.exec();
    return result;
}

class IncrementalBackupJobTest : public ::testing::Test {
protected:
    QTemporaryDir srcDir, baseDir, dstDir;
};

// тип точки восстановления равен "incremental"
TEST_F(IncrementalBackupJobTest, TypeIsIncremental) {
    writeFile(srcDir.filePath("a.txt"), "v1");
    writeFile(baseDir.filePath("a.txt"), "v0");  // изменён

    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();

    RestorePoint base; base.path = baseDir.path(); base.type = "full";
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);
    EXPECT_EQ(rp.type, "incremental");
}

// только изменённый файл попадает в инкрементальную копию
TEST_F(IncrementalBackupJobTest, OnlyChangedFileCopied) {
    writeFile(srcDir.filePath("changed.txt"),   "new content");
    writeFile(srcDir.filePath("unchanged.txt"), "same content");
    writeFile(baseDir.filePath("changed.txt"),  "old content");
    writeFile(baseDir.filePath("unchanged.txt"),"same content");

    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();

    RestorePoint base; base.path = baseDir.path(); base.type = "full";
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);

    EXPECT_TRUE(QFileInfo::exists(rp.path + "/changed.txt"));
    EXPECT_FALSE(QFileInfo::exists(rp.path + "/unchanged.txt"));
}

// новый файл (отсутствующий в базе) попадает в копию
TEST_F(IncrementalBackupJobTest, NewFileIncluded) {
    writeFile(srcDir.filePath("new.txt"), "brand new");

    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();

    RestorePoint base; base.path = baseDir.path(); base.type = "full";
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);
    EXPECT_TRUE(QFileInfo::exists(rp.path + "/new.txt"));
}

// если ничего не изменилось — директория создаётся, но пустая
TEST_F(IncrementalBackupJobTest, NoChangesResultsInEmptyBackupDir) {
    writeFile(srcDir.filePath("same.txt"),  "identical");
    writeFile(baseDir.filePath("same.txt"), "identical");

    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();

    RestorePoint base; base.path = baseDir.path(); base.type = "full";
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);

    QDir d(rp.path);
    EXPECT_TRUE(d.exists());
    EXPECT_TRUE(d.entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty());
}

// точка восстановления содержит корректный timestamp
TEST_F(IncrementalBackupJobTest, TimestampIsValid) {
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    RestorePoint base; base.path = baseDir.path();
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);
    EXPECT_TRUE(rp.timestamp.isValid());
}

// содержимое скопированного изменённого файла соответствует источнику
TEST_F(IncrementalBackupJobTest, CopiedFileContentMatchesSource) {
    writeFile(srcDir.filePath("mod.txt"),  "new version");
    writeFile(baseDir.filePath("mod.txt"), "old version");

    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    RestorePoint base; base.path = baseDir.path();
    auto *job = new IncrementalBackupJob(s, base);
    RestorePoint rp = runIncJob(job);

    QFile f(rp.path + "/mod.txt");
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    EXPECT_EQ(QString::fromUtf8(f.readAll()).trimmed(), "new version");
}

// задача завершается без ошибок при пустом источнике
TEST_F(IncrementalBackupJobTest, EmptySourceNoThrow) {
    BackupSettings s;
    s.sourcePaths    = {srcDir.path()};
    s.destinationPath = dstDir.path();
    RestorePoint base; base.path = baseDir.path();
    auto *job = new IncrementalBackupJob(s, base);
    EXPECT_NO_THROW({ runIncJob(job); });
}