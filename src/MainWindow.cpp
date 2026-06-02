#include "MainWindow.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTableWidgetItem>
#include <QSizePolicy>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include <QAction>


static QLabel *makeLabel(const QString &text, const QString &style = {}) {
    auto *l = new QLabel(text);
    if (!style.isEmpty()) l->setStyleSheet(style);
    return l;
}

static QFrame *makeCard() {
    auto *f = new QFrame;
    f->setObjectName("card");
    return f;
}

static QFrame *makeHLine() {
    auto *f = new QFrame;
    f->setFrameShape(QFrame::HLine);
    f->setObjectName("hline");
    return f;
}


MainWindow::MainWindow(BackupManager *manager,
                       AppSettings   *settings,
                       BackupRepository *repo,
                       QWidget *parent)
    : QMainWindow(parent)
{
    m_manager     = manager;
    m_appSettings = settings;
    m_repo        = repo;

    setWindowTitle("BackupSystem");
    resize(1100, 720);

    applyDarkTheme();
    buildUI();
    setupConnections();
    loadSettingsToUI();
    refreshHistoryTable();
    refreshRestoreTable();

    appendLog("// Система резервного копирования запущена.");
    statusBar()->showMessage("Готово");
}


void MainWindow::applyDarkTheme() {
    qApp->setStyleSheet(R"(

QMainWindow, QWidget {
    background-color: #0E1117;
    color: #C9D1D9;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 13px;
}

QWidget#sidebar {
    background-color: #161B22;
    border-right: 1px solid #21262D;
}

QFrame#card {
    background-color: #161B22;
    border: 1px solid #21262D;
    border-radius: 6px;
}

QPushButton#nav {
    background-color: transparent;
    color: #8B949E;
    border: none;
    border-left: 3px solid transparent;
    text-align: left;
    padding: 10px 16px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QPushButton#nav:hover {
    background-color: #1C2128;
    color: #C9D1D9;
    border-left: 3px solid #30363D;
}
QPushButton#nav:checked {
    background-color: #1C2128;
    color: #00D4AA;
    border-left: 3px solid #00D4AA;
}

QPushButton#primary {
    background-color: #00D4AA;
    color: #0E1117;
    border: none;
    border-radius: 4px;
    padding: 10px 24px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
    font-weight: bold;
}
QPushButton#primary:hover   { background-color: #00EFC0; }
QPushButton#primary:pressed { background-color: #00B891; }
QPushButton#primary:disabled { background-color: #21262D; color: #484F58; }

QPushButton#secondary {
    background-color: #21262D;
    color: #C9D1D9;
    border: 1px solid #30363D;
    border-radius: 4px;
    padding: 7px 14px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 11px;
}
QPushButton#secondary:hover { background-color: #2D333B; }

QPushButton#warn {
    background-color: transparent;
    color: #F0B429;
    border: 1px solid #F0B429;
    border-radius: 4px;
    padding: 9px 18px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QPushButton#warn:hover    { background-color: rgba(240,180,41,0.08); }
QPushButton#warn:disabled { color: #484F58; border-color: #30363D; }

QPushButton#danger {
    background-color: transparent;
    color: #F85149;
    border: 1px solid #F85149;
    border-radius: 4px;
    padding: 7px 14px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 11px;
}
QPushButton#danger:hover { background-color: rgba(248,81,73,0.08); }

QPushButton#blue {
    background-color: transparent;
    color: #58A6FF;
    border: 1px solid #58A6FF;
    border-radius: 4px;
    padding: 7px 14px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 11px;
}
QPushButton#blue:hover    { background-color: rgba(88,166,255,0.08); }
QPushButton#blue:disabled { color: #484F58; border-color: #30363D; }

QProgressBar {
    background-color: #21262D;
    border: none;
    border-radius: 3px;
    max-height: 6px;
    text-align: center;
    color: transparent;
}
QProgressBar::chunk { background-color: #00D4AA; border-radius: 3px; }

QPlainTextEdit#log {
    background-color: #0D1117;
    color: #7EE787;
    border: 1px solid #21262D;
    border-radius: 4px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 11px;
    padding: 8px;
}

