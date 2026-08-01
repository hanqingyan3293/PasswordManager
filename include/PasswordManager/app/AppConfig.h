#pragma once

#include <QString>

namespace PasswordManager {

class AppPaths;

struct SmartMatchSettings {
    bool enableExactHistory = true;
    bool enableDirectoryHistory = true;
    bool enableCategoryCandidates = true;
    bool enablePasswordLibrary = true;
    bool enableDescriptionFiles = true;
    bool calculateFullHashDuringScan = true;
    int maxCandidates = 100;
    int maxDescriptionCandidates = 20;
    int maxDescriptionFileBytes = 256 * 1024;
};

struct ShellMenuSettings {
    bool enableArchiveLookup = true;
    bool enableArchiveTest = true;
    bool enableArchiveViewResults = true;
    bool enableArchiveExtract = true;
    bool enableArchiveCompress = true;
    bool enableArchiveOpenMain = true;
    bool enableFolderScan = true;
    bool enableFolderCompress = true;
    bool enableFolderOpenMain = true;
    bool enableFileCompress = true;
};

class AppConfig {
public:
    explicit AppConfig(const AppPaths& paths);

    SmartMatchSettings smartMatchSettings() const;
    void saveSmartMatchSettings(const SmartMatchSettings& settings) const;

    ShellMenuSettings shellMenuSettings() const;
    void saveShellMenuSettings(const ShellMenuSettings& settings) const;

private:
    QString settingsPath() const;

    const AppPaths& m_paths;
};

} // namespace PasswordManager
