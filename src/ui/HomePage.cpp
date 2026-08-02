#include "PasswordManager/ui/HomePage.h"

#include "PasswordManager/app/AppConfig.h"
#include "PasswordManager/app/AppLogger.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ArchiveScanner.h"
#include "PasswordManager/app/PasswordMatcher.h"
#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/ArchivePasswordRecord.h"
#include "PasswordManager/ui/UiStyle.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QComboBox>
#include <QFont>
#include <QHash>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>

namespace PasswordManager {

namespace {

class NumericTableWidgetItem final : public QTableWidgetItem {
public:
    explicit NumericTableWidgetItem(int value)
        : QTableWidgetItem(QString::number(value))
        , m_value(value)
    {
    }

    bool operator<(const QTableWidgetItem& other) const override
    {
        if (const auto* numericOther = dynamic_cast<const NumericTableWidgetItem*>(&other)) {
            return m_value < numericOther->m_value;
        }
        return QTableWidgetItem::operator<(other);
    }

private:
    int m_value = 0;
};

QTableWidgetItem* numericItem(int value)
{
    auto* item = new NumericTableWidgetItem(value);
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QFont font("Segoe UI");
    font.setPointSize(10);
    item->setFont(font);
    return item;
}

QString formatElapsed(qint64 elapsedMs)
{
    if (elapsedMs < 1000) {
        return QString("%1 ms").arg(elapsedMs);
    }
    return QString::number(elapsedMs / 1000.0, 'f', 2) + " s";
}

void fillPageSizeOptions(QComboBox* combo)
{
    combo->addItem("10", 10);
    combo->addItem("20", 20);
    combo->addItem("50", 50);
    combo->addItem("100", 100);
    combo->addItem("200", 200);
    combo->addItem("500", 500);
    combo->addItem("全部", 0);
    combo->setCurrentIndex(1);
}

} // namespace

QList<ArchivePasswordRecord> directoryHistoryForArchive(const QList<ArchivePasswordRecord>& allHistory, const ArchiveRecord& archive)
{
    QList<ArchivePasswordRecord> records;
    const QString archiveDirectory = QFileInfo(archive.path).absolutePath();
    for (const ArchivePasswordRecord& record : allHistory) {
        if (record.archiveId == archive.id) {
            continue;
        }
        if (QFileInfo(record.archivePath).absolutePath().compare(archiveDirectory, Qt::CaseInsensitive) == 0) {
            records.append(record);
        }
    }
    return records;
}

HomePage::HomePage(
    const AppPaths& paths,
    const ArchiveRepository& archiveRepository,
    const ArchivePasswordRepository& archivePasswordRepository,
    const PasswordRepository& passwordRepository,
    PasswordTestTaskManager& taskManager,
    QWidget* parent)
    : QWidget(parent)
    , m_paths(paths)
    , m_archiveRepository(archiveRepository)
    , m_archivePasswordRepository(archivePasswordRepository)
    , m_passwordRepository(passwordRepository)
    , m_taskManager(taskManager)
{
    buildUi();
    reload();
}

void HomePage::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* title = new QLabel("首页", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* actions = new QHBoxLayout;
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("搜索压缩包路径、文件名或格式");

    auto* scanFilesButton = new QPushButton("扫描文件", this);
    auto* scanDirectoryButton = new QPushButton("扫描文件夹", this);
    auto* categoryButton = new QPushButton("设置分类", this);
    auto* refreshButton = new QPushButton("刷新", this);

    actions->addWidget(m_search, 1);
    actions->addWidget(scanFilesButton);
    actions->addWidget(scanDirectoryButton);
    actions->addWidget(categoryButton);
    actions->addWidget(refreshButton);

    auto* testActions = new QHBoxLayout;
    m_passwordInput = new QLineEdit(this);
    m_passwordInput->setPlaceholderText("输入密码测试选中压缩包；留空可测试无密码压缩包");
    auto* testButton = new QPushButton("测试密码", this);
    auto* matchButton = new QPushButton("智能匹配测试", this);
    testActions->addWidget(m_passwordInput, 1);
    testActions->addWidget(testButton);
    testActions->addWidget(matchButton);

    m_summary = new QLabel(this);
    m_emptyState = new QLabel("还没有压缩包记录。可以点击“扫描文件”或“扫描文件夹”开始。", this);
    m_emptyState->setAlignment(Qt::AlignCenter);
    m_emptyState->setStyleSheet("color: #667085; padding: 16px;");

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"ID", "文件名", "格式", "分类", "大小", "修改时间", "文件指纹", "路径"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->verticalHeader()->setVisible(false);
    UiStyle::applyTableStyle(m_table);
    m_table->setColumnWidth(0, 64);
    m_table->setColumnWidth(1, 220);
    m_table->setColumnWidth(2, 72);
    m_table->setColumnWidth(3, 120);
    m_table->setColumnWidth(4, 96);
    m_table->setColumnWidth(5, 160);
    m_table->setColumnWidth(6, 160);
    m_table->setColumnWidth(7, 420);

    layout->addWidget(title);
    layout->addLayout(actions);
    layout->addLayout(testActions);
    layout->addWidget(m_summary);
    layout->addWidget(m_table, 1);
    layout->addWidget(m_emptyState);

    auto* pagination = new QHBoxLayout;
    m_pageSize = new QComboBox(this);
    fillPageSizeOptions(m_pageSize);
    m_previousPage = new QPushButton("上一页", this);
    m_nextPage = new QPushButton("下一页", this);
    m_pageNumber = new QSpinBox(this);
    m_pageNumber->setMinimum(1);
    m_pageNumber->setMaximum(1);
    m_pageSummary = new QLabel(this);
    pagination->addWidget(new QLabel("每页", this));
    pagination->addWidget(m_pageSize);
    pagination->addWidget(m_previousPage);
    pagination->addWidget(new QLabel("页码", this));
    pagination->addWidget(m_pageNumber);
    pagination->addWidget(m_nextPage);
    pagination->addWidget(m_pageSummary, 1);
    layout->addLayout(pagination);

    connect(m_search, &QLineEdit::textChanged, this, [this]() {
        m_currentPage = 1;
        reload();
    });
    connect(scanFilesButton, &QPushButton::clicked, this, &HomePage::scanFiles);
    connect(scanDirectoryButton, &QPushButton::clicked, this, &HomePage::scanDirectory);
    connect(categoryButton, &QPushButton::clicked, this, &HomePage::setSelectedArchiveCategory);
    connect(refreshButton, &QPushButton::clicked, this, &HomePage::reload);
    connect(testButton, &QPushButton::clicked, this, &HomePage::testSelectedArchivePassword);
    connect(matchButton, &QPushButton::clicked, this, &HomePage::matchSelectedArchivePasswords);
    connect(m_pageSize, &QComboBox::currentIndexChanged, this, [this]() {
        m_currentPage = 1;
        renderPage();
    });
    connect(m_previousPage, &QPushButton::clicked, this, [this]() {
        if (m_currentPage > 1) {
            --m_currentPage;
            renderPage();
        }
    });
    connect(m_nextPage, &QPushButton::clicked, this, [this]() {
        if (m_currentPage < totalPages()) {
            ++m_currentPage;
            renderPage();
        }
    });
    connect(m_pageNumber, &QSpinBox::valueChanged, this, [this](int page) {
        if (page != m_currentPage) {
            m_currentPage = page;
            renderPage();
        }
    });
}

void HomePage::reload()
{
    m_records = m_archiveRepository.list(m_search->text());
    if (m_currentPage > totalPages()) {
        m_currentPage = totalPages();
    }
    renderPage();
}

void HomePage::renderPage()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    const int size = pageSize();
    const int start = size == 0 ? 0 : (m_currentPage - 1) * size;
    const int end = size == 0 ? m_records.size() : qMin(start + size, m_records.size());
    for (int index = start; index < end; ++index) {
        const int row = m_table->rowCount();
        const ArchiveRecord& record = m_records.at(index);
        m_table->insertRow(row);
        m_table->setItem(row, 0, numericItem(record.id));
        m_table->setItem(row, 1, new QTableWidgetItem(record.fileName));
        m_table->setItem(row, 2, new QTableWidgetItem(record.extension.toUpper()));
        m_table->setItem(row, 3, new QTableWidgetItem(record.category));
        m_table->setItem(row, 4, new QTableWidgetItem(formatSize(record.sizeBytes)));
        m_table->setItem(row, 5, new QTableWidgetItem(record.modifiedAt.toString("yyyy-MM-dd HH:mm:ss")));
        m_table->setItem(row, 6, new QTableWidgetItem(record.fullHash.left(16)));
        m_table->setItem(row, 7, new QTableWidgetItem(record.path));
    }
    m_table->setSortingEnabled(true);

