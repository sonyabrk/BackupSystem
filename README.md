# BackupSystem - Система резервного копирования

**Автор:** Брюханова Софья, ЭФБО-02-24  
**Дисциплина:** Программирование корпоративных систем

---

## О проекте

BackupSystem — десктопное приложение для резервного копирования и восстановления файлов с графическим интерфейсом на Qt.

**Возможности:**
- **Полное резервное копирование** — копирует все выбранные файлы и директории в точку восстановления
- **Инкрементальное копирование** — сохраняет только изменившиеся с последнего бэкапа файлы (на основе контрольных сумм)
- **Восстановление данных** — восстанавливает файлы из любой сохранённой точки восстановления
- **Управление расписанием** — автоматический запуск бэкапов с настраиваемым интервалом
- **История бэкапов** — хранение метаданных всех точек восстановления в SQLite-базе
- **Логирование** — подробный журнал всех операций с уровнями INFO / WARN / ERROR

---

## Стек технологий

| Слой | Технологии |
|------|-----------|
| Язык | C++17 |
| UI-фреймворк | Qt 5.15 / Qt 6 (Core, Gui, Widgets) |
| База данных | SQLite через Qt Sql |
| Многопоточность | QThreadPool, QRunnable |
| Тестирование | Google Test + Google Mock |
| Сборка | CMake 3.10+ |
| Контейнеризация | Docker, Docker Compose |

---

## Структура проекта

```
backup_system/
├── CMakeLists.txt
├── Dockerfile
├── docker-compose.yml
├── run_tests.sh
├── README.md
├── include/                       # Заголовочные файлы
│   ├── AppSettings.h              # Сохранение/загрузка настроек (QSettings)
│   ├── BackupJob.h                # Абстрактный базовый класс задачи (QRunnable)
│   ├── BackupManager.h            # Оркестратор бэкапов, сигналы прогресса
│   ├── BackupRepository.h         # Репозиторий точек восстановления (SQLite)
│   ├── BackupSettings.h           # Структура параметров бэкапа
│   ├── FileScanner.h              # Сканирование файлов и контрольные суммы
│   ├── FullBackupJob.h            # Задача полного бэкапа
│   ├── IncrementalBackupJob.h     # Задача инкрементального бэкапа
│   ├── Logger.h                   # Логгер (INFO / WARN / ERROR)
│   ├── MainWindow.h               # Главное окно приложения
│   └── RestorePoint.h             # Структура точки восстановления
├── src/                           # Реализации
│   ├── main.cpp
│   ├── AppSettings.cpp
│   ├── BackupJob.cpp
│   ├── BackupManager.cpp
│   ├── BackupRepository.cpp
│   ├── FileScanner.cpp
│   ├── FullBackupJob.cpp
│   ├── IncrementalBackupJob.cpp
│   ├── Logger.cpp
│   └── MainWindow.cpp
└── tests/                         # Тесты и сценарии
    ├── main_test.cpp
    ├── test_Logger.cpp
    ├── test_AppSettings.cpp
    ├── test_BackupRepository.cpp
    ├── test_FileScanner.cpp
    ├── test_BackupManager.cpp
    ├── test_FullBackupJob.cpp
    ├── test_IncrementalBackupJob.cpp
    ├── scenario_full_backup.cpp   # Сценарий: полный бэкап
    ├── scenario_incremental.cpp   # Сценарий: инкрементальный бэкап
    └── scenario_restore.cpp       # Сценарий: восстановление
```

---

## Запуск проекта

### macOS

**1. Установка зависимостей (один раз)**
```bash
brew install cmake qt@6
```

**2. Сборка**
```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=$(brew --prefix qt@6)
cmake --build build
```

**3. Запуск приложения**
```bash
./build/backup_system
```

---

### Linux (Ubuntu / Debian)

**1. Установка зависимостей**
```bash
sudo apt update
sudo apt install cmake qt6-base-dev qt6-base-dev-tools libqt6sql6-sqlite
```

