#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "Logger.h"
#include "AppSettings.h"
#include "BackupRepository.h"
#include "BackupManager.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("BackupSystem");
    app.setOrganizationName("RTU-MIREA");

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);

    QString dbPath  = dataDir + "/backup.db";
    QString logPath = dataDir + "/backup.log";

    auto *logger   = new Logger(logPath, &app);
    auto *repo     = new BackupRepository(dbPath, &app);
    repo->initialize();

    auto *settings = new AppSettings(&app);
    auto *manager  = new BackupManager(repo, logger, &app);

    logger->info("Приложение запущено. Данные: " + dataDir);

    MainWindow w(manager, settings, repo);
    w.show();

    return app.exec();
}