QTableWidget {
    background-color: #161B22;
    color: #C9D1D9;
    border: 1px solid #21262D;
    border-radius: 4px;
    gridline-color: #21262D;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
    selection-background-color: #1C2128;
    selection-color: #E6EDF3;
    outline: none;
}
QTableWidget::item       { padding: 6px 10px; border: none; }
QTableWidget::item:hover { background-color: #1C2128; }
QHeaderView::section {
    background-color: #21262D;
    color: #8B949E;
    border: none;
    border-bottom: 1px solid #30363D;
    padding: 6px 10px;
    font-size: 10px;
    letter-spacing: 1px;
}

QLineEdit, QSpinBox {
    background-color: #161B22;
    color: #E6EDF3;
    border: 1px solid #30363D;
    border-radius: 4px;
    padding: 6px 10px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QLineEdit:focus, QSpinBox:focus { border-color: #00D4AA; }

QListWidget {
    background-color: #0D1117;
    color: #C9D1D9;
    border: 1px solid #21262D;
    border-radius: 4px;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QListWidget::item          { padding: 5px 10px; }
QListWidget::item:selected { background-color: #1C2128; color: #00D4AA; }

QCheckBox { color: #C9D1D9; spacing: 8px; }
QCheckBox::indicator {
    width: 16px; height: 16px;
    border: 1px solid #30363D;
    border-radius: 3px;
    background: #161B22;
}
QCheckBox::indicator:checked { background: #00D4AA; border-color: #00D4AA; }

QScrollBar:vertical {
    background: transparent; width: 6px; margin: 0;
}
QScrollBar::handle:vertical {
    background: #30363D; border-radius: 3px; min-height: 20px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }

QFrame#hline { color: #21262D; background-color: #21262D; max-height: 1px; }

QStatusBar {
    background-color: #161B22;
    color: #484F58;
    border-top: 1px solid #21262D;
    font-size: 11px;
    font-family: "Fira Code", "Courier New", monospace;
}

QMenuBar {
    background-color: #161B22;
    color: #8B949E;
    border-bottom: 1px solid #21262D;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QMenuBar::item:selected { background: #21262D; color: #C9D1D9; }
QMenu {
    background: #161B22; color: #C9D1D9;
    border: 1px solid #30363D;
    font-family: "Fira Code", "Courier New", monospace;
    font-size: 12px;
}
QMenu::item:selected { background: #21262D; }

    )");
}


void MainWindow::buildUI() {
    auto *central = new QWidget(this);
    setCentralWidget(central);

    auto *root = new QHBoxLayout(central);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    root->addWidget(buildSidebar());

    m_stack = new QStackedWidget;
    m_stack->addWidget(buildPageBackup());
    m_stack->addWidget(buildPageRestore());
    m_stack->addWidget(buildPageHistory());
    m_stack->addWidget(buildPageSettings());
    m_stack->setCurrentIndex(0);
    root->addWidget(m_stack, 1);

    auto *mb    = menuBar();
    auto *mFile = mb->addMenu("Файл");
    auto *mBack = mb->addMenu("Резервная копия");
    auto *mHelp = mb->addMenu("Справка");

    auto *aQuit = mFile->addAction("Выход");
    aQuit->setShortcut(QKeySequence("Ctrl+Q"));
    connect(aQuit, &QAction::triggered, this, &QMainWindow::close);

    auto *aStart = mBack->addAction("Запустить копирование");
    aStart->setShortcut(QKeySequence("Ctrl+B"));
    connect(aStart, &QAction::triggered, this, &MainWindow::onStartBackup);

    auto *aAbout = mHelp->addAction("О программе");
    connect(aAbout, &QAction::triggered, this, [this] {
        QMessageBox::about(this, "О программе",
                           "<b>BackupSystem</b>"
                           "Система резервного копирования.<br>Собрано на Qt.");
    });
}

QWidget *MainWindow::buildSidebar() {
    auto *w = new QWidget;
    w->setObjectName("sidebar");
    w->setFixedWidth(200);

    auto *lay = new QVBoxLayout(w);
    lay->setSpacing(0);
    lay->setContentsMargins(0, 0, 0, 0);

    auto *logoBlock = new QWidget;
    auto *logoLay   = new QVBoxLayout(logoBlock);
    logoLay->setSpacing(2);
    logoLay->setContentsMargins(16, 18, 16, 14);
    logoLay->addWidget(makeLabel("BACKUP SYS",
                                 "color:#00D4AA; font-size:15px; font-weight:bold; letter-spacing:2px;"));
    logoLay->addWidget(makeLabel("РТУ МИРЭА",
                                 "color:#484F58; font-size:10px; letter-spacing:1px;"));
    lay->addWidget(logoBlock);
    lay->addWidget(makeHLine());

    const char *labels[4] = {
        "  \xe2\x96\xb6  \xd0\x9a\xd0\x9e\xd0\x9f\xd0\x98\xd0\xa0\xd0\x9e\xd0\x92\xd0\x90\xd0\x9d\xd0\x98\xd0\x95",
        "  \xe2\x86\xa9  \xd0\x92\xd0\x9e\xd0\xa1\xd0\xa1\xd0\xa2\xd0\x90\xd0\x9d\xd0\x9e\xd0\x92\xd0\x9b\xd0\x95\xd0\x9d\xd0\x98\xd0\x95",
        "  \xe2\x89\xa1  \xd0\x98\xd0\xa1\xd0\xa2\xd0\x9e\xd0\xa0\xd0\x98\xd0\xaf",
        "  \xe2\x9c\xa6  \xd0\x9d\xd0\x90\xd0\xa1\xd0\xa2\xd0\xa0\xd0\x9e\xd0\x99\xd0\x9a\xd0\x98"
    };

    m_navGroup = new QButtonGroup(this);
    auto *navBlock = new QWidget;
    auto *navLay   = new QVBoxLayout(navBlock);
    navLay->setSpacing(2);
    navLay->setContentsMargins(0, 10, 0, 0);

    for (int i = 0; i < 4; ++i) {
        auto *btn = new QPushButton(QString::fromUtf8(labels[i]));
        btn->setObjectName("nav");
        btn->setCheckable(true);
        btn->setChecked(i == 0);
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_navGroup->addButton(btn, i);
        navLay->addWidget(btn);
        m_navBtns[i] = btn;
    }
    lay->addWidget(navBlock);
    lay->addStretch();

    m_sidebarStatus = makeLabel("● ОЖИДАНИЕ", "color:#484F58; font-size:11px;");
    auto *stBlock = new QWidget;
    auto *stLay   = new QVBoxLayout(stBlock);
    stLay->setContentsMargins(16, 8, 0, 14);
    stLay->addWidget(m_sidebarStatus);
    lay->addWidget(stBlock);

    return w;
}

QWidget *MainWindow::buildPageBackup() {
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setSpacing(16);
    lay->setContentsMargins(28, 28, 28, 20);

    auto *hdr     = new QHBoxLayout;
    auto *hdrLeft = new QVBoxLayout;
    hdrLeft->setSpacing(2);
    hdrLeft->addWidget(makeLabel("Новая копия",
                                 "color:#E6EDF3; font-size:18px; font-weight:bold;"));
    hdrLeft->addWidget(makeLabel("резервная копия / создать",
                                 "color:#484F58; font-size:11px;"));
    hdr->addLayout(hdrLeft);
    hdr->addStretch();

    m_btnPause = new QPushButton("⏸  ПАУЗА");
    m_btnPause->setObjectName("warn");
    m_btnPause->setEnabled(false);
    hdr->addWidget(m_btnPause);

    m_btnStart = new QPushButton("▶  ЗАПУСТИТЬ");
    m_btnStart->setObjectName("primary");
    hdr->addWidget(m_btnStart);
    lay->addLayout(hdr);

    // Stat cards
    auto *cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(12);

    auto *cStatus = makeCard();
    auto *csLay   = new QVBoxLayout(cStatus);
    csLay->setContentsMargins(16, 14, 16, 14);
    csLay->addWidget(makeLabel("СТАТУС",
                               "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    m_lblStatus = makeLabel("ОЖИДАНИЕ", "color:#484F58; font-size:20px; font-weight:bold;");
    csLay->addWidget(m_lblStatus);
    cardsRow->addWidget(cStatus);

    auto *cType = makeCard();
    auto *ctLay = new QVBoxLayout(cType);
    ctLay->setContentsMargins(16, 14, 16, 14);
    ctLay->addWidget(makeLabel("ТИП",
                               "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    m_lblBackupType = makeLabel("—", "color:#00D4AA; font-size:20px; font-weight:bold;");
    ctLay->addWidget(m_lblBackupType);
    cardsRow->addWidget(cType);

    auto *cProg = makeCard();
    auto *cpLay = new QVBoxLayout(cProg);
    cpLay->setContentsMargins(16, 14, 16, 14);
    cpLay->addWidget(makeLabel("ПРОГРЕСС",
                               "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    m_lblProgressPct = makeLabel("0%", "color:#E6EDF3; font-size:20px; font-weight:bold;");
    cpLay->addWidget(m_lblProgressPct);
    m_progressBar = new QProgressBar;
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(false);
    cpLay->addWidget(m_progressBar);
    cardsRow->addWidget(cProg, 1);

    lay->addLayout(cardsRow);

    lay->addWidget(makeLabel("ЛОГ ОПЕРАЦИЙ",
                             "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    m_logView = new QPlainTextEdit;
    m_logView->setObjectName("log");
    m_logView->setReadOnly(true);
    m_logView->setPlaceholderText("// Здесь появится вывод...");
    lay->addWidget(m_logView, 1);

    return page;
}

QWidget *MainWindow::buildPageRestore() {
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setSpacing(16);
    lay->setContentsMargins(28, 28, 28, 20);

    auto *hdrLay = new QVBoxLayout;
    hdrLay->setSpacing(2);
    hdrLay->addWidget(makeLabel("Восстановление",
                                "color:#E6EDF3; font-size:18px; font-weight:bold;"));
    hdrLay->addWidget(makeLabel("резервная копия / восстановление",
                                "color:#484F58; font-size:11px;"));
    lay->addLayout(hdrLay);

    auto *cTarget = makeCard();
    auto *ctLay   = new QVBoxLayout(cTarget);
    ctLay->setContentsMargins(16, 14, 16, 14);
    ctLay->addWidget(makeLabel("ПУТЬ ДЛЯ ВОССТАНОВЛЕНИЯ",
                               "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));

    auto *pathRow = new QHBoxLayout;
    m_restorePath = new QLineEdit;
    m_restorePath->setPlaceholderText("/путь/для/восстановления");
    pathRow->addWidget(m_restorePath, 1);

    auto *btnBrowse = new QPushButton("Обзор…");
    btnBrowse->setObjectName("secondary");
    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::onBrowseRestore);
    pathRow->addWidget(btnBrowse);

    auto *btnRestore = new QPushButton("↩  ВОССТАНОВИТЬ");
    btnRestore->setObjectName("blue");
    connect(btnRestore, &QPushButton::clicked, this, &MainWindow::onRestoreSelected);
    pathRow->addWidget(btnRestore);

    ctLay->addLayout(pathRow);
    lay->addWidget(cTarget);

    lay->addWidget(makeLabel("ВЫБЕРИТЕ ТОЧКУ ВОССТАНОВЛЕНИЯ",
                             "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));

    m_restoreTable = new QTableWidget(0, 4);
    m_restoreTable->setHorizontalHeaderLabels({"ТИП", "ДАТА И ВРЕМЯ", "РАЗМЕР", "ПУТЬ"});
    m_restoreTable->horizontalHeader()->setStretchLastSection(true);
    m_restoreTable->verticalHeader()->setVisible(false);
    m_restoreTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_restoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lay->addWidget(m_restoreTable, 1);

    return page;
}

QWidget *MainWindow::buildPageHistory() {
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setSpacing(16);
    lay->setContentsMargins(28, 28, 28, 20);

    auto *hdr     = new QHBoxLayout;
    auto *hdrLeft = new QVBoxLayout;
    hdrLeft->setSpacing(2);
    hdrLeft->addWidget(makeLabel("История",
                                 "color:#E6EDF3; font-size:18px; font-weight:bold;"));
    hdrLeft->addWidget(makeLabel("резервная копия / история",
                                 "color:#484F58; font-size:11px;"));
    hdr->addLayout(hdrLeft);
    hdr->addStretch();

    auto *btnRefresh = new QPushButton("↻  ОБНОВИТЬ");
    btnRefresh->setObjectName("secondary");
    connect(btnRefresh, &QPushButton::clicked, this, &MainWindow::onRefreshHistory);
    hdr->addWidget(btnRefresh);

    auto *btnDelete = new QPushButton("✕  УДАЛИТЬ");
    btnDelete->setObjectName("danger");
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDeletePoint);
    hdr->addWidget(btnDelete);
    lay->addLayout(hdr);

    auto *cardsRow = new QHBoxLayout;
    cardsRow->setSpacing(12);

    auto makeStatCard = [&](const QString &title, QLabel *&valueLabel,
                            const QString &color) {
        auto *c  = makeCard();
        c->setMinimumHeight(80);
        auto *cl = new QVBoxLayout(c);
        cl->setContentsMargins(16, 14, 16, 14);
        cl->addWidget(makeLabel(title,
                                "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
        valueLabel = makeLabel("0",
                               QString("color:%1; font-size:22px; font-weight:bold;").arg(color));
        cl->addWidget(valueLabel);
        cardsRow->addWidget(c);
    };

    makeStatCard("ВСЕГО КОПИЙ",       m_lblTotal,      "#E6EDF3");
    makeStatCard("ПОЛНЫЕ",            m_lblFullCount,  "#00D4AA");
    makeStatCard("ИНКРЕМЕНТАЛЬНЫЕ",   m_lblIncrCount,  "#58A6FF");
    lay->addLayout(cardsRow);

    lay->addWidget(makeLabel("ВСЕ ТОЧКИ ВОССТАНОВЛЕНИЯ",
                             "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));

    m_historyTable = new QTableWidget(0, 5);
    m_historyTable->setHorizontalHeaderLabels({"ID", "ТИП", "ДАТА И ВРЕМЯ", "РАЗМЕР", "ПУТЬ"});
    m_historyTable->horizontalHeader()->setStretchLastSection(true);
    m_historyTable->verticalHeader()->setVisible(false);
    m_historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    lay->addWidget(m_historyTable, 1);

    return page;
}

QWidget *MainWindow::buildPageSettings() {
    auto *page = new QWidget;
    auto *lay  = new QVBoxLayout(page);
    lay->setSpacing(16);
    lay->setContentsMargins(28, 28, 28, 20);

    auto *hdrLay = new QVBoxLayout;
    hdrLay->setSpacing(2);
    hdrLay->addWidget(makeLabel("Настройки",
                                "color:#E6EDF3; font-size:18px; font-weight:bold;"));
    hdrLay->addWidget(makeLabel("резервная копия / настройки",
                                "color:#484F58; font-size:11px;"));
    lay->addLayout(hdrLay);

    auto *cSrc    = makeCard();
    auto *cSrcLay = new QVBoxLayout(cSrc);
    cSrcLay->setContentsMargins(16, 14, 16, 14);
    cSrcLay->addWidget(makeLabel("ИСХОДНЫЕ ПАПКИ",
                                 "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    m_sourceList = new QListWidget;
    m_sourceList->setMaximumHeight(130);
    cSrcLay->addWidget(m_sourceList);
    auto *srcBtnRow = new QHBoxLayout;
    auto *btnAdd    = new QPushButton("+ Добавить");
    btnAdd->setObjectName("secondary");
    connect(btnAdd, &QPushButton::clicked, this, &MainWindow::onAddPath);
    auto *btnRem = new QPushButton("− Удалить");
    btnRem->setObjectName("secondary");
    connect(btnRem, &QPushButton::clicked, this, &MainWindow::onRemovePath);
    srcBtnRow->addWidget(btnAdd);
    srcBtnRow->addWidget(btnRem);
    srcBtnRow->addStretch();
    cSrcLay->addLayout(srcBtnRow);
    lay->addWidget(cSrc);

    // Destination
    auto *cDest    = makeCard();
    auto *cDestLay = new QVBoxLayout(cDest);
    cDestLay->setContentsMargins(16, 14, 16, 14);
    cDestLay->addWidget(makeLabel("ПАПКА НАЗНАЧЕНИЯ",
                                  "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    auto *destRow = new QHBoxLayout;
    m_destPath = new QLineEdit;
    m_destPath->setPlaceholderText("/путь/к/хранилищу");
    destRow->addWidget(m_destPath, 1);
    auto *btnBrowse = new QPushButton("Обзор…");
    btnBrowse->setObjectName("secondary");
    connect(btnBrowse, &QPushButton::clicked, this, &MainWindow::onBrowseDest);
    destRow->addWidget(btnBrowse);
    cDestLay->addLayout(destRow);
    lay->addWidget(cDest);

    // Schedule
    auto *cSched    = makeCard();
    auto *cSchedLay = new QVBoxLayout(cSched);
    cSchedLay->setContentsMargins(16, 14, 16, 14);
    cSchedLay->addWidget(makeLabel("РАСПИСАНИЕ",
                                   "color:#8B949E; font-size:10px; letter-spacing:1.5px; font-weight:bold;"));
    auto *schedRow = new QHBoxLayout;
    m_autoSched = new QCheckBox("Включить автоматическое копирование");
    schedRow->addWidget(m_autoSched);
    schedRow->addStretch();
    schedRow->addWidget(makeLabel("Интервал (дней):", "color:#8B949E;"));
    m_interval = new QSpinBox;
    m_interval->setRange(1, 365);
    m_interval->setValue(1);
    m_interval->setFixedWidth(80);
    schedRow->addWidget(m_interval);
    cSchedLay->addLayout(schedRow);
    lay->addWidget(cSched);

    lay->addStretch();

    auto *saveBtnRow = new QHBoxLayout;
    saveBtnRow->addStretch();
    auto *btnSave = new QPushButton("СОХРАНИТЬ");
    btnSave->setObjectName("primary");
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::onSaveSettings);
    saveBtnRow->addWidget(btnSave);
    lay->addLayout(saveBtnRow);

    return page;
}


void MainWindow::setupConnections() {
    connect(m_navBtns[0], &QPushButton::clicked, this, &MainWindow::onNavBackup);
    connect(m_navBtns[1], &QPushButton::clicked, this, &MainWindow::onNavRestore);
    connect(m_navBtns[2], &QPushButton::clicked, this, &MainWindow::onNavHistory);
    connect(m_navBtns[3], &QPushButton::clicked, this, &MainWindow::onNavSettings);

    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStartBackup);
    connect(m_btnPause, &QPushButton::clicked, this, &MainWindow::onPauseBackup);

    connect(m_manager, &BackupManager::progressChanged, this, &MainWindow::onProgressChanged);
    connect(m_manager, &BackupManager::statusChanged,   this, &MainWindow::onStatusChanged);
    connect(m_manager, &BackupManager::errorOccurred,   this, &MainWindow::onErrorOccurred);
    connect(m_manager, &BackupManager::backupFinished,  this, &MainWindow::onBackupFinished);
}


void MainWindow::switchPage(int index, QPushButton *btn) {
    for (int i = 0; i < 4; ++i) m_navBtns[i]->setChecked(false);
    btn->setChecked(true);
    m_stack->setCurrentIndex(index);
}

void MainWindow::onNavBackup()   { switchPage(0, m_navBtns[0]); }
void MainWindow::onNavRestore()  { refreshRestoreTable();  switchPage(1, m_navBtns[1]); }
void MainWindow::onNavHistory()  { refreshHistoryTable();  switchPage(2, m_navBtns[2]); }
void MainWindow::onNavSettings() { loadSettingsToUI();     switchPage(3, m_navBtns[3]); }


void MainWindow::onStartBackup() {
    BackupSettings s = m_appSettings->load();
    if (s.sourcePaths.isEmpty()) {
        QMessageBox::warning(this, "Нет исходных папок",
                             "Укажите хотя бы одну исходную папку в Настройках.");
        switchPage(3, m_navBtns[3]); return;
    }
    if (s.destinationPath.isEmpty()) {
        QMessageBox::warning(this, "Нет папки назначения",
                             "Укажите папку назначения в Настройках.");
        switchPage(3, m_navBtns[3]); return;
    }
    m_progressBar->setValue(0);
    m_lblProgressPct->setText("0%");
    setBackupRunning(true);
    appendLog("Запуск копирования...");
    m_manager->startBackup(s);
}

void MainWindow::onPauseBackup() {
    if (!m_backupPaused) {
        m_manager->pauseBackup();
        m_btnPause->setText("▶  ПРОДОЛЖИТЬ");
        m_backupPaused = true;
    } else {
        m_manager->resumeBackup();
        m_btnPause->setText("⏸  ПАУЗА");
        m_backupPaused = false;
    }
}


void MainWindow::onBrowseRestore() {
    QString d = QFileDialog::getExistingDirectory(this, "Выбор папки для восстановления");
    if (!d.isEmpty()) m_restorePath->setText(d);
}

void MainWindow::onRestoreSelected() {
    int row = m_restoreTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Ничего не выбрано",
                                 "Выберите точку восстановления.");
        return;
    }
    QString target = m_restorePath->text().trimmed();
    if (target.isEmpty()) {
        QMessageBox::warning(this, "Нет пути",
                             "Укажите путь для восстановления.");
        return;
    }
    RestorePoint rp;
    rp.id   = m_restoreTable->item(row, 0)->data(Qt::UserRole).toInt();
    rp.type = m_restoreTable->item(row, 0)->text().toLower();
    rp.path = m_restoreTable->item(row, 3)->text();

    if (QMessageBox::question(this, "Подтверждение",
                              QString("Восстановить\n  %1\nв\n  %2?").arg(rp.path, target))
        != QMessageBox::Yes) return;

    appendLog("Восстановление: " + rp.path + " → " + target);
    m_manager->restoreFromPoint(rp, target);
    appendLog("Восстановление завершено.");
    statusBar()->showMessage("Восстановление завершено.", 5000);
    QMessageBox::information(this, "Готово", "Восстановление выполнено успешно.");
}


void MainWindow::onRefreshHistory() {
    refreshHistoryTable();
    refreshRestoreTable();
    statusBar()->showMessage("Обновлено.", 3000);
}

void MainWindow::onDeletePoint() {
    int row = m_historyTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Ничего не выбрано", "Выберите строку.");
        return;
    }
    int id = m_historyTable->item(row, 0)->text().toInt();
    if (QMessageBox::question(this, "Подтверждение",
                              QString("Удалить запись #%1 из базы данных?").arg(id))
        != QMessageBox::Yes) return;
    m_repo->deleteRestorePoint(id);
    appendLog(QString("Запись #%1 удалена.").arg(id));
    refreshHistoryTable();
    refreshRestoreTable();
}


void MainWindow::onAddPath() {
    QString d = QFileDialog::getExistingDirectory(this, "Добавить исходную папку");
    if (!d.isEmpty()) m_sourceList->addItem(d);
}

void MainWindow::onRemovePath() {
    int row = m_sourceList->currentRow();
    if (row >= 0) delete m_sourceList->takeItem(row);
}

void MainWindow::onBrowseDest() {
    QString d = QFileDialog::getExistingDirectory(this, "Выбор папки назначения");
    if (!d.isEmpty()) m_destPath->setText(d);
}

void MainWindow::onSaveSettings() {
    BackupSettings s;
    for (int i = 0; i < m_sourceList->count(); ++i)
        s.sourcePaths << m_sourceList->item(i)->text();
    s.destinationPath      = m_destPath->text().trimmed();
    s.autoScheduleEnabled  = m_autoSched->isChecked();
    s.scheduleIntervalDays = m_interval->value();
    m_appSettings->save(s);
    appendLog("Настройки сохранены.");
    statusBar()->showMessage("Настройки сохранены.", 3000);
    QMessageBox::information(this, "Сохранено", "Настройки успешно сохранены.");
}

void MainWindow::onProgressChanged(int p) {
    m_progressBar->setValue(p);
    m_lblProgressPct->setText(QString::number(p) + "%");
}

void MainWindow::onStatusChanged(const QString &s) {
    m_lblStatus->setText(s.toUpper());
    appendLog(s);
    statusBar()->showMessage(s);

    bool done = s.contains("Завершено") || s.contains("finished", Qt::CaseInsensitive);
    if (done) {
        setBackupRunning(false);
        m_backupPaused = false;
        m_btnPause->setText("⏸  ПАУЗА");
        statusBar()->showMessage("Копирование завершено.", 5000);
    }
}

void MainWindow::onErrorOccurred(const QString &msg) {
    setBackupRunning(false);
    m_lblStatus->setText("ОШИБКА");
    m_lblStatus->setStyleSheet("color:#F85149; font-size:20px; font-weight:bold;");
    appendLog("[ОШИБКА] " + msg);
    statusBar()->showMessage("Ошибка: " + msg);
    QMessageBox::critical(this, "Ошибка копирования", msg);
}


void MainWindow::setBackupRunning(bool running) {
    m_btnStart->setEnabled(!running);
    m_btnPause->setEnabled(running);
    m_sidebarStatus->setText(running ? "● ВЫПОЛНЯЕТСЯ" : "● ОЖИДАНИЕ");
    m_sidebarStatus->setStyleSheet(running
                                       ? "color:#00D4AA; font-size:11px;"
                                       : "color:#484F58; font-size:11px;");
}

void MainWindow::loadSettingsToUI() {
    BackupSettings s = m_appSettings->load();
    m_sourceList->clear();
    for (const QString &p : qAsConst(s.sourcePaths))
        m_sourceList->addItem(p);
    m_destPath->setText(s.destinationPath);
    m_autoSched->setChecked(s.autoScheduleEnabled);
    m_interval->setValue(s.scheduleIntervalDays);
}

void MainWindow::refreshHistoryTable() {
    auto points = m_repo->getAllRestorePoints();
    m_historyTable->setRowCount(0);
    int full = 0, incr = 0;
    for (const RestorePoint &rp : qAsConst(points)) {
        int row = m_historyTable->rowCount();
        m_historyTable->insertRow(row);
        auto *tt = new QTableWidgetItem(rp.type.toUpper());
        tt->setForeground(rp.type == "full"
                              ? QColor(0, 212, 170) : QColor(88, 166, 255));
        m_historyTable->setItem(row, 0, new QTableWidgetItem(QString::number(rp.id)));
        m_historyTable->setItem(row, 1, tt);
        m_historyTable->setItem(row, 2, new QTableWidgetItem(
                                            rp.timestamp.toString("yyyy-MM-dd HH:mm:ss")));
        m_historyTable->setItem(row, 3, new QTableWidgetItem(formatBytes(rp.sizeBytes)));
        m_historyTable->setItem(row, 4, new QTableWidgetItem(rp.path));
        rp.type == "full" ? ++full : ++incr;
    }
    m_lblTotal->setText(QString::number(points.size()));
    m_lblFullCount->setText(QString::number(full));
    m_lblIncrCount->setText(QString::number(incr));
}

void MainWindow::refreshRestoreTable() {
    auto points = m_repo->getAllRestorePoints();
    m_restoreTable->setRowCount(0);
    for (const RestorePoint &rp : qAsConst(points)) {
        int row = m_restoreTable->rowCount();
        m_restoreTable->insertRow(row);
        auto *tt = new QTableWidgetItem(rp.type.toUpper());
        tt->setForeground(rp.type == "full"
                              ? QColor(0, 212, 170) : QColor(88, 166, 255));
        tt->setData(Qt::UserRole, rp.id);
        m_restoreTable->setItem(row, 0, tt);
        m_restoreTable->setItem(row, 1, new QTableWidgetItem(
                                            rp.timestamp.toString("yyyy-MM-dd HH:mm:ss")));
        m_restoreTable->setItem(row, 2, new QTableWidgetItem(formatBytes(rp.sizeBytes)));
        m_restoreTable->setItem(row, 3, new QTableWidgetItem(rp.path));
    }
}

void MainWindow::appendLog(const QString &text) {
    m_logView->appendPlainText(
        QString("[%1]  %2").arg(
            QDateTime::currentDateTime().toString("HH:mm:ss"), text));
}

void MainWindow::onBackupFinished(const RestorePoint &point) {
    if (point.type == "full") {
        m_lblBackupType->setText("FULL");
        m_lblBackupType->setStyleSheet(
            "color:#00D4AA; font-size:20px; font-weight:bold;");
    } else {
        m_lblBackupType->setText("INCREMENTAL");
        m_lblBackupType->setStyleSheet(
            "color:#58A6FF; font-size:20px; font-weight:bold;");
    }

    refreshHistoryTable();
    refreshRestoreTable();
}

QString MainWindow::formatBytes(qint64 b) const {
    if (b <= 0)    return "—";
    if (b < 1024)  return QString::number(b) + " Б";
    if (b < 1<<20) return QString::number(b / 1024.0, 'f', 1) + " КБ";
    if (b < 1<<30) return QString::number(b / (1024.0 * 1024), 'f', 2) + " МБ";
    return QString::number(b / (1024.0 * 1024 * 1024), 'f', 2) + " ГБ";
}