    m_emptyState->setVisible(m_records.isEmpty());
    m_summary->setText(m_records.isEmpty()
            ? "当前没有压缩包记录"
            : QString("已记录 %1 个压缩包").arg(m_records.size()));
    updatePaginationControls();
}

void HomePage::updatePaginationControls()
{
    const int pages = totalPages();
    {
        const QSignalBlocker blocker(m_pageNumber);
        m_pageNumber->setMaximum(pages);
        m_pageNumber->setValue(m_currentPage);
    }
    m_previousPage->setEnabled(m_currentPage > 1);
    m_nextPage->setEnabled(m_currentPage < pages);
    const int size = pageSize();
    const int start = m_records.isEmpty() ? 0 : (size == 0 ? 1 : (m_currentPage - 1) * size + 1);
    const int end = m_records.isEmpty() ? 0 : (size == 0 ? m_records.size() : qMin(m_currentPage * size, m_records.size()));
    m_pageSummary->setText(QString("第 %1/%2 页，显示 %3-%4，共 %5 条")
            .arg(m_currentPage)
            .arg(pages)
            .arg(start)
            .arg(end)
            .arg(m_records.size()));
}

int HomePage::pageSize() const
{
    return m_pageSize ? m_pageSize->currentData().toInt() : 20;
}