> Если Qt 6 недоступен в репозитории вашего дистрибутива, используйте Qt 5:
> ```bash
> sudo apt install cmake qt5-default libqt5sql5-sqlite
> ```

**2. Сборка**
```bash
cmake -S . -B build
cmake --build build
```

**3. Запуск приложения**
```bash
./build/backup_system
```

---

### Windows

**1. Установка зависимостей**

- Установите [CMake](https://cmake.org/download/) (≥ 3.10) и добавьте в `PATH`
- Установите [Qt 6](https://www.qt.io/download-qt-installer) (или Qt 5.15) — выберите компонент **MSVC** или **MinGW** в зависимости от компилятора
- Установите **Visual Studio 2019/2022** (рекомендуется) или **MinGW**

**2. Сборка через командную строку (MSVC)**
```cmd
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build --config Release
```

**3. Сборка через Qt Creator (проще)**

Откройте `CMakeLists.txt` в Qt Creator → нажмите «Собрать» → «Запустить».

**4. Запуск приложения**
```cmd
build\Release\backup_system.exe
```

---

## Запуск тестов

Тесты собираются вместе с проектом (цель `backup_tests`). Google Test скачивается автоматически через CMake FetchContent.

### macOS / Linux

```bash
# Сборка (если ещё не собирали)
cmake -S . -B build
cmake --build build

# Запуск всех тестов
cd build
export QT_QPA_PLATFORM=offscreen   # нужно для headless-окружения
./backup_tests --gtest_color=yes

# Или через CTest
ctest --test-dir build --output-on-failure
```

### Windows

```cmd
cd build
set QT_QPA_PLATFORM=offscreen
.\backup_tests.exe --gtest_color=yes
```

### Сценарные тесты 

Помимо unit-тестов, проект содержит три мини-программы, проверяющие сквозные сценарии:

```bash
./build/scenario_full_backup    # Сценарий 1: полный бэкап
./build/scenario_incremental    # Сценарий 2: инкрементальный бэкап
./build/scenario_restore        # Сценарий 3: восстановление данных
```

---

## Docker

Проект полностью контейнеризован. Все зависимости (Qt, CMake, GTest) устанавливаются внутри образа.

### Требования

- [Docker](https://docs.docker.com/get-docker/) ≥ 20.x
- [Docker Compose](https://docs.docker.com/compose/) ≥ 2.x

### Сборка образа

```bash
docker build -t backup-system:latest .
```

### Запуск тестов через Docker Compose

```bash
docker compose up tests
```

Контейнер соберёт проект, запустит `./run_tests.sh` и выведет результаты в консоль. При успехе завершится с кодом 0.

### Запуск приложения через Docker (offscreen-режим)

```bash
docker compose up app
```

Приложение запускается с `QT_QPA_PLATFORM=offscreen` — без реального дисплея (полезно для CI/CD).

### Запуск отдельным командой (без Compose)

```bash
# Тесты
docker run --rm -e QT_QPA_PLATFORM=offscreen backup-system:latest ./run_tests.sh

# Приложение
docker run --rm -e QT_QPA_PLATFORM=offscreen backup-system:latest ./backup_system
```

### Пересборка образа после изменений

```bash
docker compose build --no-cache
docker compose up tests
```

---

## Покрытие тестами

| Модуль | Тест-файл | Количество тестов |
|--------|-----------|:-----------------:|
| Logger | test_Logger.cpp | 7 |
| AppSettings | test_AppSettings.cpp | 7 |
| BackupRepository | test_BackupRepository.cpp | 7 |
| FileScanner | test_FileScanner.cpp | 7 |
| FullBackupJob | test_FullBackupJob.cpp | 7 |
| IncrementalBackupJob | test_IncrementalBackupJob.cpp | 7 |
| BackupManager | test_BackupManager.cpp | 6 |
| **Итого** | | **48** |