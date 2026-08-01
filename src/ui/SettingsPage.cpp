#include "PasswordManager/ui/SettingsPage.h"

#include "PasswordManager/app/AppConfig.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ArchiveFingerprintService.h"
#include "PasswordManager/app/DiagnosticService.h"
#include "PasswordManager/app/PerformanceBenchmarkService.h"
#include "PasswordManager/app/SevenZipProbe.h"
#include "PasswordManager/app/ShellIntegration.h"
#include "PasswordManager/data/DatabaseBackupService.h"

#include <QCoreApplication>
#include <QCheckBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStringList>
#include <QUrl>
#include <QVBoxLayout>

#include <algorithm>
#include <utility>

namespace PasswordManager {

SettingsPage::SettingsPage(const AppPaths& paths, const ArchiveRepository& archiveRepository, QString databaseConnectionName, QWidget* parent)
    : QWidget(parent)
    , m_paths(paths)
    , m_archiveRepository(archiveRepository)
    , m_databaseConnectionName(std::move(databaseConnectionName))
{
    buildUi();
    loadFeatureSettings();
    refreshSevenZipStatus();
    refreshShellIntegrationStatus();
}

void SettingsPage::buildUi()
{
    auto* pageLayout = new QVBoxLayout(this);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(0);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* content = new QWidget(scrollArea);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(14);

    auto* title = new QLabel("设置", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title);

    addSectionTitle(layout, "运行目录");
    addKeyValue(layout, "程序目录", m_paths.applicationDir());
    addKeyValue(layout, "数据目录", m_paths.dataDir());
    addKeyValue(layout, "配置目录", m_paths.configDir());
    addKeyValue(layout, "日志目录", m_paths.logsDir());
    addKeyValue(layout, "备份目录", m_paths.backupDir());
    addKeyValue(layout, "工具目录", m_paths.toolsDir());

    auto* directoryActions = new QGridLayout;
    directoryActions->setHorizontalSpacing(8);
    directoryActions->setVerticalSpacing(8);
    auto* openApplicationButton = new QPushButton("打开程序目录", this);
    auto* openDataButton = new QPushButton("打开数据目录", this);
    auto* openBackupButton = new QPushButton("打开备份目录", this);
    auto* openLogsButton = new QPushButton("打开日志目录", this);
    auto* openManualButton = new QPushButton("打开用户手册", this);
    auto* openTestDataButton = new QPushButton("打开测试数据", this);
    auto* exportDiagnosticButton = new QPushButton("导出诊断包", this);
    auto* runBenchmarkButton = new QPushButton("运行性能基准", this);
    connect(openApplicationButton, &QPushButton::clicked, this, [this]() {
        openDirectory(m_paths.applicationDir());
    });
    connect(openDataButton, &QPushButton::clicked, this, [this]() {
        openDirectory(m_paths.dataDir());
    });
    connect(openBackupButton, &QPushButton::clicked, this, [this]() {
        openDirectory(m_paths.backupDir());
    });
    connect(openLogsButton, &QPushButton::clicked, this, [this]() {
        openDirectory(m_paths.logsDir());
    });
    connect(openManualButton, &QPushButton::clicked, this, [this]() {
        const QString portableManual = QDir(m_paths.applicationDir()).filePath("USER_MANUAL.md");
        if (QFileInfo::exists(portableManual)) {
            openFile(portableManual);
            return;
        }
        const QString userManual = resolveProjectPath("docs/46_User_Manual.md");
        if (QFileInfo::exists(userManual)) {
            openFile(userManual);
            return;
        }
        openFile(QDir(m_paths.applicationDir()).filePath("README_RELEASE.txt"));
    });
    connect(openTestDataButton, &QPushButton::clicked, this, [this]() {
        const QString testDataDir = resolveProjectPath("testdata");
        if (QDir(testDataDir).exists()) {
            openDirectory(testDataDir);
            return;
        }
        QMessageBox::information(this, "PasswordManager", "发布包内不包含项目测试数据。请在开发项目目录中打开 testdata。");
    });
    connect(exportDiagnosticButton, &QPushButton::clicked, this, [this]() {
        QString outputDirectory;
        QString error;
        if (!DiagnosticService(m_paths, m_databaseConnectionName).exportDiagnosticPackage(&outputDirectory, &error)) {
            QMessageBox::warning(this, "PasswordManager", error.isEmpty() ? "导出诊断包失败。" : error);
            return;
        }
        QMessageBox::information(this, "PasswordManager", "诊断包已导出：\n" + outputDirectory);
        openDirectory(outputDirectory);
    });
    connect(runBenchmarkButton, &QPushButton::clicked, this, [this]() {
        const QString scanDirectory = QFileDialog::getExistingDirectory(this, "选择可选扫描目录；取消则只测试当前数据库");
        QString outputPath;
        QString error;
        if (!PerformanceBenchmarkService(m_paths, m_databaseConnectionName).run(scanDirectory, &outputPath, &error)) {
            QMessageBox::warning(this, "PasswordManager", error.isEmpty() ? "性能基准运行失败。" : error);
            return;
        }
        QMessageBox::information(this, "PasswordManager", "性能基准报告已生成：\n" + outputPath);
    });
    directoryActions->addWidget(openApplicationButton, 0, 0);
    directoryActions->addWidget(openDataButton, 0, 1);
    directoryActions->addWidget(openBackupButton, 0, 2);
    directoryActions->addWidget(openLogsButton, 0, 3);
    directoryActions->addWidget(openManualButton, 1, 0);
    directoryActions->addWidget(openTestDataButton, 1, 1);
    directoryActions->addWidget(exportDiagnosticButton, 1, 2);
    directoryActions->addWidget(runBenchmarkButton, 1, 3);
    for (int column = 0; column < 4; ++column) {
        directoryActions->setColumnStretch(column, 1);
    }
    layout->addLayout(directoryActions);

    addSectionTitle(layout, "内置 7-Zip");
    addKeyValue(layout, "执行文件", m_paths.sevenZipExecutable());

    m_sevenZipStatus = new QLabel(this);
    m_sevenZipVersion = new QLabel(this);
    m_sevenZipStatus->setWordWrap(true);
    m_sevenZipVersion->setWordWrap(true);
    m_sevenZipStatus->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_sevenZipVersion->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    auto* statusFrame = new QFrame(this);
    statusFrame->setObjectName("card");
    auto* statusLayout = new QGridLayout(statusFrame);
    statusLayout->setContentsMargins(14, 12, 14, 12);
    statusLayout->setHorizontalSpacing(16);
    statusLayout->setVerticalSpacing(8);
    statusLayout->addWidget(new QLabel("状态", statusFrame), 0, 0);
    statusLayout->addWidget(m_sevenZipStatus, 0, 1);
    statusLayout->addWidget(new QLabel("版本", statusFrame), 1, 0);
    statusLayout->addWidget(m_sevenZipVersion, 1, 1);

    auto* refreshButton = new QPushButton("重新检查", statusFrame);
    connect(refreshButton, &QPushButton::clicked, this, &SettingsPage::refreshSevenZipStatus);
    statusLayout->addWidget(refreshButton, 2, 0, 1, 2);
    statusLayout->setColumnStretch(1, 1);
    layout->addWidget(statusFrame);

    addSectionTitle(layout, "Windows 右键菜单");
    m_shellStatus = new QLabel(this);
    m_shellStatus->setWordWrap(true);
    m_shellStatus->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    auto* shellFrame = new QFrame(this);
    shellFrame->setObjectName("card");
    auto* shellLayout = new QGridLayout(shellFrame);
    shellLayout->setContentsMargins(14, 12, 14, 12);
    shellLayout->setHorizontalSpacing(16);
    shellLayout->setVerticalSpacing(8);
    shellLayout->addWidget(new QLabel("状态", shellFrame), 0, 0);
    shellLayout->addWidget(m_shellStatus, 0, 1);

    auto* installButton = new QPushButton("安装", shellFrame);
    auto* repairButton = new QPushButton("重新安装/修复", shellFrame);
    auto* uninstallButton = new QPushButton("卸载", shellFrame);
    auto* shellRefreshButton = new QPushButton("刷新", shellFrame);
    connect(installButton, &QPushButton::clicked, this, &SettingsPage::installShellIntegration);
    connect(repairButton, &QPushButton::clicked, this, &SettingsPage::repairShellIntegration);
    connect(uninstallButton, &QPushButton::clicked, this, &SettingsPage::uninstallShellIntegration);
    connect(shellRefreshButton, &QPushButton::clicked, this, &SettingsPage::refreshShellIntegrationStatus);
    shellLayout->addWidget(installButton, 1, 0);
    shellLayout->addWidget(repairButton, 1, 1);
    shellLayout->addWidget(uninstallButton, 2, 0);
    shellLayout->addWidget(shellRefreshButton, 2, 1);
    shellLayout->setColumnStretch(1, 1);
    layout->addWidget(shellFrame);

    addSectionTitle(layout, "智能匹配");
    auto* smartFrame = new QFrame(this);
    smartFrame->setObjectName("card");
    auto* smartLayout = new QGridLayout(smartFrame);
    smartLayout->setContentsMargins(14, 12, 14, 12);
    smartLayout->setHorizontalSpacing(16);
    smartLayout->setVerticalSpacing(8);

    m_enableExactHistory = new QCheckBox("当前压缩包历史优先", smartFrame);
    m_enableDirectoryHistory = new QCheckBox("同目录历史候选", smartFrame);
    m_enableCategoryCandidates = new QCheckBox("同分类密码候选", smartFrame);
    m_enablePasswordLibrary = new QCheckBox("密码库候选", smartFrame);
    m_enableDescriptionFiles = new QCheckBox("同目录说明文件候选", smartFrame);
    m_calculateFullHashDuringScan = new QCheckBox("扫描时计算完整文件指纹", smartFrame);
    m_maxCandidates = new QSpinBox(smartFrame);
    m_maxCandidates->setRange(1, 500);
    m_maxDescriptionCandidates = new QSpinBox(smartFrame);
    m_maxDescriptionCandidates->setRange(0, 100);
    m_maxDescriptionFileKb = new QSpinBox(smartFrame);
    m_maxDescriptionFileKb->setRange(1, 1024);
    m_maxDescriptionFileKb->setSuffix(" KB");

    smartLayout->addWidget(m_enableExactHistory, 0, 0);
    smartLayout->addWidget(m_enableDirectoryHistory, 0, 1);
    smartLayout->addWidget(m_enableCategoryCandidates, 1, 0);
    smartLayout->addWidget(m_enablePasswordLibrary, 1, 1);
    smartLayout->addWidget(m_enableDescriptionFiles, 2, 0);
    smartLayout->addWidget(m_calculateFullHashDuringScan, 2, 1);
    smartLayout->addWidget(new QLabel("最大候选数", smartFrame), 3, 0);
    smartLayout->addWidget(m_maxCandidates, 3, 1);
    smartLayout->addWidget(new QLabel("说明文件候选数", smartFrame), 4, 0);
    smartLayout->addWidget(m_maxDescriptionCandidates, 4, 1);
    smartLayout->addWidget(new QLabel("单个说明文件读取上限", smartFrame), 5, 0);
    smartLayout->addWidget(m_maxDescriptionFileKb, 5, 1);
    smartLayout->setColumnStretch(1, 1);
    layout->addWidget(smartFrame);

    addSectionTitle(layout, "右键菜单功能项");
    auto* shellOptionsFrame = new QFrame(this);
    shellOptionsFrame->setObjectName("card");
    auto* shellOptionsLayout = new QGridLayout(shellOptionsFrame);
    shellOptionsLayout->setContentsMargins(14, 12, 14, 12);
    shellOptionsLayout->setHorizontalSpacing(16);
    shellOptionsLayout->setVerticalSpacing(8);

    m_shellArchiveLookup = new QCheckBox("压缩包：自动查找密码", shellOptionsFrame);
    m_shellArchiveTest = new QCheckBox("压缩包：使用密码库测试", shellOptionsFrame);
    m_shellArchiveViewResults = new QCheckBox("压缩包：查看结果", shellOptionsFrame);
    m_shellArchiveExtract = new QCheckBox("压缩包：解压", shellOptionsFrame);
    m_shellArchiveCompress = new QCheckBox("压缩包：打包压缩包", shellOptionsFrame);
    m_shellArchiveOpenMain = new QCheckBox("压缩包：打开主程序", shellOptionsFrame);
    m_shellFolderScan = new QCheckBox("文件夹：扫描文件夹", shellOptionsFrame);
    m_shellFolderCompress = new QCheckBox("文件夹：打包压缩包", shellOptionsFrame);
    m_shellFolderOpenMain = new QCheckBox("文件夹：打开主程序", shellOptionsFrame);
    m_shellFileCompress = new QCheckBox("普通文件：打包压缩包", shellOptionsFrame);

    shellOptionsLayout->addWidget(m_shellArchiveLookup, 0, 0);
    shellOptionsLayout->addWidget(m_shellArchiveTest, 0, 1);
    shellOptionsLayout->addWidget(m_shellArchiveViewResults, 1, 0);
    shellOptionsLayout->addWidget(m_shellArchiveExtract, 1, 1);
    shellOptionsLayout->addWidget(m_shellArchiveCompress, 2, 0);
    shellOptionsLayout->addWidget(m_shellArchiveOpenMain, 2, 1);
    shellOptionsLayout->addWidget(m_shellFolderScan, 3, 0);
    shellOptionsLayout->addWidget(m_shellFolderCompress, 3, 1);
    shellOptionsLayout->addWidget(m_shellFolderOpenMain, 4, 0);
    shellOptionsLayout->addWidget(m_shellFileCompress, 4, 1);
    auto* shellOptionsHint = new QLabel("右键菜单功能项设置会保存到本地配置；重新安装/修复右键菜单后生效。", shellOptionsFrame);
    shellOptionsHint->setWordWrap(true);
    shellOptionsLayout->addWidget(shellOptionsHint, 5, 0, 1, 2);
    layout->addWidget(shellOptionsFrame);

    auto* saveFeatureButton = new QPushButton("保存功能设置", this);
    connect(saveFeatureButton, &QPushButton::clicked, this, &SettingsPage::saveFeatureSettings);
    layout->addWidget(saveFeatureButton);

    addSectionTitle(layout, "数据备份与恢复");
    auto* backupFrame = new QFrame(this);
    backupFrame->setObjectName("card");
    auto* backupLayout = new QGridLayout(backupFrame);
    backupLayout->setContentsMargins(14, 12, 14, 12);
    backupLayout->setHorizontalSpacing(16);
    backupLayout->setVerticalSpacing(8);
    auto* backupPathLabel = new QLabel(m_paths.backupDir(), backupFrame);
    backupPathLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    backupPathLabel->setWordWrap(true);
    backupPathLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    backupLayout->addWidget(new QLabel("备份目录", backupFrame), 0, 0);
    backupLayout->addWidget(backupPathLabel, 0, 1);

    auto* backupButton = new QPushButton("立即备份", backupFrame);
    auto* restoreButton = new QPushButton("从备份恢复", backupFrame);
    auto* fingerprintButton = new QPushButton("补全文件指纹", backupFrame);
    connect(backupButton, &QPushButton::clicked, this, &SettingsPage::createDatabaseBackup);
    connect(restoreButton, &QPushButton::clicked, this, &SettingsPage::restoreDatabaseBackup);
    connect(fingerprintButton, &QPushButton::clicked, this, &SettingsPage::backfillArchiveFingerprints);
    backupLayout->addWidget(backupButton, 1, 0);
    backupLayout->addWidget(restoreButton, 1, 1);
    backupLayout->addWidget(fingerprintButton, 2, 0, 1, 2);
    backupLayout->setColumnStretch(1, 1);
    layout->addWidget(backupFrame);

    layout->addStretch();
    scrollArea->setWidget(content);
    pageLayout->addWidget(scrollArea);
}

void SettingsPage::addSectionTitle(QVBoxLayout* layout, const QString& title)
{
    auto* label = new QLabel(title, this);
    QFont font = label->font();
    font.setBold(true);
    label->setFont(font);
    layout->addWidget(label);
}

void SettingsPage::addKeyValue(QVBoxLayout* layout, const QString& key, const QString& value)
{
    auto* frame = new QFrame(this);
    frame->setObjectName("card");

    auto* row = new QGridLayout(frame);
    row->setContentsMargins(14, 10, 14, 10);
    row->setHorizontalSpacing(16);

    auto* keyLabel = new QLabel(key, frame);
    auto* valueLabel = new QLabel(value, frame);
    valueLabel->setObjectName("value");
    valueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel->setWordWrap(true);
    valueLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    row->addWidget(keyLabel, 0, 0);
    row->addWidget(valueLabel, 0, 1);
    row->setColumnStretch(1, 1);

    layout->addWidget(frame);
}

void SettingsPage::refreshSevenZipStatus()
{
    const SevenZipStatus status = SevenZipProbe().probe(m_paths.sevenZipExecutable());
    if (status.exists && status.runnable) {
        m_sevenZipStatus->setText("可用");
    } else if (!status.exists) {
        m_sevenZipStatus->setText("缺失");
    } else {
        m_sevenZipStatus->setText("不可用");
    }

    if (!status.errorMessage.isEmpty() && !status.runnable) {
        m_sevenZipVersion->setText(status.errorMessage);
    } else {
        m_sevenZipVersion->setText(status.version.isEmpty() ? "未识别" : status.version);
    }
}

void SettingsPage::refreshShellIntegrationStatus()
{
    const ShellIntegration integration;
    const QList<ShellIntegrationExtensionStatus> entries = integration.status(QCoreApplication::applicationFilePath(), AppConfig(m_paths).shellMenuSettings());
    int completeCount = 0;
    QStringList details;
    for (const ShellIntegrationExtensionStatus& entry : entries) {
        const bool complete = entry.rootInstalled
            && entry.addQueueInstalled
            && entry.viewResultsInstalled
            && entry.commandsPointToExecutable;
        if (complete) {
            ++completeCount;
        }

        QString state = "未安装";
        if (complete) {
            state = "正常";
        } else if (entry.rootInstalled || entry.addQueueInstalled || entry.viewResultsInstalled) {
            state = entry.commandsPointToExecutable ? "不完整" : "路径不一致或不完整";
        }
        details.append(QString("%1：%2").arg(entry.extension, state));
    }

    const QString summary = completeCount == entries.size()
        ? "已安装，命令指向当前程序"
        : (completeCount == 0 ? "未完整安装" : "部分安装");
    m_shellStatus->setText(summary + "\n" + details.join("\n"));
}

void SettingsPage::loadFeatureSettings()
{
    const AppConfig config(m_paths);
    const SmartMatchSettings smart = config.smartMatchSettings();
    m_enableExactHistory->setChecked(smart.enableExactHistory);
    m_enableDirectoryHistory->setChecked(smart.enableDirectoryHistory);
    m_enableCategoryCandidates->setChecked(smart.enableCategoryCandidates);
    m_enablePasswordLibrary->setChecked(smart.enablePasswordLibrary);
    m_enableDescriptionFiles->setChecked(smart.enableDescriptionFiles);
    m_calculateFullHashDuringScan->setChecked(smart.calculateFullHashDuringScan);
    m_maxCandidates->setValue(smart.maxCandidates);
    m_maxDescriptionCandidates->setValue(smart.maxDescriptionCandidates);
    m_maxDescriptionFileKb->setValue(std::max(1, smart.maxDescriptionFileBytes / 1024));

    const ShellMenuSettings shell = config.shellMenuSettings();
    m_shellArchiveLookup->setChecked(shell.enableArchiveLookup);
    m_shellArchiveTest->setChecked(shell.enableArchiveTest);
    m_shellArchiveViewResults->setChecked(shell.enableArchiveViewResults);
    m_shellArchiveExtract->setChecked(shell.enableArchiveExtract);
    m_shellArchiveCompress->setChecked(shell.enableArchiveCompress);
    m_shellArchiveOpenMain->setChecked(shell.enableArchiveOpenMain);
    m_shellFolderScan->setChecked(shell.enableFolderScan);
    m_shellFolderCompress->setChecked(shell.enableFolderCompress);
    m_shellFolderOpenMain->setChecked(shell.enableFolderOpenMain);
    m_shellFileCompress->setChecked(shell.enableFileCompress);
}

void SettingsPage::saveFeatureSettings()
{
    SmartMatchSettings smart;
    smart.enableExactHistory = m_enableExactHistory->isChecked();
    smart.enableDirectoryHistory = m_enableDirectoryHistory->isChecked();
    smart.enableCategoryCandidates = m_enableCategoryCandidates->isChecked();
    smart.enablePasswordLibrary = m_enablePasswordLibrary->isChecked();
    smart.enableDescriptionFiles = m_enableDescriptionFiles->isChecked();
    smart.calculateFullHashDuringScan = m_calculateFullHashDuringScan->isChecked();
    smart.maxCandidates = m_maxCandidates->value();
    smart.maxDescriptionCandidates = m_maxDescriptionCandidates->value();
    smart.maxDescriptionFileBytes = m_maxDescriptionFileKb->value() * 1024;

    ShellMenuSettings shell;
    shell.enableArchiveLookup = m_shellArchiveLookup->isChecked();
    shell.enableArchiveTest = m_shellArchiveTest->isChecked();
    shell.enableArchiveViewResults = m_shellArchiveViewResults->isChecked();
    shell.enableArchiveExtract = m_shellArchiveExtract->isChecked();
    shell.enableArchiveCompress = m_shellArchiveCompress->isChecked();
    shell.enableArchiveOpenMain = m_shellArchiveOpenMain->isChecked();
    shell.enableFolderScan = m_shellFolderScan->isChecked();
    shell.enableFolderCompress = m_shellFolderCompress->isChecked();
    shell.enableFolderOpenMain = m_shellFolderOpenMain->isChecked();
    shell.enableFileCompress = m_shellFileCompress->isChecked();

    AppConfig config(m_paths);
    config.saveSmartMatchSettings(smart);
    config.saveShellMenuSettings(shell);
    QMessageBox::information(this, "PasswordManager", "功能设置已保存。智能匹配立即生效；右键菜单功能项请执行重新安装/修复后生效。");
}

void SettingsPage::openDirectory(const QString& path)
{
    if (!QDir().mkpath(path)) {
        QMessageBox::warning(this, "PasswordManager", "无法创建或打开目录：\n" + path);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void SettingsPage::openFile(const QString& path)
{
    if (!QFileInfo::exists(path)) {
        QMessageBox::warning(this, "PasswordManager", "文件不存在：\n" + path);
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

QString SettingsPage::resolveProjectPath(const QString& relativePath) const
{
    const QString applicationDir = m_paths.applicationDir();
    const QStringList roots = {
        applicationDir,
        QDir(applicationDir).absoluteFilePath(".."),
        QDir(applicationDir).absoluteFilePath("../.."),
    };

    for (const QString& root : roots) {
        const QString candidate = QDir(root).absoluteFilePath(relativePath);
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }
    return QDir(applicationDir).absoluteFilePath(relativePath);
}

void SettingsPage::createDatabaseBackup()
{
    QString backupPath;
    QString error;
    if (!DatabaseBackupService(m_databaseConnectionName, QDir(m_paths.dataDir()).filePath("passwordmanager.sqlite3"), m_paths.backupDir())
            .createBackup(&backupPath, &error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "数据库备份失败。" : error);
        return;
    }

    QMessageBox::information(this, "PasswordManager", "备份完成：\n" + backupPath);
}

void SettingsPage::restoreDatabaseBackup()
{
    const QString sourcePath = QFileDialog::getOpenFileName(
        this,
        "选择数据库备份",
        m_paths.backupDir(),
        "SQLite 数据库 (*.sqlite3);;所有文件 (*.*)");
    if (sourcePath.isEmpty()) {
        return;
    }

    if (QMessageBox::question(
            this,
            "PasswordManager",
            "恢复会替换当前数据库。程序会先自动备份当前数据库，恢复完成后需要重新打开程序。确认继续吗？")
        != QMessageBox::Yes) {
        return;
    }

    QString safetyBackupPath;
    QString error;
    if (!DatabaseBackupService(m_databaseConnectionName, QDir(m_paths.dataDir()).filePath("passwordmanager.sqlite3"), m_paths.backupDir())
            .restoreFromBackup(sourcePath, &safetyBackupPath, &error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "数据库恢复失败。" : error);
        return;
    }

    QMessageBox::information(this, "PasswordManager", "恢复完成。当前数据库已备份到：\n" + safetyBackupPath + "\n\n程序现在将退出，请重新打开。");
    QCoreApplication::quit();
}

void SettingsPage::backfillArchiveFingerprints()
{
    if (QMessageBox::question(
            this,
            "PasswordManager",
            "将为数据库中缺少完整文件指纹的压缩包记录计算 SHA256。\n\n不会删除记录，不会修改密码库和历史记录。大文件可能耗时较长。确认开始吗？")
        != QMessageBox::Yes) {
        return;
    }

    const FingerprintBackfillResult result = ArchiveFingerprintService(m_archiveRepository).backfillMissingFullHashes();
    QString message = QString("文件指纹补全完成。\n\n待补全记录：%1\n已补全：%2\n文件不存在：%3\n失败：%4")
                          .arg(result.totalMissing)
                          .arg(result.updated)
                          .arg(result.missingFiles)
                          .arg(result.failed);
    if (!result.lastError.isEmpty()) {
        message += "\n\n最后错误：\n" + result.lastError;
    }
    QMessageBox::information(this, "PasswordManager", message);
}

void SettingsPage::installShellIntegration()
{
    if (QMessageBox::question(this, "PasswordManager", "确认安装 Windows 资源管理器右键菜单吗？") != QMessageBox::Yes) {
        return;
    }

    QString error;
    const ShellIntegration integration;
    integration.uninstall(&error);
    error.clear();
    if (!integration.install(QCoreApplication::applicationFilePath(), AppConfig(m_paths).shellMenuSettings(), &error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "右键菜单安装失败。" : error);
        return;
    }

    refreshShellIntegrationStatus();
    QMessageBox::information(this, "PasswordManager", "右键菜单已安装。");
}

void SettingsPage::repairShellIntegration()
{
    if (QMessageBox::question(this, "PasswordManager", "确认重新安装/修复 Windows 资源管理器右键菜单吗？\n\n这会把右键菜单命令更新为当前程序路径。") != QMessageBox::Yes) {
        return;
    }

    QString error;
    const ShellIntegration integration;
    if (!integration.uninstall(&error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "右键菜单清理失败。" : error);
        return;
    }
    if (!integration.install(QCoreApplication::applicationFilePath(), AppConfig(m_paths).shellMenuSettings(), &error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "右键菜单修复失败。" : error);
        return;
    }

    refreshShellIntegrationStatus();
    QMessageBox::information(this, "PasswordManager", "右键菜单已重新安装/修复。");
}

void SettingsPage::uninstallShellIntegration()
{
    if (QMessageBox::question(this, "PasswordManager", "确认卸载 Windows 资源管理器右键菜单吗？") != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!ShellIntegration().uninstall(&error)) {
        QMessageBox::critical(this, "PasswordManager", error.isEmpty() ? "右键菜单卸载失败。" : error);
        return;
    }

    refreshShellIntegrationStatus();
    QMessageBox::information(this, "PasswordManager", "右键菜单已卸载。");
}

} // namespace PasswordManager
