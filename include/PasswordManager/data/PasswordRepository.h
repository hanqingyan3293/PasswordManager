#pragma once

#include "PasswordManager/domain/PasswordRecord.h"

#include <QList>
#include <QSqlQuery>
#include <QString>

namespace PasswordManager {

class PasswordRepository {
public:
    explicit PasswordRepository(QString connectionName);

    QList<PasswordRecord> list(const QString& filter = QString()) const;
    QList<PasswordRecord> listByCategory(const QString& category) const;
    PasswordRecord findByPassword(const QString& password) const;
    bool add(const PasswordRecord& record, QString* errorMessage = nullptr) const;
    bool update(const PasswordRecord& record, QString* errorMessage = nullptr) const;
    bool remove(int id, QString* errorMessage = nullptr) const;
    bool incrementStats(int id, bool success, QString* errorMessage = nullptr) const;

private:
    PasswordRecord readRecord(const QSqlQuery& query) const;

    QString m_connectionName;
};

} // namespace PasswordManager
