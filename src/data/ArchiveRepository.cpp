#include "PasswordManager/data/ArchiveRepository.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <utility>

namespace PasswordManager {

ArchiveRepository::ArchiveRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

QList<ArchiveRecord> ArchiveRepository::list(const QString& filter) const
{
    QList<ArchiveRecord> records;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    if (filter.trimmed().isEmpty()) {
        query.prepare(R"(
            SELECT id, path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at
            FROM archives
            ORDER BY scanned_at DESC, id DESC
        )");
    } else {
        query.prepare(R"(
            SELECT id, path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at
            FROM archives
            WHERE path LIKE :filter OR file_name LIKE :filter OR extension LIKE :filter
            ORDER BY scanned_at DESC, id DESC
        )");
        query.bindValue(":filter", "%" + filter.trimmed() + "%");
    }

    if (!query.exec()) {
        return records;
    }

    while (query.next()) {
        records.append(readRecord(query));
    }
    return records;
}

ArchiveRecord ArchiveRepository::findByPath(const QString& path) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT id, path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at
        FROM archives
        WHERE path = :path
        LIMIT 1
    )");
    query.bindValue(":path", path);

    if (!query.exec() || !query.next()) {
        return ArchiveRecord();
    }
    return readRecord(query);
}

bool ArchiveRepository::upsert(const ArchiveRecord& record, QString* errorMessage) const
{
    const ArchiveRecord existing = findByPath(record.path);
    if (existing.id > 0 && existing.quickHash != record.quickHash) {
        QSqlQuery cleanup(QSqlDatabase::database(m_connectionName));
        cleanup.prepare("DELETE FROM archive_passwords WHERE archive_id = :archive_id");
        cleanup.bindValue(":archive_id", existing.id);
        if (!cleanup.exec()) {
            if (errorMessage) {
                *errorMessage = cleanup.lastError().text();
            }
            return false;
        }
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO archives (
            path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at
        ) VALUES (
            :path, :file_name, :extension, :size_bytes, :modified_at, :quick_hash, :scanned_at
        )
        ON CONFLICT(path) DO UPDATE SET
            file_name = excluded.file_name,
            extension = excluded.extension,
            size_bytes = excluded.size_bytes,
            modified_at = excluded.modified_at,
            quick_hash = excluded.quick_hash,
            scanned_at = excluded.scanned_at
    )");

    query.bindValue(":path", record.path);
    query.bindValue(":file_name", record.fileName);
    query.bindValue(":extension", record.extension);
    query.bindValue(":size_bytes", record.sizeBytes);
    query.bindValue(":modified_at", record.modifiedAt.toString(Qt::ISODate));
    query.bindValue(":quick_hash", record.quickHash);
    query.bindValue(":scanned_at", record.scannedAt.toString(Qt::ISODate));

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

int ArchiveRepository::upsertMany(const QList<ArchiveRecord>& records, QString* errorMessage) const
{
    int count = 0;
    for (const ArchiveRecord& record : records) {
        if (!upsert(record, errorMessage)) {
            return count;
        }
        ++count;
    }
    return count;
}

ArchiveRecord ArchiveRepository::readRecord(const QSqlQuery& query) const
{
    ArchiveRecord record;
    record.id = query.value(0).toInt();
    record.path = query.value(1).toString();
    record.fileName = query.value(2).toString();
    record.extension = query.value(3).toString();
    record.sizeBytes = query.value(4).toLongLong();
    record.modifiedAt = QDateTime::fromString(query.value(5).toString(), Qt::ISODate);
    record.quickHash = query.value(6).toString();
    record.scannedAt = QDateTime::fromString(query.value(7).toString(), Qt::ISODate);
    return record;
}

} // namespace PasswordManager
