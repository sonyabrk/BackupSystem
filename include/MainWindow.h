#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>

#include "AppSettings.h"
#include "BackupRepository.h"
#include "BackupManager.h"
#include "RestorePoint.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(BackupManager    *manager,
                        AppSettings      *settings,
                        BackupRepository *repo,
                        QWidget          *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onNavBackup();
    void onNavRestore();
    void onNavHistory();
    void onNavSettings();

    void onStartBackup();
    void onPauseBackup();

    void onBrowseRestore();
    void onRestoreSelected();

    void onRefreshHistory();
    void onDeletePoint();

    void onAddPath();
    void onRemovePath();
    void onBrowseDest();
    void onSaveSettings();

    void onProgressChanged(int percent);
    void onStatusChanged(const QString &status);
    void onErrorOccurred(const QString &message);
    void onBackupFinished(const RestorePoint &point);

private:
    void buildUI();
    QWidget* buildSidebar();
    QWidget* buildPageBackup();
    QWidget* buildPageRestore();
    QWidget* buildPageHistory();
    QWidget* buildPageSettings();

    void applyDarkTheme();
    void setupConnections();
    void switchPage(int index, QPushButton *btn);
    void loadSettingsToUI();
    void refreshHistoryTable();
    void refreshRestoreTable();
    void appendLog(const QString &text);
    void setBackupRunning(bool running);
    QString formatBytes(qint64 bytes) const;

    // бэкенд
    AppSettings      *m_appSettings = nullptr;
    BackupRepository *m_repo        = nullptr;
    BackupManager    *m_manager     = nullptr;

    // навигация
    QButtonGroup   *m_navGroup   = nullptr;
    QStackedWidget *m_stack      = nullptr;
    QPushButton    *m_navBtns[4] = {};

    // страница копирования
    QLabel         *m_lblStatus      = nullptr;
    QLabel         *m_lblBackupType  = nullptr;
    QLabel         *m_lblProgressPct = nullptr;
    QProgressBar   *m_progressBar    = nullptr;
    QPlainTextEdit *m_logView        = nullptr;
    QPushButton    *m_btnStart       = nullptr;
    QPushButton    *m_btnPause       = nullptr;
    QLabel         *m_sidebarStatus  = nullptr;

    // страница восстановления
    QLineEdit    *m_restorePath  = nullptr;
    QTableWidget *m_restoreTable = nullptr;

    // страница истории
    QTableWidget *m_historyTable  = nullptr;
    QLabel       *m_lblTotal      = nullptr;
    QLabel       *m_lblFullCount  = nullptr;
    QLabel       *m_lblIncrCount  = nullptr;

    // страница настроек
    QListWidget *m_sourceList = nullptr;
    QLineEdit   *m_destPath   = nullptr;
    QCheckBox   *m_autoSched  = nullptr;
    QSpinBox    *m_interval   = nullptr;

    bool m_backupPaused = false;
};