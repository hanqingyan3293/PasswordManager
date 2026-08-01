#include "PasswordManager/app/MainWindow.h"

#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/app/ShellActionService.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/ExtractLogRepository.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/ui/HistoryPage.h"
#include "PasswordManager/ui/HomePage.h"
#include "PasswordManager/ui/PasswordsPage.h"
#include "PasswordManager/ui/SettingsPage.h"
#include "PasswordManager/ui/TaskQueuePage.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <utility>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace PasswordManager {

MainWindow::MainWindow(
    const AppPaths& paths,
    const ArchiveRepository& archiveRepository,
    const ArchivePasswordRepository& archivePasswordRepository,
    const ExtractLogRepository& extractLogRepository,
    const PasswordRepository& passwordRepository,
    QString databaseConnectionName,
    PasswordTestTaskManager& taskManager,
    QWidget* parent)
    : QMainWindow(parent)
    , m_paths(paths)
    , m_archiveRepository(archiveRepository)
    , m_archivePasswordRepository(archivePasswordRepository)
    , m_extractLogRepository(extractLogRepository)
    , m_passwordRepository(passwordRepository)
    , m_databaseConnectionName(std::move(databaseConnectionName))
    , m_taskManager(taskManager)
{
    buildUi();
    applyBaseStyle();
}

QWidget* MainWindow::createPage(const QString& title, const QString& description)
{
    auto* page = new QWidget(this);
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(8);

    auto* titleLabel = new QLabel(title, page);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto* descriptionLabel = new QLabel(description, page);
    descriptionLabel->setWordWrap(true);

    layout->addWidget(titleLabel);
    layout->addWidget(descriptionLabel);
    layout->addStretch();
    return page;
}

void MainWindow::buildUi()
{
    setWindowTitle("PasswordManager");
    resize(960, 640);
    setMinimumSize(560, 360);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigation = new QListWidget(central);
    m_navigation->setMinimumWidth(108);
    m_navigation->setMaximumWidth(160);
    m_navigation->addItems({"首页", "密码库", "历史记录", "解压队列", "设置"});

    m_pages = new QStackedWidget(central);
    m_homePage = new HomePage(m_paths, m_archiveRepository, m_archivePasswordRepository, m_passwordRepository, m_taskManager, this);
    m_passwordsPage = new PasswordsPage(m_passwordRepository, this);
    m_historyPage = new HistoryPage(m_paths, m_archivePasswordRepository, m_extractLogRepository, this);
    m_taskQueuePage = new TaskQueuePage(m_taskManager, this);
    m_pages->addWidget(m_homePage);
    m_pages->addWidget(m_passwordsPage);
    m_pages->addWidget(m_historyPage);
    m_pages->addWidget(m_taskQueuePage);
    m_pages->addWidget(new SettingsPage(m_paths, m_archiveRepository, m_databaseConnectionName, this));

    layout->addWidget(m_navigation);
    layout->addWidget(m_pages, 1);

    connect(m_navigation, &QListWidget::currentRowChanged, m_pages, &QStackedWidget::setCurrentIndex);
    connect(m_pages, &QStackedWidget::currentChanged, this, [this](int) {
        refreshCurrentPage();
    });
    connect(m_historyPage, &HistoryPage::passwordRecordRequested, this, [this](int passwordId, const QString& password) {
        selectPage(1);
        m_passwordsPage->focusPassword(passwordId, password);
        statusBar()->showMessage("已定位密码库记录：" + password);
    });
    m_navigation->setCurrentRow(0);
    QTimer::singleShot(0, this, [this]() {
        refreshCurrentPage();
    });

    setCentralWidget(central);
    statusBar()->showMessage("就绪");
}

QSize MainWindow::minimumSizeHint() const
{
    return QSize(560, 360);
}

