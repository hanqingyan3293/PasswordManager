#include "PasswordManager/ui/HistoryPage.h"

#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/AppLogger.h"
#include "PasswordManager/app/ExtractService.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ExtractLogRepository.h"
#include "PasswordManager/ui/UiStyle.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
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

HistoryPage::HistoryPage(
    const AppPaths& paths,
    const ArchivePasswordRepository& repository,
    const ExtractLogRepository& extractLogRepository,
    QWidget* parent)
    : QWidget(parent)
    , m_paths(paths)
    , m_repository(repository)
    , m_extractLogRepository(extractLogRepository)
{
    buildUi();
    reload();
}

void HistoryPage::focusArchivePath(const QString& archivePath)
{
    const QString absolutePath = QFileInfo(archivePath).absoluteFilePath();
    m_search->setText(absolutePath);
    reload();
    if (m_table->rowCount() > 0) {
        m_table->selectRow(0);
    }
}

void HistoryPage::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* actions = new QHBoxLayout;
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("搜索压缩包、路径或密码");
    auto* extractButton = new QPushButton("解压", this);
    auto* copyPasswordButton = new QPushButton("复制密码", this);
    auto* copyRowButton = new QPushButton("复制整行", this);
    auto* openPasswordButton = new QPushButton("查看密码库", this);
    auto* deleteButton = new QPushButton("删除历史", this);
    auto* refreshButton = new QPushButton("刷新", this);
    actions->addWidget(m_search, 1);
    actions->addWidget(copyPasswordButton);
    actions->addWidget(copyRowButton);
    actions->addWidget(openPasswordButton);
    actions->addWidget(extractButton);
    actions->addWidget(deleteButton);
    actions->addWidget(refreshButton);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"ID", "压缩包", "密码", "成功次数", "最近成功", "更新时间", "路径"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);
    UiStyle::applyTableStyle(m_table);
    m_table->setColumnWidth(0, 64);
    m_table->setColumnWidth(1, 220);
    m_table->setColumnWidth(2, 160);
    m_table->setColumnWidth(3, 88);
    m_table->setColumnWidth(4, 160);
    m_table->setColumnWidth(5, 160);
    m_table->setColumnWidth(6, 420);

    layout->addLayout(actions);
    m_emptyState = new QLabel("还没有成功密码历史。完成正确密码测试后会出现在这里。", this);
    m_emptyState->setAlignment(Qt::AlignCenter);
    m_emptyState->setStyleSheet("color: #667085; padding: 16px;");
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
    connect(copyPasswordButton, &QPushButton::clicked, this, &HistoryPage::copySelectedPassword);
    connect(copyRowButton, &QPushButton::clicked, this, &HistoryPage::copySelectedRow);
    connect(openPasswordButton, &QPushButton::clicked, this, &HistoryPage::requestSelectedPasswordRecord);
    connect(extractButton, &QPushButton::clicked, this, &HistoryPage::extractSelected);
    connect(deleteButton, &QPushButton::clicked, this, &HistoryPage::deleteSelectedHistory);
    connect(refreshButton, &QPushButton::clicked, this, &HistoryPage::reload);
    connect(m_table, &QTableWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        if (QTableWidgetItem* item = m_table->itemAt(position)) {
            m_table->setCurrentItem(item);
        }

        QMenu menu(this);
        menu.addAction("复制密码", this, &HistoryPage::copySelectedPassword);
        menu.addAction("复制整行", this, &HistoryPage::copySelectedRow);
        menu.addAction("查看密码库", this, &HistoryPage::requestSelectedPasswordRecord);
        menu.addSeparator();
        menu.addAction("删除历史", this, &HistoryPage::deleteSelectedHistory);
        menu.exec(m_table->viewport()->mapToGlobal(position));
    });
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

void HistoryPage::reload()
{
    m_records = m_repository.list(m_search->text());
    if (m_currentPage > totalPages()) {
        m_currentPage = totalPages();
    }
    renderPage();
}

