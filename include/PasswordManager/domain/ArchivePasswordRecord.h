#pragma once

#include <QDateTime>
#include <QString>

namespace PasswordManager {

struct ArchivePasswordRecord {
    int id = 0;
    int archiveId = 0;
    int passwordId = 0;
    QString archiveName;
    QString archivePath;
    QString password;
    int successCount = 0;
    QDateTime lastSuccessAt;
    QDateTime updatedAt;
};

} // namespace PasswordManager

