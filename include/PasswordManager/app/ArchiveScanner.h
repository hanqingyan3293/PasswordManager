#pragma once

#include "PasswordManager/domain/ArchiveRecord.h"

#include <QList>
#include <QStringList>

namespace PasswordManager {

struct ScanResult {
    QList<ArchiveRecord> archives;
    int skippedCount = 0;
    qint64 elapsedMs = 0;
    bool fullHashCalculated = true;
};

class ArchiveScanner {
public:
    explicit ArchiveScanner(bool calculateFullHash = true);

    static bool isSupportedArchive(const QString& filePath);

    ScanResult scanFiles(const QStringList& filePaths) const;
    ScanResult scanDirectory(const QString& directoryPath) const;

private:
    bool scanOneFile(const QString& filePath, ArchiveRecord* record) const;
    QString quickHash(const QString& filePath, qint64 fileSize) const;
    QString fullHash(const QString& filePath) const;

    bool m_calculateFullHash = true;
};

} // namespace PasswordManager
