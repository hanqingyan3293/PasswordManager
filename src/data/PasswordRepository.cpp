#include "PasswordManager/data/PasswordRepository.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <utility>

namespace PasswordManager {

PasswordRepository::PasswordRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

QList<PasswordRecord> PasswordRepository::list(const QString& filter) const
{
    QList<PasswordRecord> records;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));

    if (filter.trimmed().isEmpty()) {
        query.prepare(R"(
            SELECT id, password, category, note, favorite, success_count, failure_count, created_at, updated_at
            FROM passwords
            ORDER BY favorite DESC, updated_at DESC, id DESC
        )");
    } else {
        query.prepare(R"(
            SELECT id, password, category, note, favorite, success_count, failure_count, created_at, updated_at
            FROM passwords
            WHERE password LIKE :filter OR category LIKE :filter OR note LIKE :filter
            ORDER BY favorite DESC, updated_at DESC, id DESC
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

QList<PasswordRecord> PasswordRepository::listByCategory(const QString& category) const
{
    QList<PasswordRecord> records;
    const QString trimmedCategory = category.trimmed();
    if (trimmedCategory.isEmpty()) {
        return records;
    }

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT id, password, category, note, favorite, success_count, failure_count, created_at, updated_at
        FROM passwords
        WHERE category = :category
        ORDER BY favorite DESC, success_count DESC, failure_count ASC, updated_at DESC, id DESC
    )");
    query.bindValue(":category", trimmedCategory);

    if (!query.exec()) {
        return records;
    }

    while (query.next()) {
        records.append(readRecord(query));
    }
    return records;
}

PasswordRecord PasswordRepository::findByPassword(const QString& password) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT id, password, category, note, favorite, success_count, failure_count, created_at, updated_at
        FROM passwords
        WHERE password = :password
        ORDER BY id ASC
        LIMIT 1
    )");
    query.bindValue(":password", password);

    if (!query.exec() || !query.next()) {
        return PasswordRecord();
    }
    return readRecord(query);
}

bool PasswordRepository::add(const PasswordRecord& record, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO passwords (
            password, category, note, favorite, success_count, failure_count, created_at, updated_at
        ) VALUES (
            :password, :category, :note, :favorite, :success_count, :failure_count, :created_at, :updated_at
        )
    )");

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    query.bindValue(":password", record.password);
    query.bindValue(":category", record.category.isNull() ? QString("") : record.category);
    query.bindValue(":note", record.note.isNull() ? QString("") : record.note);
    query.bindValue(":favorite", record.favorite ? 1 : 0);
    query.bindValue(":success_count", record.successCount);
    query.bindValue(":failure_count", record.failureCount);
    query.bindValue(":created_at", now);
    query.bindValue(":updated_at", now);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool PasswordRepository::update(const PasswordRecord& record, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        UPDATE passwords
        SET password = :password,
            category = :category,
            note = :note,
            favorite = :favorite,
            success_count = :success_count,
            failure_count = :failure_count,
            updated_at = :updated_at
        WHERE id = :id
    )");

    query.bindValue(":password", record.password);
    query.bindValue(":category", record.category.isNull() ? QString("") : record.category);
    query.bindValue(":note", record.note.isNull() ? QString("") : record.note);
    query.bindValue(":favorite", record.favorite ? 1 : 0);
    query.bindValue(":success_count", record.successCount);
    query.bindValue(":failure_count", record.failureCount);
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id", record.id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool PasswordRepository::remove(int id, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool PasswordRepository::incrementStats(int id, bool success, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(success
            ? "UPDATE passwords SET success_count = success_count + 1, updated_at = :updated_at WHERE id = :id"
            : "UPDATE passwords SET failure_count = failure_count + 1, updated_at = :updated_at WHERE id = :id");
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

PasswordRecord PasswordRepository::readRecord(const QSqlQuery& query) const
{
    PasswordRecord record;
    record.id = query.value(0).toInt();
    record.password = query.value(1).toString();
    record.category = query.value(2).toString();
    record.note = query.value(3).toString();
    record.favorite = query.value(4).toInt() != 0;
    record.successCount = query.value(5).toInt();
    record.failureCount = query.value(6).toInt();
    record.createdAt = QDateTime::fromString(query.value(7).toString(), Qt::ISODate);
    record.updatedAt = QDateTime::fromString(query.value(8).toString(), Qt::ISODate);
    return record;
}

} // namespace PasswordManager
