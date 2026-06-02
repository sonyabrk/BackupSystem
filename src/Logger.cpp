#include "Logger.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

Logger::Logger(const QString &logFilePath, QObject *parent)
    : QObject(parent), m_logFilePath(logFilePath) {}

void Logger::info(const QString &message)  { write("INFO",  message); }
void Logger::warn(const QString &message)  { write("WARN",  message); }
void Logger::error(const QString &message) { write("ERROR", message); }

void Logger::write(const QString &level, const QString &message) {
    QString line = QString("[%1] [%2] %3\n")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
        .arg(level)
        .arg(message);

    qDebug().noquote() << line.trimmed();

    if (m_logFilePath.isEmpty()) return;

    QFile file(m_logFilePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << line;
    }
}