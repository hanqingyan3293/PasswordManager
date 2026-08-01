#pragma once

#include <QDateTime>
#include <QString>

namespace PasswordManager {

struct ArchiveRecord {
    int id = 0;
    QString path;
    QString fileName;
    QString extension;
    qint64 sizeBytes = 0;
    QDateTime modifiedAt;
    QString quickHash;
    QDateTime scannedAt;
};

} // namespace PasswordManager

