#pragma once
#include <QObject>
#include <QString>

class Logger : public QObject {
    Q_OBJECT
public:
    explicit Logger(const QString &logFilePath, QObject *parent = nullptr);
    void info(const QString &message);
    void warn(const QString &message);
    void error(const QString &message);
private:
    QString m_logFilePath;
    void write(const QString &level, const QString &message);
};
