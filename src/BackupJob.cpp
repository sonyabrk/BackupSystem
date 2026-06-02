#include "BackupJob.h"

BackupJob::BackupJob(const BackupSettings &settings, QObject *parent)
    : QObject(parent), QRunnable(), m_settings(settings)
{
    setAutoDelete(false);
}
