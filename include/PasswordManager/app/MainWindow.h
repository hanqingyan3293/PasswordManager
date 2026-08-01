#pragma once

#include <QMainWindow>
#include <QMessageBox>
#include <QSize>
#include <QString>

class QListWidget;
class QStackedWidget;

namespace PasswordManager {

class AppPaths;
class ArchivePasswordRepository;
class ArchiveRepository;
class ExtractLogRepository;
class PasswordRepository;
class PasswordTestTaskManager;
class HomePage;
class HistoryPage;
class PasswordsPage;
class TaskQueuePage;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        const AppPaths& paths,
        const ArchiveRepository& archiveRepository,
        const ArchivePasswordRepository& archivePasswordRepository,
        const ExtractLogRepository& extractLogRepository,
        const PasswordRepository& passwordRepository,
        QString databaseConnectionName,
        PasswordTestTaskManager& taskManager,
        QWidget* parent = nullptr);

    void openShellAction(const QString& action, const QString& filePath);
    QSize minimumSizeHint() const override;

private:
    QWidget* createPage(const QString& title, const QString& description);
    void buildUi();
    void applyBaseStyle();
    void bringToFront();
    void showShellMessage(QMessageBox::Icon icon, const QString& title, const QString& message);
    void selectPage(int index);
    void refreshCurrentPage();

    const AppPaths& m_paths;
    const ArchiveRepository& m_archiveRepository;
    const ArchivePasswordRepository& m_archivePasswordRepository;
    const ExtractLogRepository& m_extractLogRepository;
    const PasswordRepository& m_passwordRepository;
    QString m_databaseConnectionName;
    PasswordTestTaskManager& m_taskManager;

    QListWidget* m_navigation = nullptr;
    QStackedWidget* m_pages = nullptr;
    HomePage* m_homePage = nullptr;
    PasswordsPage* m_passwordsPage = nullptr;
    HistoryPage* m_historyPage = nullptr;
    TaskQueuePage* m_taskQueuePage = nullptr;
};

} // namespace PasswordManager
