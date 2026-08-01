#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace PasswordManager {

class AppPaths;

class SettingsPage final : public QWidget {
    Q_OBJECT

public:
    SettingsPage(const AppPaths& paths, QString databaseConnectionName, QWidget* parent = nullptr);

private:
    void buildUi();
    void addSectionTitle(QVBoxLayout* layout, const QString& title);
    void addKeyValue(QVBoxLayout* layout, const QString& key, const QString& value);
    void refreshSevenZipStatus();
    void refreshShellIntegrationStatus();
    void openDirectory(const QString& path);
    void openFile(const QString& path);
    QString resolveProjectPath(const QString& relativePath) const;
    void createDatabaseBackup();
    void restoreDatabaseBackup();
    void installShellIntegration();
    void repairShellIntegration();
    void uninstallShellIntegration();

    const AppPaths& m_paths;
    QString m_databaseConnectionName;
    QLabel* m_sevenZipStatus = nullptr;
    QLabel* m_sevenZipVersion = nullptr;
    QLabel* m_shellStatus = nullptr;
};

} // namespace PasswordManager
