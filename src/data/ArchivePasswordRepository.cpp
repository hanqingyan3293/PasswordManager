#include "PasswordManager/data/ArchivePasswordRepository.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <utility>

namespace PasswordManager {

ArchivePasswordRepository::ArchivePasswordRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

bool ArchivePasswordRepository::recordSuccess(int archiveId, int passwordId, const QString& password, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO archive_passwords (
            archive_id, password_id, password, success_count, last_success_at, created_at, updated_at
        ) VALUES (
            :archive_id, :password_id, :password, 1, :now, :now, :now
        )
        ON CONFLICT(archive_id, password) DO UPDATE SET
            password_id = excluded.password_id,
            success_count = success_count + 1,
            last_success_at = excluded.last_success_at,
            updated_at = excluded.updated_at
    )");

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    query.bindValue(":archive_id", archiveId);
    if (passwordId > 0) {
        query.bindValue(":password_id", passwordId);
    } else {
        query.bindValue(":password_id", QVariant());
    }
    query.bindValue(":password", password);
    query.bindValue(":now", now);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

QList<ArchivePasswordRecord> ArchivePasswordRepository::list(const QString& filter) const
{
    QList<ArchivePasswordRecord> records;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    if (filter.trimmed().isEmpty()) {
        query.prepare(R"(
            SELECT ap.id, ap.archive_id, COALESCE(ap.password_id, 0), a.file_name, a.path,
                   ap.password, ap.success_count, ap.last_success_at, ap.updated_at
            FROM archive_passwords ap
            JOIN archives a ON a.id = ap.archive_id
            ORDER BY ap.last_success_at DESC, ap.id DESC
        )");
    } else {
        query.prepare(R"(
            SELECT ap.id, ap.archive_id, COALESCE(ap.password_id, 0), a.file_name, a.path,
                   ap.password, ap.success_count, ap.last_success_at, ap.updated_at
            FROM archive_passwords ap
            JOIN archives a ON a.id = ap.archive_id
            WHERE a.file_name LIKE :filter OR a.path LIKE :filter OR ap.password LIKE :filter
            ORDER BY ap.last_success_at DESC, ap.id DESC
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

QList<ArchivePasswordRecord> ArchivePasswordRepository::listForArchive(int archiveId) const
{
    QList<ArchivePasswordRecord> records;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT ap.id, ap.archive_id, COALESCE(ap.password_id, 0), a.file_name, a.path,
               ap.password, ap.success_count, ap.last_success_at, ap.updated_at
        FROM archive_passwords ap
        JOIN archives a ON a.id = ap.archive_id
        WHERE ap.archive_id = :archive_id
        ORDER BY ap.last_success_at DESC, ap.id DESC
    )");
    query.bindValue(":archive_id", archiveId);

    if (!query.exec()) {
        return records;
    }

    while (query.next()) {
        records.append(readRecord(query));
    }
    return records;
}

bool ArchivePasswordRepository::remove(int id, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("DELETE FROM archive_passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

ArchivePasswordRecord ArchivePasswordRepository::readRecord(const QSqlQuery& query) const
{
    ArchivePasswordRecord record;
    record.id = query.value(0).toInt();
    record.archiveId = query.value(1).toInt();
    record.passwordId = query.value(2).toInt();
    record.archiveName = query.value(3).toString();
    record.archivePath = query.value(4).toString();
    record.password = query.value(5).toString();
    record.successCount = query.value(6).toInt();
    record.lastSuccessAt = QDateTime::fromString(query.value(7).toString(), Qt::ISODate);
    record.updatedAt = QDateTime::fromString(query.value(8).toString(), Qt::ISODate);
    return record;
}

} // namespace PasswordManager
