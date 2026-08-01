#pragma once

#include "PasswordManager/app/ExtractService.h"

#include <QString>

namespace PasswordManager {

class ExtractLogRepository {
public:
    explicit ExtractLogRepository(QString connectionName);

    bool add(int archiveId, const QString& archivePath, const QString& outputDirectory, ExtractStatus status, const QString& message, QString* errorMessage = nullptr) const;

private:
    QString m_connectionName;
};

} // namespace PasswordManager

