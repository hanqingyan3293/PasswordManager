#pragma once

#include <QString>

namespace PasswordManager {

class AppPaths;

class DatabaseService {
public:
    explicit DatabaseService(const AppPaths& paths);
    ~DatabaseService();

    bool open();
    QString lastError() const;
    QString connectionName() const;
    QString databasePath() const;

private:
    bool migrate();
    bool execute(const QString& sql);
    bool executeOptionalAlter(const QString& sql);

    const AppPaths& m_paths;
    QString m_connectionName;
    QString m_lastError;
};

} // namespace PasswordManager