int HomePage::totalPages() const
{
    const int size = pageSize();
    if (size == 0 || m_records.isEmpty()) {
        return 1;
    }
    return (m_records.size() + size - 1) / size;
}

void HomePage::scanFiles()
{
    const QStringList paths = QFileDialog::getOpenFileNames(
        this,
        "选择压缩包",
        QString(),
        "压缩包 (*.zip *.rar *.7z);;所有文件 (*.*)");
    if (paths.isEmpty()) {
        return;
    }

    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    AppLogger(m_paths.logsDir()).archive(QString("Home scan files started: count=%1 mode=%2")
            .arg(paths.size())
            .arg(settings.calculateFullHashDuringScan ? "full_hash" : "quick_hash"));
    persistScanResult(ArchiveScanner(settings.calculateFullHashDuringScan).scanFiles(paths));
}

void HomePage::scanDirectory()
{
    const QString path = QFileDialog::getExistingDirectory(this, "选择要递归扫描的文件夹");
    if (path.isEmpty()) {
        return;
    }

    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    AppLogger(m_paths.logsDir()).archive(QString("Home scan directory started: path=%1 mode=%2")
            .arg(path)
            .arg(settings.calculateFullHashDuringScan ? "full_hash" : "quick_hash"));
    persistScanResult(ArchiveScanner(settings.calculateFullHashDuringScan).scanDirectory(path));
}

