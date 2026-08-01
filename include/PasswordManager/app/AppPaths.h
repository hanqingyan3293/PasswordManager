#pragma once

#include <QString>

namespace PasswordManager {

class AppPaths {
public:
    explicit AppPaths(QString applicationDir);

    const QString& applicationDir() const;
    QString dataDir() const;
    QString configDir() const;
    QString logsDir() const;
    QString backupDir() const;
    QString toolsDir() const;
    QString sevenZipDir() const;
    QString sevenZipExecutable() const;
    QString sevenZipGuiExecutable() const;
    QString sevenZipFileManagerExecutable() const;
    QString sevenZipChineseLanguageFile() const;

    bool ensureRuntimeDirectories() const;
    bool ensureSevenZipChineseLanguage(QString* errorMessage = nullptr) const;

private:
    QString m_applicationDir;
};

} // namespace PasswordManager
