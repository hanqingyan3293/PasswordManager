#include "PasswordManager/app/ArchiveFingerprintService.h"

#include "PasswordManager/data/ArchiveRepository.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>

namespace PasswordManager {

ArchiveFingerprintService::ArchiveFingerprintService(const ArchiveRepository& archiveRepository)
    : m_archiveRepository(archiveRepository)
{
}

FingerprintBackfillResult ArchiveFingerprintService::backfillMissingFullHashes() const
{
    FingerprintBackfillResult result;
    const QList<ArchiveRecord> records = m_archiveRepository.listMissingFullHash();
    result.totalMissing = records.size();

    for (const ArchiveRecord& record : records) {
        if (!QFileInfo::exists(record.path)) {
            ++result.missingFiles;
            continue;
        }

        const QString fullHash = calculateFullHash(record.path);
        if (fullHash.isEmpty()) {
            ++result.failed;
            result.lastError = "计算文件指纹失败：" + record.path;
            continue;
        }

        QString error;
        if (!m_archiveRepository.updateFullHash(record.id, fullHash, &error)) {
            ++result.failed;
            result.lastError = error.isEmpty() ? "更新文件指纹失败：" + record.path : error;
            continue;
        }
        ++result.updated;
    }

    return result;
}

QString ArchiveFingerprintService::calculateFullHash(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    constexpr qint64 chunkSize = 1024 * 1024;
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty() && file.error() != QFile::NoError) {
            return {};
        }
        hash.addData(chunk);
    }

    return QString::fromLatin1(hash.result().toHex());
}

} // namespace PasswordManager