void HomePage::setSelectedArchiveCategory()
{
    const QList<ArchiveRecord> archives = selectedRecords();
    if (archives.isEmpty()) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个或多个压缩包。");
        return;
    }

    bool accepted = false;
    const QString initialCategory = archives.size() == 1 ? archives.first().category : QString();
    const QString category = QInputDialog::getText(
        this,
        "PasswordManager",
        QString("设置 %1 个压缩包的分类：").arg(archives.size()),
        QLineEdit::Normal,
        initialCategory,
        &accepted).trimmed();
    if (!accepted) {
        return;
    }

    QString error;
    int updated = 0;
    for (const ArchiveRecord& archive : archives) {
        if (!m_archiveRepository.updateCategory(archive.id, category, &error)) {
            QMessageBox::warning(this, "PasswordManager", "保存压缩包分类失败：" + error);
            return;
        }
        ++updated;
    }

    reload();
    QMessageBox::information(this, "PasswordManager", QString("已更新 %1 个压缩包分类。").arg(updated));
}

void HomePage::testSelectedArchivePassword()
{
    const ArchiveRecord record = selectedRecord();
    if (record.id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个压缩包。");
        return;
    }

    const QString password = m_passwordInput->text();
    if (password.isEmpty()
        && QMessageBox::question(this, "PasswordManager", "当前密码为空。是否按“无密码压缩包”进行测试？") != QMessageBox::Yes) {
        return;
    }

    const int id = m_taskManager.enqueuePasswordTest(record.id, 0, record.path, password);
    QMessageBox::information(this, "PasswordManager", QString("已加入测试任务：%1").arg(id));
}

void HomePage::matchSelectedArchivePasswords()
{
    const QList<ArchiveRecord> archives = selectedRecords();
    if (archives.isEmpty()) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个或多个压缩包。");
        return;
    }

    const QList<PasswordRecord> passwordRecords = m_passwordRepository.list();
    const QList<ArchivePasswordRecord> allHistory = m_archivePasswordRepository.list();
    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    PasswordMatcher matcher;
    QHash<QString, int> passwordIdsByText;
    for (const PasswordRecord& passwordRecord : passwordRecords) {
        const QString password = passwordRecord.password.trimmed();
        if (!password.isEmpty() && !passwordIdsByText.contains(password)) {
            passwordIdsByText.insert(password, passwordRecord.id);
        }
    }

    QHash<int, QStringList> candidatesByArchiveId;
    int noPasswordArchives = 0;
    int archivesNeedingPassword = 0;
    int totalCandidates = 0;
    for (const ArchiveRecord& archive : archives) {
        if (m_taskManager.isNoPasswordArchive(archive.path)) {
            ++noPasswordArchives;
            continue;
        }

        ++archivesNeedingPassword;
        const QStringList descriptionPasswords = settings.enableDescriptionFiles
            ? matcher.extractLocalDescriptionPasswords(archive.path, settings.maxDescriptionCandidates, settings.maxDescriptionFileBytes)
            : QStringList();
        const QStringList candidates = matcher.buildLayeredCandidates(
            settings.enableExactHistory ? m_archivePasswordRepository.listForArchive(archive.id) : QList<ArchivePasswordRecord>(),
            settings.enableExactHistory ? m_archivePasswordRepository.listForFullHash(archive.fullHash, archive.id) : QList<ArchivePasswordRecord>(),
            settings.enableDirectoryHistory ? directoryHistoryForArchive(allHistory, archive) : QList<ArchivePasswordRecord>(),
            settings.enableCategoryCandidates ? m_passwordRepository.listByCategory(archive.category) : QList<PasswordRecord>(),
            settings.enablePasswordLibrary ? passwordRecords : QList<PasswordRecord>(),
            descriptionPasswords,
            settings.maxCandidates);
        if (!candidates.isEmpty()) {
            candidatesByArchiveId.insert(archive.id, candidates);
            totalCandidates += candidates.size();
        }
    }

    if (archivesNeedingPassword <= 0) {
        QMessageBox::information(this, "PasswordManager", QString("选中的 %1 个压缩包都无需密码，未加入密码库测试任务。").arg(archives.size()));
        return;
    }
    if (totalCandidates <= 0) {
        QMessageBox::information(this, "PasswordManager", "密码库和同目录说明文件里都没有可用候选密码。无密码压缩包请留空后点击“测试密码”。");
        return;
    }

    if (QMessageBox::question(
            this,
            "PasswordManager",
            QString("将对 %1 个需要密码的压缩包加入 %2 个候选密码测试任务。\n\n候选来源按设置页开关控制。\n已跳过 %3 个无需密码的压缩包，不会把密码库密码记为正确。\n\n确认开始批量智能匹配测试吗？")
                .arg(archivesNeedingPassword)
                .arg(totalCandidates)
                .arg(noPasswordArchives))
        != QMessageBox::Yes) {
        return;
    }

    int enqueued = 0;
    for (const ArchiveRecord& archive : archives) {
        if (m_taskManager.isNoPasswordArchive(archive.path)) {
            continue;
        }
        const QStringList candidates = candidatesByArchiveId.value(archive.id);
        for (const QString& password : candidates) {
            const int passwordId = passwordIdsByText.value(password, 0);
            m_taskManager.enqueuePasswordTest(archive.id, passwordId, archive.path, password);
            ++enqueued;
        }
    }

    QMessageBox::information(this, "PasswordManager", QString("已加入 %1 个候选密码测试任务，跳过 %2 个无需密码的压缩包。").arg(enqueued).arg(noPasswordArchives));
}

