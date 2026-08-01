#pragma once

#include <QString>

namespace PasswordManager {

class DatabaseBackupService {
public:
    DatabaseBackupService(QString connectionName, QString databasePath, QString backupDir);

    bool createBackup(QString* backupPath = nullptr, QString* errorMessage = nullptr) const;
    bool restoreFromBackup(const QString& sourcePath, QString* safetyBackupPath = nullptr, QString* errorMessage = nullptr) const;

private:
    bool validateBackupFile(const QString& sourcePath, QString* errorMessage) const;
    QString timestampedBackupPath(const QString& prefix) const;
    QString sqliteStringLiteral(const QString& value) const;

    QString m_connectionName;
    QString m_databasePath;
    QString m_backupDir;
};

} // namespace PasswordManager
