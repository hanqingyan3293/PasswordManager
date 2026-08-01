#pragma once

#include <QString>

namespace PasswordManager {

class AppPaths;

class DiagnosticService {
public:
    DiagnosticService(const AppPaths& paths, QString databaseConnectionName);

    bool exportDiagnosticPackage(QString* outputDirectory = nullptr, QString* errorMessage = nullptr) const;

private:
    int tableCount(const QString& tableName) const;
    QString diagnosticText() const;
    bool copyLogFiles(const QString& outputDirectory, QString* errorMessage) const;

    const AppPaths& m_paths;
    QString m_databaseConnectionName;
};

} // namespace PasswordManager
