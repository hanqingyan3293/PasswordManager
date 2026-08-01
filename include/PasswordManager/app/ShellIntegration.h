#pragma once

#include "PasswordManager/app/AppConfig.h"

#include <QList>
#include <QString>
#include <QStringList>

namespace PasswordManager {

struct ShellIntegrationExtensionStatus {
    QString extension;
    bool rootInstalled = false;
    bool addQueueInstalled = false;
    bool viewResultsInstalled = false;
    bool commandsPointToExecutable = false;
    QString addQueueCommand;
    QString viewResultsCommand;
};

class ShellIntegration {
public:
    static QStringList supportedExtensions();
    static QString menuRootKey(const QString& extension);
    static QString fileMenuRootKey();
    static QString directoryMenuRootKey();
    static QString directoryBackgroundMenuRootKey();
    static QString archiveMenuClassKey();
    static QString fileMenuClassKey();
    static QString directoryMenuClassKey();
    static QString directoryBackgroundMenuClassKey();
    static QStringList uninstallRegistryKeys();
    static QString commandFor(const QString& executablePath, const QString& action, const QString& filePlaceholder = "%1");
    static QString compressCommandFor(const QString& executablePath, const QString& filePlaceholder = "%1");
    static bool commandMatches(
        const QString& command,
        const QString& executablePath,
        const QString& action,
        const QString& filePlaceholder = "%1");

    bool install(const QString& executablePath, QString* errorMessage = nullptr) const;
    bool install(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage = nullptr) const;
    bool uninstall(QString* errorMessage = nullptr) const;
    bool isInstalled() const;
    bool isInstalledForExecutable(const QString& executablePath) const;
    QList<ShellIntegrationExtensionStatus> status(const QString& executablePath = QString(), const ShellMenuSettings& settings = ShellMenuSettings()) const;

private:
    bool installForExtension(const QString& extension, const QString& executablePath, QString* errorMessage) const;
    bool installForFile(const QString& executablePath, QString* errorMessage) const;
    bool installForDirectory(const QString& executablePath, QString* errorMessage) const;
    bool installForDirectoryBackground(const QString& executablePath, QString* errorMessage) const;
    bool installArchiveMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const;
    bool installFileMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const;
    bool installDirectoryMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const;
    bool installDirectoryBackgroundMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const;
    bool uninstallForExtension(const QString& extension, QString* errorMessage) const;
    bool uninstallForFile(QString* errorMessage) const;
    bool uninstallForDirectory(QString* errorMessage) const;
    bool uninstallForDirectoryBackground(QString* errorMessage) const;
    bool uninstallMenuClass(const QString& classKey, QString* errorMessage) const;
    bool uninstallLegacyRoots(QString* errorMessage) const;
};

} // namespace PasswordManager