void HomePage::persistScanResult(const ScanResult& result)
{
    QString error;
    const int saved = m_archiveRepository.upsertMany(result.archives, &error);
    if (!error.isEmpty()) {
        AppLogger(m_paths.logsDir()).error("Archive scan save failed: " + error);
        QMessageBox::critical(this, "PasswordManager", "保存扫描结果失败：" + error);
        return;
    }

    AppLogger(m_paths.logsDir()).archive(QString("Home scan completed: saved=%1 skipped=%2 elapsed_ms=%3 mode=%4")
            .arg(saved)
            .arg(result.skippedCount)
            .arg(result.elapsedMs)
            .arg(result.fullHashCalculated ? "full_hash" : "quick_hash"));
    reload();
    QMessageBox::information(
        this,
        "PasswordManager",
        QString("扫描完成。保存 %1 个压缩包，跳过 %2 个文件。\n耗时：%3\n模式：%4")
            .arg(saved)
            .arg(result.skippedCount)
            .arg(formatElapsed(result.elapsedMs))
            .arg(result.fullHashCalculated ? "精确模式，扫描时计算完整文件指纹" : "快速模式，只计算快速 Hash"));
}

ArchiveRecord HomePage::selectedRecord() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return ArchiveRecord();
    }

    const QTableWidgetItem* idItem = m_table->item(row, 0);
    const int id = idItem ? idItem->text().toInt() : 0;
    for (const ArchiveRecord& record : m_records) {
        if (record.id == id) {
            return record;
        }
    }
    return ArchiveRecord();
}

QList<ArchiveRecord> HomePage::selectedRecords() const
{
    QSet<int> selectedIds;
    const QList<QTableWidgetItem*> items = m_table->selectedItems();
    for (const QTableWidgetItem* item : items) {
        const QTableWidgetItem* idItem = m_table->item(item->row(), 0);
        if (idItem) {
            selectedIds.insert(idItem->text().toInt());
        }
    }

    if (selectedIds.isEmpty()) {
        const ArchiveRecord current = selectedRecord();
        if (current.id > 0) {
            return {current};
        }
        return {};
    }

    QList<ArchiveRecord> records;
    for (const ArchiveRecord& record : m_records) {
        if (selectedIds.contains(record.id)) {
            records.append(record);
        }
    }
    return records;
}

QString HomePage::formatSize(qint64 bytes) const
{
    const double kb = 1024.0;
    const double mb = kb * 1024.0;
    const double gb = mb * 1024.0;
    if (bytes >= gb) {
        return QString::number(bytes / gb, 'f', 2) + " GB";
    }
    if (bytes >= mb) {
        return QString::number(bytes / mb, 'f', 2) + " MB";
    }
    if (bytes >= kb) {
        return QString::number(bytes / kb, 'f', 2) + " KB";
    }
    return QString::number(bytes) + " B";
}

} // namespace PasswordManager
