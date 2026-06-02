cmake_minimum_required(VERSION 3.10)
project(BackupSystem VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

find_package(Qt6 COMPONENTS Core Gui Widgets Sql QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 COMPONENTS Core Gui Widgets Sql REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# основное приложение
set(APP_SOURCES
    src/main.cpp
    src/Logger.cpp
    src/AppSettings.cpp
    src/BackupRepository.cpp
    src/FileScanner.cpp
    src/BackupJob.cpp
    src/FullBackupJob.cpp
    src/IncrementalBackupJob.cpp
    src/BackupManager.cpp
    src/MainWindow.cpp
    include/Logger.h
    include/AppSettings.h
    include/BackupRepository.h
    include/FileScanner.h
    include/BackupJob.h
    include/FullBackupJob.h
    include/IncrementalBackupJob.h
    include/BackupManager.h
    include/MainWindow.h
)

add_executable(backup_system ${APP_SOURCES})
target_include_directories(backup_system PRIVATE include)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(backup_system PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)
else()
    target_link_libraries(backup_system PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql)
endif()

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

# Общие исходники (без main.cpp и MainWindow)
set(LIB_SOURCES
    src/Logger.cpp
    src/AppSettings.cpp
    src/BackupRepository.cpp
    src/FileScanner.cpp
    src/BackupJob.cpp
    src/FullBackupJob.cpp
    src/IncrementalBackupJob.cpp
    src/BackupManager.cpp
    include/Logger.h
    include/AppSettings.h
    include/BackupRepository.h
    include/FileScanner.h
    include/BackupJob.h
    include/FullBackupJob.h
    include/IncrementalBackupJob.h
    include/BackupManager.h
)

# Тесты
set(TEST_SOURCES
    tests/main_test.cpp
    tests/test_Logger.cpp
    tests/test_AppSettings.cpp
    tests/test_BackupRepository.cpp
    tests/test_FileScanner.cpp
    tests/test_BackupManager.cpp
    tests/test_FullBackupJob.cpp
    tests/test_IncrementalBackupJob.cpp
    ${LIB_SOURCES}
)

add_executable(backup_tests ${TEST_SOURCES})
target_include_directories(backup_tests PRIVATE include)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(backup_tests PRIVATE
        Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql
        GTest::gtest GTest::gmock)
else()
    target_link_libraries(backup_tests PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql
        GTest::gtest GTest::gmock)
endif()

include(GoogleTest)
gtest_discover_tests(backup_tests
    PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    DISCOVERY_TIMEOUT 30
)

# Сценарные мини-программы
foreach(scenario full_backup incremental restore)
    add_executable(scenario_${scenario}
        tests/scenario_${scenario}.cpp
        ${LIB_SOURCES}
    )
    target_include_directories(scenario_${scenario} PRIVATE include)
    if(QT_VERSION_MAJOR EQUAL 6)
        target_link_libraries(scenario_${scenario} PRIVATE
            Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql)
    else()
        target_link_libraries(scenario_${scenario} PRIVATE
            Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Sql)
    endif()
endforeach()

add_custom_target(project_files SOURCES
    Dockerfile
    docker-compose.yml
    run_tests.sh
)