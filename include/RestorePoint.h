#pragma once
#include <QString>
#include <QDateTime>

struct RestorePoint {
    int       id = 0;
    QString   type;       // "full" или "incremental"
    QDateTime timestamp;
    qint64    sizeBytes = 0;
    QString   path;
};
