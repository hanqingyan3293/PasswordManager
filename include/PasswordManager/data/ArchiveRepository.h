#pragma once

#include "PasswordManager/domain/ArchiveRecord.h"

#include <QList>
#include <QSqlQuery>
#include <QString>

namespace PasswordManager {

class ArchiveRepository {
public:
    explicit ArchiveRepository(QString connectionName);

    QList<ArchiveRecord> list(const QString& filter = QString()) const;
    ArchiveRecord findByPath(const QString& path) const;
    bool upsert(const ArchiveRecord& record, QString* errorMessage = nullptr) const;
    int upsertMany(const QList<ArchiveRecord>& records, QString* errorMessage = nullptr) const;

private:
    ArchiveRecord readRecord(const QSqlQuery& query) const;

    QString m_connectionName;
};

} // namespace PasswordManager
