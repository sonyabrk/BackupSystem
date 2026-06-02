// Восстановление данных из точки восстановления
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QDebug>
#include "Logger.h"
#include "BackupRepository.h"
#include "BackupManager.h"
#include "AppSettings.h"

static void writeFile(const QString &path, const QString &content) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream(&f) << content;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTemporaryDir srcDir, dstDir, restoreDir, tmpDir;
    writeFile(srcDir.filePath("important.txt"), "critical data");

    Logger           logger(tmpDir.filePath("scenario.log"));
    BackupRepository repo(tmpDir.filePath("scenario.db"));
    repo.initialize();
    BackupManager manager(&repo, &logger);

    BackupSettings settings;
    settings.sourcePaths     = {srcDir.path()};
    settings.destinationPath = dstDir.path();

    QEventLoop loop;
    QObject::connect(&manager, &BackupManager::statusChanged,
                     [&](const QString &s){ if (s == "Завершено") loop.quit(); });
    manager.startBackup(settings);
    loop.exec();

    RestorePoint rp = repo.getLastFullBackup();
    Q_ASSERT(!rp.path.isEmpty());

    // Восстанавливаем в новую директорию
    logger.info("Сценарий 3: Восстановление");
    manager.restoreFromPoint(rp, restoreDir.path());

    Q_ASSERT(QFileInfo::exists(restoreDir.filePath("important.txt")));

    QFile f(restoreDir.filePath("important.txt"));
    f.open(QIODevice::ReadOnly | QIODevice::Text);
    Q_ASSERT(QString::fromUtf8(f.readAll()).trimmed() == "critical data");

    qDebug() << "Сценарий 3 прошёл успешно.";
    return 0;
}