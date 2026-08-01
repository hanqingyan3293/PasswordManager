#pragma once

#include <QString>

namespace PasswordManager {

class ArchiveRepository;
class ArchivePasswordRepository;
class AppPaths;
class PasswordRepository;
class PasswordTestTaskManager;

struct ShellActionResult {
    bool success = false;
    int archiveId = 0;
    int enqueuedCount = 0;
    int scannedCount = 0;
    int knownPasswordCount = 0;
    bool noPasswordArchive = false;
    QString message;
};

class ShellActionService {
public:
    ShellActionService(
        const AppPaths& paths,
        const ArchiveRepository& archiveRepository,
        const ArchivePasswordRepository& archivePasswordRepository,
        const PasswordRepository& passwordRepository,
        PasswordTestTaskManager& taskManager);

    ShellActionResult enqueueArchivePasswordTests(const QString& archivePath) const;
    ShellActionResult lookupKnownPasswords(const QString& archivePath) const;
    ShellActionResult scanFolder(const QString& folderPath) const;

private:
    ShellActionResult scanAndSaveArchive(const QString& archivePath) const;

    const AppPaths& m_paths;
    const ArchiveRepository& m_archiveRepository;
    const ArchivePasswordRepository& m_archivePasswordRepository;
    const PasswordRepository& m_passwordRepository;
    PasswordTestTaskManager& m_taskManager;
};

} // namespace PasswordManager
