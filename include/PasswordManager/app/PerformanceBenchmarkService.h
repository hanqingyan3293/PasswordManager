#pragma once

#include <QString>

namespace PasswordManager {

class AppPaths;

class PerformanceBenchmarkService {
public:
    PerformanceBenchmarkService(const AppPaths& paths, QString databaseConnectionName);

    bool run(const QString& scanDirectory = QString(), QString* outputPath = nullptr, QString* errorMessage = nullptr) const;

private:
    QString report(const QString& scanDirectory) const;

    const AppPaths& m_paths;
    QString m_databaseConnectionName;
};

} // namespace PasswordManager
