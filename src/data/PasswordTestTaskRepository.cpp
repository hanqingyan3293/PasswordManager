#include "PasswordManager/data/PasswordTestTaskRepository.h"

#include "PasswordManager/app/SevenZipRunner.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <utility>

namespace PasswordManager {

namespace {

PasswordTestTaskStatus taskStatusFromText(const QString& text)
{
    if (text == "WAITING") {
        return PasswordTestTaskStatus::Waiting;
    }
    if (text == "RUNNING") {
        return PasswordTestTaskStatus::Running;
    }
    if (text == "COMPLETED") {
        return PasswordTestTaskStatus::Completed;
    }
    if (text == "FAILED") {
        return PasswordTestTaskStatus::Failed;
    }
    if (text == "CANCELLED") {
        return PasswordTestTaskStatus::Cancelled;
    }
    if (text == "TIMEOUT") {
        return PasswordTestTaskStatus::Timeout;
    }
    return PasswordTestTaskStatus::Failed;
}

SevenZipTestStatus sevenZipStatusFromText(const QString& text)
{
    if (text == "SUCCESS") {
        return SevenZipTestStatus::Success;
    }
    if (text == "WRONG_PASSWORD") {
        return SevenZipTestStatus::WrongPassword;
    }
    if (text == "MISSING_7ZIP") {
        return SevenZipTestStatus::MissingSevenZip;
    }
    if (text == "ARCHIVE_ERROR") {
        return SevenZipTestStatus::ArchiveError;
    }
    if (text == "TIMEOUT") {
        return SevenZipTestStatus::Timeout;
    }
    return SevenZipTestStatus::ProcessError;
}

} // namespace

PasswordTestTaskRepository::PasswordTestTaskRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

int PasswordTestTaskRepository::add(const PasswordTestTask& task, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO password_test_tasks (
            archive_id, password_id, archive_path, password, status, test_status, message, timeout_ms, created_at, updated_at
        ) VALUES (
            :archive_id, :password_id, :archive_path, :password, :status, :test_status, :message, :timeout_ms, :now, :now
        )
    )");

    const QString now = QDateTime::currentDateTime().toString(Qt::ISODate);
    query.bindValue(":archive_id", task.archiveId);
    query.bindValue(":password_id", task.passwordId);
    query.bindValue(":archive_path", task.archivePath.isNull() ? QString("") : task.archivePath);
    query.bindValue(":password", task.password.isNull() ? QString("") : task.password);
    query.bindValue(":status", passwordTestTaskStatusText(task.status));
    query.bindValue(":test_status", sevenZipTestStatusText(task.testStatus));
    query.bindValue(":message", task.message.isNull() ? QString("") : task.message);
    query.bindValue(":timeout_ms", task.timeoutMs);
    query.bindValue(":now", now);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return 0;
    }
    return query.lastInsertId().toInt();
}

bool PasswordTestTaskRepository::update(const PasswordTestTask& task, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        UPDATE password_test_tasks
        SET archive_id = :archive_id,
            password_id = :password_id,
            archive_path = :archive_path,
            password = :password,
            status = :status,
            test_status = :test_status,
            message = :message,
            timeout_ms = :timeout_ms,
            updated_at = :updated_at
        WHERE id = :id
    )");

    query.bindValue(":archive_id", task.archiveId);
    query.bindValue(":password_id", task.passwordId);
    query.bindValue(":archive_path", task.archivePath.isNull() ? QString("") : task.archivePath);
    query.bindValue(":password", task.password.isNull() ? QString("") : task.password);
    query.bindValue(":status", passwordTestTaskStatusText(task.status));
    query.bindValue(":test_status", sevenZipTestStatusText(task.testStatus));
    query.bindValue(":message", task.message.isNull() ? QString("") : task.message);
    query.bindValue(":timeout_ms", task.timeoutMs);
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));
    query.bindValue(":id", task.id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

QList<PasswordTestTask> PasswordTestTaskRepository::list() const
{
    QList<PasswordTestTask> records;
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        SELECT id, archive_id, password_id, archive_path, password, status, test_status, message, timeout_ms
        FROM password_test_tasks
        ORDER BY id ASC
    )");

    if (!query.exec()) {
        return records;
    }

    while (query.next()) {
        records.append(readRecord(query));
    }
    return records;
}

bool PasswordTestTaskRepository::prepareTasksForStartup(QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        UPDATE password_test_tasks
        SET status = 'FAILED',
            test_status = 'PROCESS_ERROR',
            message = 'Interrupted before application restart.',
            updated_at = :updated_at
        WHERE status = 'RUNNING'
    )");
    query.bindValue(":updated_at", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

bool PasswordTestTaskRepository::removeFinishedById(int id, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        DELETE FROM password_test_tasks
        WHERE id = :id
          AND status IN ('COMPLETED', 'FAILED', 'CANCELLED', 'TIMEOUT')
    )");
    query.bindValue(":id", id);

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return query.numRowsAffected() > 0;
}

int PasswordTestTaskRepository::removeFinished(QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        DELETE FROM password_test_tasks
        WHERE status IN ('COMPLETED', 'FAILED', 'CANCELLED', 'TIMEOUT')
    )");

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return -1;
    }
    return query.numRowsAffected();
}

PasswordTestTask PasswordTestTaskRepository::readRecord(const QSqlQuery& query) const
{
    PasswordTestTask task;
    task.id = query.value(0).toInt();
    task.archiveId = query.value(1).toInt();
    task.passwordId = query.value(2).toInt();
    task.archivePath = query.value(3).toString();
    task.password = query.value(4).toString();
    task.status = taskStatusFromText(query.value(5).toString());
    task.testStatus = sevenZipStatusFromText(query.value(6).toString());
    task.message = query.value(7).toString();
    task.timeoutMs = query.value(8).toInt();
    return task;
}

} // namespace PasswordManager