void MainWindow::openShellAction(const QString& action, const QString& filePath)
{
    bringToFront();

    if (action == "view-results") {
        selectPage(2);
        m_historyPage->focusArchivePath(filePath);
        statusBar()->showMessage("已按压缩包路径过滤历史记录：" + filePath);
        showShellMessage(QMessageBox::Information, "PasswordManager", "已打开历史记录并按压缩包路径过滤。\n" + filePath);
        return;
    }

    if (action == "add-test-queue" || action == "use-password-library-test") {
        const ShellActionResult result = ShellActionService(
            m_paths,
            m_archiveRepository,
            m_archivePasswordRepository,
            m_passwordRepository,
            m_taskManager).enqueueArchivePasswordTests(filePath);

        if (result.success) {
            selectPage(3);
            statusBar()->showMessage(QString("已加入 %1 个密码测试任务：%2").arg(result.enqueuedCount).arg(filePath));
        } else {
            selectPage(0);
            statusBar()->showMessage(result.message + " " + filePath);
        }
        showShellMessage(result.success ? QMessageBox::Information : QMessageBox::Warning, "PasswordManager", result.message + "\n" + filePath);
        return;
    }

    if (action == "lookup-password") {
        const ShellActionResult result = ShellActionService(
            m_paths,
            m_archiveRepository,
            m_archivePasswordRepository,
            m_passwordRepository,
            m_taskManager).lookupKnownPasswords(filePath);

        selectPage(result.knownPasswordCount > 0 ? 2 : 0);
        if (result.knownPasswordCount > 0) {
            m_historyPage->focusArchivePath(filePath);
        }
        statusBar()->showMessage(result.message + " " + filePath);
        showShellMessage(QMessageBox::Information, "PasswordManager", result.message + "\n" + filePath);
        return;
    }

    if (action == "scan-folder") {
        const ShellActionResult result = ShellActionService(
            m_paths,
            m_archiveRepository,
            m_archivePasswordRepository,
            m_passwordRepository,
            m_taskManager).scanFolder(filePath);

        selectPage(0);
        statusBar()->showMessage(result.message + " " + filePath);
        showShellMessage(result.success ? QMessageBox::Information : QMessageBox::Warning, "PasswordManager", result.message + "\n" + filePath);
        return;
    }

    if (action == "open-main") {
        selectPage(0);
        statusBar()->showMessage("已打开主程序");
        return;
    }

    selectPage(0);
    showShellMessage(QMessageBox::Warning, "PasswordManager", "未知右键菜单动作：" + action);
}

void MainWindow::bringToFront()
{
    showNormal();
    raise();
    activateWindow();

#ifdef Q_OS_WIN
    HWND handle = reinterpret_cast<HWND>(winId());
    if (handle) {
        ShowWindow(handle, SW_SHOWNORMAL);
        SetForegroundWindow(handle);
    }
#endif
}

void MainWindow::showShellMessage(QMessageBox::Icon icon, const QString& title, const QString& message)
{
    bringToFront();

    QMessageBox box(icon, title, message, QMessageBox::Ok, this);
    box.setWindowModality(Qt::ApplicationModal);
    box.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    box.raise();
    box.activateWindow();
    box.exec();

    bringToFront();
}

void MainWindow::selectPage(int index)
{
    if (index < 0 || index >= m_pages->count()) {
        return;
    }
    m_navigation->setCurrentRow(index);
}

void MainWindow::refreshCurrentPage()
{
    if (!m_pages) {
        return;
    }

    QWidget* page = m_pages->currentWidget();
    if (page == m_homePage && m_homePage) {
        m_homePage->reload();
        return;
    }
    if (page == m_passwordsPage && m_passwordsPage) {
        m_passwordsPage->reload();
        return;
    }
    if (page == m_historyPage && m_historyPage) {
        m_historyPage->reload();
        return;
    }
    if (page == m_taskQueuePage && m_taskQueuePage) {
        m_taskQueuePage->reload();
    }
}

void MainWindow::applyBaseStyle()
{
    setStyleSheet(R"(
        QMainWindow {
            background: #f6f7f9;
        }
        QListWidget {
            background: #ffffff;
            border: none;
            border-right: 1px solid #dfe3e8;
            padding: 10px;
            font-size: 14px;
        }
        QListWidget::item {
            border-radius: 6px;
            margin: 2px 0;
            padding: 9px 10px;
        }
        QListWidget::item:selected {
            background: #e8f0fe;
            color: #174ea6;
        }
        QLabel {
            color: #1f2933;
            font-size: 14px;
        }
        QFrame#card {
            background: #ffffff;
            border: 1px solid #e1e5ea;
            border-radius: 8px;
        }
        QPushButton {
            background: #1f6feb;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 8px 14px;
        }
        QPushButton:hover {
            background: #1a5fd0;
        }
        QStatusBar {
            background: #ffffff;
            border-top: 1px solid #dfe3e8;
        }
    )");
}

} // namespace PasswordManager