void HistoryPage::renderPage()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    const int size = pageSize();
    const int start = size == 0 ? 0 : (m_currentPage - 1) * size;
    const int end = size == 0 ? m_records.size() : qMin(start + size, m_records.size());
    for (int index = start; index < end; ++index) {
        const int row = m_table->rowCount();
        const ArchivePasswordRecord& record = m_records.at(index);
        m_table->insertRow(row);
        m_table->setItem(row, 0, numericItem(record.id));
        m_table->setItem(row, 1, new QTableWidgetItem(record.archiveName));
        m_table->setItem(row, 2, new QTableWidgetItem(record.password));
        m_table->setItem(row, 3, numericItem(record.successCount));
        m_table->setItem(row, 4, new QTableWidgetItem(record.lastSuccessAt.toString("yyyy-MM-dd HH:mm:ss")));
        m_table->setItem(row, 5, new QTableWidgetItem(record.updatedAt.toString("yyyy-MM-dd HH:mm:ss")));
        m_table->setItem(row, 6, new QTableWidgetItem(record.archivePath));
    }
    m_table->setSortingEnabled(true);
    m_emptyState->setVisible(m_records.isEmpty());
    updatePaginationControls();
}

void HistoryPage::updatePaginationControls()
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

int HistoryPage::pageSize() const
{
    return m_pageSize ? m_pageSize->currentData().toInt() : 20;
}

int HistoryPage::totalPages() const
{
    const int size = pageSize();
    if (size == 0 || m_records.isEmpty()) {
        return 1;
    }
    return (m_records.size() + size - 1) / size;
}

void HistoryPage::extractSelected()
{
    const ArchivePasswordRecord record = selectedRecord();
    if (record.id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一条成功密码记录。");
        return;
    }

    const QString outputDirectory = QFileDialog::getExistingDirectory(this, "选择解压输出目录");
    if (outputDirectory.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, "PasswordManager", "确认使用该密码解压选中的压缩包吗？") != QMessageBox::Yes) {
        return;
    }

    const ExtractResult result = ExtractService(m_paths.sevenZipExecutable())
        .extract(record.archivePath, record.password, outputDirectory);
    const QString message = result.errorMessage.isEmpty() ? result.output.left(800) : result.errorMessage;

    QString logError;
    m_extractLogRepository.add(record.archiveId, record.archivePath, outputDirectory, result.status, message, &logError);
    AppLogger(m_paths.logsDir()).extract(QString("Extract completed from history: archive_id=%1 status=%2 output=%3 path=%4")
            .arg(record.archiveId)
            .arg(extractStatusText(result.status))
            .arg(outputDirectory)
            .arg(record.archivePath));
    if (!logError.isEmpty()) {
        AppLogger(m_paths.logsDir()).error("Extract log write failed: " + logError);
    }

    if (result.status == ExtractStatus::Success) {
        QMessageBox::information(this, "PasswordManager", "解压完成。");
    } else {
        QMessageBox::warning(this, "PasswordManager", "解压失败：" + message);
    }
}

void HistoryPage::copySelectedPassword() const
{
    const ArchivePasswordRecord record = selectedRecord();
    if (record.id <= 0) {
        QMessageBox::information(const_cast<HistoryPage*>(this), "PasswordManager", "请先选择一条成功密码记录。");
        return;
    }
    QApplication::clipboard()->setText(record.password);
}

void HistoryPage::copySelectedRow() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(const_cast<HistoryPage*>(this), "PasswordManager", "请先选择一条成功密码记录。");
        return;
    }

    QStringList values;
    values.reserve(m_table->columnCount());
    for (int column = 0; column < m_table->columnCount(); ++column) {
        const QTableWidgetItem* item = m_table->item(row, column);
        values.append(item ? item->text() : QString());
    }
    QApplication::clipboard()->setText(values.join('\t'));
}

void HistoryPage::requestSelectedPasswordRecord()
{
    const ArchivePasswordRecord record = selectedRecord();
    if (record.id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一条成功密码记录。");
        return;
    }

    emit passwordRecordRequested(record.passwordId, record.password);
}

void HistoryPage::deleteSelectedHistory()
{
    const ArchivePasswordRecord record = selectedRecord();
    if (record.id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一条成功密码记录。");
        return;
    }

    if (QMessageBox::question(this, "PasswordManager", "只删除该压缩包与密码的成功历史，不删除密码库记录。确认删除吗？") != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!m_repository.remove(record.id, &error)) {
        QMessageBox::warning(this, "PasswordManager", error.isEmpty() ? "删除历史失败。" : error);
        return;
    }
    reload();
}

ArchivePasswordRecord HistoryPage::selectedRecord() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return ArchivePasswordRecord();
    }

    const QTableWidgetItem* idItem = m_table->item(row, 0);
    const int id = idItem ? idItem->text().toInt() : 0;
    for (const ArchivePasswordRecord& record : m_records) {
        if (record.id == id) {
            return record;
        }
    }
    return ArchivePasswordRecord();
}

} // namespace PasswordManager
