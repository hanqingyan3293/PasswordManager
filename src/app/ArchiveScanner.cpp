#include "PasswordManager/app/ArchiveScanner.h"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QElapsedTimer>

namespace PasswordManager {

ArchiveScanner::ArchiveScanner(bool calculateFullHash)
    : m_calculateFullHash(calculateFullHash)
{
}

bool ArchiveScanner::isSupportedArchive(const QString& filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    return suffix == "zip" || suffix == "rar" || suffix == "7z";
}

ScanResult ArchiveScanner::scanFiles(const QStringList& filePaths) const
{
    QElapsedTimer timer;
    timer.start();
    ScanResult result;
    result.fullHashCalculated = m_calculateFullHash;
    for (const QString& filePath : filePaths) {
        ArchiveRecord record;
        if (scanOneFile(filePath, &record)) {
            result.archives.append(record);
        } else {
            ++result.skippedCount;
        }
    }
    result.elapsedMs = timer.elapsed();
    return result;
}

ScanResult ArchiveScanner::scanDirectory(const QString& directoryPath) const
{
    QElapsedTimer timer;
    timer.start();
    ScanResult result;
    result.fullHashCalculated = m_calculateFullHash;
    QDirIterator iterator(directoryPath, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        ArchiveRecord record;
        if (scanOneFile(iterator.next(), &record)) {
            result.archives.append(record);
        } else {
            ++result.skippedCount;
        }
    }
    result.elapsedMs = timer.elapsed();
    return result;
}

bool ArchiveScanner::scanOneFile(const QString& filePath, ArchiveRecord* record) const
{
    const QFileInfo info(filePath);
    if (!info.exists() || !info.isFile() || !isSupportedArchive(filePath)) {
        return false;
    }

    record->path = info.absoluteFilePath();
    record->fileName = info.fileName();
    record->extension = info.suffix().toLower();
    record->sizeBytes = info.size();
    record->modifiedAt = info.lastModified();
    record->quickHash = quickHash(info.absoluteFilePath(), info.size());
    record->fullHash = m_calculateFullHash ? fullHash(info.absoluteFilePath()) : QString();
    record->scannedAt = QDateTime::currentDateTime();
    return !record->quickHash.isEmpty() && (!m_calculateFullHash || !record->fullHash.isEmpty());
}

QString ArchiveScanner::quickHash(const QString& filePath, qint64 fileSize) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    constexpr qint64 sampleSize = 1024 * 1024;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(QByteArray::number(fileSize));

    if (fileSize <= sampleSize * 2) {
        hash.addData(file.readAll());
    } else {
        hash.addData(file.read(sampleSize));
        if (file.seek(fileSize - sampleSize)) {
            hash.addData(file.read(sampleSize));
        }
    }

    return QString::fromLatin1(hash.result().toHex());
}

QString ArchiveScanner::fullHash(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    constexpr qint64 chunkSize = 1024 * 1024;
    while (!file.atEnd()) {
        const QByteArray chunk = file.read(chunkSize);
        if (chunk.isEmpty() && file.error() != QFile::NoError) {
            return QString();
        }
        hash.addData(chunk);
    }
    return QString::fromLatin1(hash.result().toHex());
}

} // namespace PasswordManager
