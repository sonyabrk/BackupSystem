#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QFile>
#include <QDate>
#include <QTemporaryDir>
#include "Logger.h"

// вспомог. класс для чтения лога
static QString readFile(const QString &path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(f.readAll());
}

// info() записывает метку [INFO]
TEST(LoggerTest, InfoWritesInfoTag) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.info("hello");
    EXPECT_TRUE(readFile(dir.filePath("test.log")).contains("[INFO]"));
}

// warn() записывает метку [WARN]
TEST(LoggerTest, WarnWritesWarnTag) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.warn("something fishy");
    EXPECT_TRUE(readFile(dir.filePath("test.log")).contains("[WARN]"));
}

// error() записывает метку [ERROR]
TEST(LoggerTest, ErrorWritesErrorTag) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.error("boom");
    EXPECT_TRUE(readFile(dir.filePath("test.log")).contains("[ERROR]"));
}

// текст сообщения присутствует в файле
TEST(LoggerTest, MessageTextIsPresentInFile) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.info("unique_marker_abc123");
    EXPECT_TRUE(readFile(dir.filePath("test.log")).contains("unique_marker_abc123"));
}

// несколько записей добавляются последовательно
TEST(LoggerTest, MultipleWritesAppend) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.info("first");
    logger.info("second");
    logger.info("third");
    QString content = readFile(dir.filePath("test.log"));
    EXPECT_TRUE(content.contains("first"));
    EXPECT_TRUE(content.contains("second"));
    EXPECT_TRUE(content.contains("third"));
}

// пустой путь к файлу — не падает (только qDebug)
TEST(LoggerTest, EmptyPathDoesNotCrash) {
    Logger logger("");
    EXPECT_NO_THROW(logger.info("no file"));
    EXPECT_NO_THROW(logger.warn("no file"));
    EXPECT_NO_THROW(logger.error("no file"));
}

// каждая запись содержит временну́ю метку в формате YYYY-MM-DD
TEST(LoggerTest, EntryContainsDateTimestamp) {
    QTemporaryDir dir;
    Logger logger(dir.filePath("test.log"));
    logger.info("ts test");
    QString content = readFile(dir.filePath("test.log"));
    // проверка, что строка содержит текущий год
    QString year = QString::number(QDate::currentDate().year());
    EXPECT_TRUE(content.contains(year));
}