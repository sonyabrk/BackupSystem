// Инкрементальное копирование после изменений
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

static void waitDone(BackupManager *m) {
    QEventLoop loop;
    QObject::connect(m, &BackupManager::statusChanged,
                     [&](const QString &s){ if (s == "Завершено") loop.quit(); });
    loop.exec();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QTemporaryDir srcDir, dstDir, tmpDir;
    writeFile(srcDir.filePath("a.txt"), "version 1");
    writeFile(srcDir.filePath("b.txt"), "unchanged");

    Logger           logger(tmpDir.filePath("scenario.log"));
    BackupRepository repo(tmpDir.filePath("scenario.db"));
    repo.initialize();
    BackupManager manager(&repo, &logger);

    BackupSettings settings;
    settings.sourcePaths     = {srcDir.path()};
    settings.destinationPath = dstDir.path();

    logger.info("Шаг 1: Полный бэкап");
    manager.startBackup(settings);
    waitDone(&manager);

    writeFile(srcDir.filePath("a.txt"), "version 2");

    logger.info("Шаг 2: Инкрементальный бэкап");
    manager.startBackup(settings);
    waitDone(&manager);

    QList<RestorePoint> all = repo.getAllRestorePoints();
    Q_ASSERT(all.size() == 2);

    // последняя (первая в отсортированном списке) — incremental
    Q_ASSERT(all[0].type == "incremental");
    Q_ASSERT(QFileInfo::exists(all[0].path + "/a.txt"));
    // b.txt не должен быть в инкрементальной копии
    Q_ASSERT(!QFileInfo::exists(all[0].path + "/b.txt"));

    qDebug() << "Сценарий 2 прошёл успешно.";
    return 0;
}