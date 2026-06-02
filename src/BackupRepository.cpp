#include "BackupRepository.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

static QSqlDatabase dbForPath(const QString &path) {
    const QString connName = "backup_conn_" + path;
    if (QSqlDatabase::contains(connName))
        return QSqlDatabase::database(connName);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connName);
    db.setDatabaseName(path);
    return db;
}

BackupRepository::BackupRepository(const QString &dbPath, QObject *parent)
    : QObject(parent), m_dbPath(dbPath) {}

bool BackupRepository::initialize() {
    QSqlDatabase db = dbForPath(m_dbPath);
    if (!db.open()) {
        qWarning() << "BackupRepository: не удалось открыть БД:" << db.lastError().text();
        return false;
    }
    QSqlQuery q(db);
    bool ok = q.exec(
        "CREATE TABLE IF NOT EXISTS restore_points ("
        "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  type      TEXT    NOT NULL,"
        "  timestamp TEXT    NOT NULL,"
        "  sizeBytes INTEGER NOT NULL DEFAULT 0,"
        "  path      TEXT    NOT NULL"
        ");"
    );
    if (!ok)
        qWarning() << "BackupRepository: ошибка создания таблицы:" << q.lastError().text();
    return ok;
}

void BackupRepository::saveRestorePoint(const RestorePoint &point) {
    QSqlDatabase db = dbForPath(m_dbPath);
    QSqlQuery q(db);
    q.prepare(
        "INSERT INTO restore_points (type, timestamp, sizeBytes, path) "
        "VALUES (:type, :ts, :sz, :path);"
    );
    q.bindValue(":type", point.type);
    q.bindValue(":ts",   point.timestamp.toString(Qt::ISODate));
    q.bindValue(":sz",   point.sizeBytes);
    q.bindValue(":path", point.path);
    if (!q.exec())
        qWarning() << "BackupRepository: ошибка INSERT:" << q.lastError().text();
}

QList<RestorePoint> BackupRepository::getAllRestorePoints() const {
    QList<RestorePoint> result;
    QSqlDatabase db = dbForPath(m_dbPath);
    QSqlQuery q(db);
    q.exec("SELECT id, type, timestamp, sizeBytes, path FROM restore_points ORDER BY timestamp DESC;");
    while (q.next()) {
        RestorePoint rp;
        rp.id        = q.value(0).toInt();
        rp.type      = q.value(1).toString();
        rp.timestamp = QDateTime::fromString(q.value(2).toString(), Qt::ISODate);
        rp.sizeBytes = q.value(3).toLongLong();
        rp.path      = q.value(4).toString();
        result.append(rp);
    }
    return result;
}

RestorePoint BackupRepository::getLastFullBackup() const {
    QSqlDatabase db = dbForPath(m_dbPath);
    QSqlQuery q(db);
    q.prepare(
        "SELECT id, type, timestamp, sizeBytes, path FROM restore_points "
        "WHERE type='full' ORDER BY timestamp DESC LIMIT 1;"
    );
    q.exec();
    if (q.next()) {
        RestorePoint rp;
        rp.id        = q.value(0).toInt();
        rp.type      = q.value(1).toString();
        rp.timestamp = QDateTime::fromString(q.value(2).toString(), Qt::ISODate);
        rp.sizeBytes = q.value(3).toLongLong();
        rp.path      = q.value(4).toString();
        return rp;
    }
    return {};
}

void BackupRepository::deleteRestorePoint(int id) {
    QSqlDatabase db = dbForPath(m_dbPath);
    QSqlQuery q(db);
    q.prepare("DELETE FROM restore_points WHERE id = :id;");
    q.bindValue(":id", id);
    if (!q.exec())
        qWarning() << "BackupRepository: ошибка DELETE:" << q.lastError().text();
}
