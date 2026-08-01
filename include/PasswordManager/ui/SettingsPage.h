#pragma once

#include <QString>
#include <QWidget>

class QCheckBox;
class QLabel;
class QSpinBox;
class QVBoxLayout;

namespace PasswordManager {

class AppPaths;
class ArchiveRepository;

class SettingsPage final : public QWidget {
    Q_OBJECT

public:
    SettingsPage(const AppPaths& paths, const ArchiveRepository& archiveRepository, QString databaseConnectionName, QWidget* parent = nullptr);

private:
    void buildUi();
    void addSectionTitle(QVBoxLayout* layout, const QString& title);
    void addKeyValue(QVBoxLayout* layout, const QString& key, const QString& value);
    void refreshSevenZipStatus();
    void refreshShellIntegrationStatus();
    void loadFeatureSettings();
    void saveFeatureSettings();
    void openDirectory(const QString& path);
    void openFile(const QString& path);
    QString resolveProjectPath(const QString& relativePath) const;
    void createDatabaseBackup();
    void restoreDatabaseBackup();
    void backfillArchiveFingerprints();
    void installShellIntegration();
    void repairShellIntegration();
    void uninstallShellIntegration();

    const AppPaths& m_paths;
    const ArchiveRepository& m_archiveRepository;
    QString m_databaseConnectionName;
    QLabel* m_sevenZipStatus = nullptr;
    QLabel* m_sevenZipVersion = nullptr;
    QLabel* m_shellStatus = nullptr;
    QCheckBox* m_enableExactHistory = nullptr;
    QCheckBox* m_enableDirectoryHistory = nullptr;
    QCheckBox* m_enableCategoryCandidates = nullptr;
    QCheckBox* m_enablePasswordLibrary = nullptr;
    QCheckBox* m_enableDescriptionFiles = nullptr;
    QCheckBox* m_calculateFullHashDuringScan = nullptr;
    QSpinBox* m_maxCandidates = nullptr;
    QSpinBox* m_maxDescriptionCandidates = nullptr;
    QSpinBox* m_maxDescriptionFileKb = nullptr;
    QCheckBox* m_shellArchiveLookup = nullptr;
    QCheckBox* m_shellArchiveTest = nullptr;
    QCheckBox* m_shellArchiveViewResults = nullptr;
    QCheckBox* m_shellArchiveExtract = nullptr;
    QCheckBox* m_shellArchiveCompress = nullptr;
    QCheckBox* m_shellArchiveOpenMain = nullptr;
    QCheckBox* m_shellFolderScan = nullptr;
    QCheckBox* m_shellFolderCompress = nullptr;
    QCheckBox* m_shellFolderOpenMain = nullptr;
    QCheckBox* m_shellFileCompress = nullptr;
};

} // namespace PasswordManager
