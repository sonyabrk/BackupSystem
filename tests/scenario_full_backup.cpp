// Полное резервное копирование с нуля
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

    QTemporaryDir srcDir, dstDir, tmpDir;
    writeFile(srcDir.filePath("doc1.txt"), "Important document 1");
    writeFile(srcDir.filePath("doc2.txt"), "Important document 2");
    writeFile(srcDir.filePath("subdir/doc3.txt"), "Nested document");

    Logger           logger(tmpDir.filePath("scenario.log"));
    BackupRepository repo(tmpDir.filePath("scenario.db"));
    repo.initialize();
    BackupManager manager(&repo, &logger);

    BackupSettings settings;
    settings.sourcePaths     = {srcDir.path()};
    settings.destinationPath = dstDir.path();

    QEventLoop loop;
    QObject::connect(&manager, &BackupManager::statusChanged,
                     [&](const QString &s){
                         qDebug() << "Статус:" << s;
                         if (s == "Завершено") loop.quit();
                     });
    QObject::connect(&manager, &BackupManager::progressChanged,
                     [](int p){ qDebug() << "Прогресс:" << p << "%"; });

    logger.info("Сценарий 1: Полный бэкап");
    manager.startBackup(settings);
    loop.exec();

    RestorePoint rp = repo.getLastFullBackup();
    Q_ASSERT(!rp.path.isEmpty());
    Q_ASSERT(QFileInfo::exists(rp.path + "/doc1.txt"));
    Q_ASSERT(QFileInfo::exists(rp.path + "/doc2.txt"));
    Q_ASSERT(QFileInfo::exists(rp.path + "/subdir/doc3.txt"));
    qDebug() << "Сценарий 1 прошёл успешно. Бэкап:" << rp.path;
    return 0;
}