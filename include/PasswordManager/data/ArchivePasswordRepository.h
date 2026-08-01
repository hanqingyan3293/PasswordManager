#pragma once

#include "PasswordManager/domain/ArchivePasswordRecord.h"

#include <QList>
#include <QSqlQuery>
#include <QString>

namespace PasswordManager {

class ArchivePasswordRepository {
public:
    explicit ArchivePasswordRepository(QString connectionName);

    bool recordSuccess(int archiveId, int passwordId, const QString& password, QString* errorMessage = nullptr) const;
    QList<ArchivePasswordRecord> list(const QString& filter = QString()) const;
    QList<ArchivePasswordRecord> listForArchive(int archiveId) const;
    bool remove(int id, QString* errorMessage = nullptr) const;

private:
    ArchivePasswordRecord readRecord(const QSqlQuery& query) const;

    QString m_connectionName;
};

} // namespace PasswordManager
