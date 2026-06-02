#pragma once
#include <QObject>
#include "BackupSettings.h"

class AppSettings : public QObject {
    Q_OBJECT
public:
    explicit AppSettings(QObject *parent = nullptr);
    BackupSettings load() const;
    void save(const BackupSettings &settings);
};
