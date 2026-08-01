#pragma once

#include <QString>

namespace PasswordManager {

class ArchiveRepository;

struct FingerprintBackfillResult {
    int totalMissing = 0;
    int updated = 0;
    int missingFiles = 0;
    int failed = 0;
    QString lastError;
};

class ArchiveFingerprintService {
public:
    explicit ArchiveFingerprintService(const ArchiveRepository& archiveRepository);

    FingerprintBackfillResult backfillMissingFullHashes() const;

private:
    QString calculateFullHash(const QString& filePath) const;

    const ArchiveRepository& m_archiveRepository;
};

} // namespace PasswordManager
