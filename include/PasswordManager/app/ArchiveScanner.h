#pragma once

#include "PasswordManager/domain/ArchiveRecord.h"

#include <QList>
#include <QStringList>

namespace PasswordManager {

struct ScanResult {
    QList<ArchiveRecord> archives;
    int skippedCount = 0;
};

class ArchiveScanner {
public:
    static bool isSupportedArchive(const QString& filePath);

    ScanResult scanFiles(const QStringList& filePaths) const;
    ScanResult scanDirectory(const QString& directoryPath) const;

private:
    bool scanOneFile(const QString& filePath, ArchiveRecord* record) const;
    QString quickHash(const QString& filePath, qint64 fileSize) const;
};

} // namespace PasswordManager

