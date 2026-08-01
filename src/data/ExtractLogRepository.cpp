#include "PasswordManager/data/ExtractLogRepository.h"

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <utility>

namespace PasswordManager {

ExtractLogRepository::ExtractLogRepository(QString connectionName)
    : m_connectionName(std::move(connectionName))
{
}

bool ExtractLogRepository::add(int archiveId, const QString& archivePath, const QString& outputDirectory, ExtractStatus status, const QString& message, QString* errorMessage) const
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    query.prepare(R"(
        INSERT INTO extract_logs (
            archive_id, archive_path, output_directory, status, message, created_at
        ) VALUES (
            :archive_id, :archive_path, :output_directory, :status, :message, :created_at
        )
    )");
    query.bindValue(":archive_id", archiveId);
    query.bindValue(":archive_path", archivePath);
    query.bindValue(":output_directory", outputDirectory);
    query.bindValue(":status", extractStatusText(status));
    query.bindValue(":message", message);
    query.bindValue(":created_at", QDateTime::currentDateTime().toString(Qt::ISODate));

    if (!query.exec()) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }
    return true;
}

} // namespace PasswordManager

