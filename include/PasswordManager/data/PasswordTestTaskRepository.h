#pragma once

#include "PasswordManager/app/PasswordTestTaskManager.h"

#include <QList>
#include <QSqlQuery>
#include <QString>

namespace PasswordManager {

class PasswordTestTaskRepository {
public:
    explicit PasswordTestTaskRepository(QString connectionName);

    int add(const PasswordTestTask& task, QString* errorMessage = nullptr) const;
    bool update(const PasswordTestTask& task, QString* errorMessage = nullptr) const;
    QList<PasswordTestTask> list() const;
    bool prepareTasksForStartup(QString* errorMessage = nullptr) const;
    bool removeFinishedById(int id, QString* errorMessage = nullptr) const;
    int removeFinished(QString* errorMessage = nullptr) const;

private:
    PasswordTestTask readRecord(const QSqlQuery& query) const;

    QString m_connectionName;
};

} // namespace PasswordManager
