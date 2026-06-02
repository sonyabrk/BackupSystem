#!/bin/bash
set -e
echo "  BackupSystem — Запуск тестов"
export QT_QPA_PLATFORM=offscreen
./backup_tests --gtest_color=yes
echo "  Все тесты пройдены успешно"
