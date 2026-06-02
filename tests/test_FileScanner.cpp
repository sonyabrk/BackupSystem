#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include "FileScanner.h"

// создаёт файл с заданным содержимым
static void writeFile(const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream(&f) << content;
}

class FileScannerTest : public ::testing::Test {
protected:
    QTemporaryDir tmpDir;
    FileScanner scanner;
};

// computeChecksum() не пустая для существующего файла
TEST_F(FileScannerTest, ChecksumNotEmptyForExistingFile) {
    QString path = tmpDir.filePath("a.txt");
    writeFile(path, "hello");
    EXPECT_FALSE(scanner.computeChecksum(path).isEmpty());
}

// computeChecksum() пустая для несуществующего файла
TEST_F(FileScannerTest, ChecksumEmptyForMissingFile) {
    EXPECT_TRUE(scanner.computeChecksum(tmpDir.filePath("nonexistent.txt")).isEmpty());
}

// Два одинаковых файла имеют одинаковую контрольную сумму
TEST_F(FileScannerTest, SameContentSameChecksum) {
    QString p1 = tmpDir.filePath("x1.txt");
    QString p2 = tmpDir.filePath("x2.txt");
    writeFile(p1, "same content");
    writeFile(p2, "same content");
    EXPECT_EQ(scanner.computeChecksum(p1), scanner.computeChecksum(p2));
}

// разные файлы имеют разные контрольные суммы
TEST_F(FileScannerTest, DifferentContentDifferentChecksum) {
    QString p1 = tmpDir.filePath("y1.txt");
    QString p2 = tmpDir.filePath("y2.txt");
    writeFile(p1, "aaa");
    writeFile(p2, "bbb");
    EXPECT_NE(scanner.computeChecksum(p1), scanner.computeChecksum(p2));
}

// getChangedFiles() возвращает новый файл (его нет в базовом бэкапе)
TEST_F(FileScannerTest, NewFileIsDetectedAsChanged) {
    QString srcDir    = tmpDir.filePath("src");
    QString backupDir = tmpDir.filePath("backup");
    QDir().mkpath(srcDir);
    QDir().mkpath(backupDir);
    writeFile(srcDir + "/new.txt", "new file");

    QStringList changed = scanner.getChangedFiles({srcDir}, backupDir);
    ASSERT_EQ(changed.size(), 1);
    EXPECT_TRUE(changed[0].endsWith("new.txt"));
}

// getChangedFiles() не возвращает неизменённый файл
TEST_F(FileScannerTest, UnchangedFileNotInChanged) {
    QString srcDir    = tmpDir.filePath("src2");
    QString backupDir = tmpDir.filePath("backup2");
    QDir().mkpath(srcDir);
    QDir().mkpath(backupDir);
    writeFile(srcDir    + "/same.txt", "same");
    writeFile(backupDir + "/same.txt", "same");

    QStringList changed = scanner.getChangedFiles({srcDir}, backupDir);
    EXPECT_TRUE(changed.isEmpty());
}

// getChangedFiles() обнаруживает изменённый файл (разное содержимое)
TEST_F(FileScannerTest, ModifiedFileIsDetectedAsChanged) {
    QString srcDir    = tmpDir.filePath("src3");
    QString backupDir = tmpDir.filePath("backup3");
    QDir().mkpath(srcDir);
    QDir().mkpath(backupDir);
    writeFile(srcDir    + "/mod.txt", "version 2");
    writeFile(backupDir + "/mod.txt", "version 1");

    QStringList changed = scanner.getChangedFiles({srcDir}, backupDir);
    ASSERT_EQ(changed.size(), 1);
    EXPECT_TRUE(changed[0].endsWith("mod.txt"));
}