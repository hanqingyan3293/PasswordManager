#include "PasswordManager/ui/PasswordsPage.h"

#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/data/PasswordLibraryTransferService.h"
#include "PasswordManager/ui/PasswordDialog.h"
#include "PasswordManager/ui/UiStyle.h"

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QComboBox>
#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QLabel>
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

void applyCurrentCellHighlight(QTableWidget* table)
{
    if (!table) {
        return;
    }

    for (int row = 0; row < table->rowCount(); ++row) {
        for (int column = 0; column < table->columnCount(); ++column) {
            if (QTableWidgetItem* item = table->item(row, column)) {
                item->setBackground(QBrush());
            }
        }
    }

    const int currentRow = table->currentRow();
    if (currentRow < 0) {
        return;
    }

    const QColor rowColor("#eaf3ff");
    const QColor cellColor("#b9d7ff");
    for (int column = 0; column < table->columnCount(); ++column) {
        if (QTableWidgetItem* item = table->item(currentRow, column)) {
            item->setBackground(rowColor);
        }
    }
    if (QTableWidgetItem* item = table->currentItem()) {
        item->setBackground(cellColor);
    }
}

} // namespace

PasswordsPage::PasswordsPage(const PasswordRepository& repository, QWidget* parent)
    : QWidget(parent)
    , m_repository(repository)
{
    buildUi();
    reload();
}

void PasswordsPage::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* actions = new QHBoxLayout;
    m_search = new QLineEdit(this);
    m_search->setPlaceholderText("搜索密码、分类或备注");

    auto* addButton = new QPushButton("新增", this);
    auto* editButton = new QPushButton("编辑", this);
    auto* deleteButton = new QPushButton("删除", this);
    auto* importButton = new QPushButton("导入 CSV", this);
    auto* exportButton = new QPushButton("导出 CSV", this);
    auto* copyCellButton = new QPushButton("复制单元格", this);
    auto* copyRowButton = new QPushButton("复制整行", this);
    auto* refreshButton = new QPushButton("刷新", this);

    actions->addWidget(m_search, 1);
    actions->addWidget(copyCellButton);
    actions->addWidget(copyRowButton);
    actions->addWidget(addButton);
    actions->addWidget(editButton);
    actions->addWidget(deleteButton);
    actions->addWidget(importButton);
    actions->addWidget(exportButton);
    actions->addWidget(refreshButton);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(8);
    m_table->setHorizontalHeaderLabels({"ID", "密码", "分类", "收藏", "成功", "失败", "备注", "更新时间"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    UiStyle::applyTableStyle(m_table);
    m_table->setColumnWidth(0, 64);
    m_table->setColumnWidth(1, 180);
    m_table->setColumnWidth(2, 140);
    m_table->setColumnWidth(3, 72);
    m_table->setColumnWidth(4, 72);
    m_table->setColumnWidth(5, 72);
    m_table->setColumnWidth(6, 320);
    m_table->setColumnWidth(7, 160);

    layout->addLayout(actions);
    m_emptyState = new QLabel("密码库为空。可以点击“新增”手动添加，或点击“导入 CSV”批量导入。", this);
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
    connect(copyCellButton, &QPushButton::clicked, this, &PasswordsPage::copySelectedCell);
    connect(copyRowButton, &QPushButton::clicked, this, &PasswordsPage::copySelectedRow);
    connect(addButton, &QPushButton::clicked, this, &PasswordsPage::addPassword);
    connect(editButton, &QPushButton::clicked, this, &PasswordsPage::editSelectedPassword);
    connect(deleteButton, &QPushButton::clicked, this, &PasswordsPage::deleteSelectedPassword);
    connect(importButton, &QPushButton::clicked, this, &PasswordsPage::importPasswordsCsv);
    connect(exportButton, &QPushButton::clicked, this, &PasswordsPage::exportPasswordsCsv);
    connect(refreshButton, &QPushButton::clicked, this, &PasswordsPage::reload);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int, int, int, int) {
        applyCurrentCellHighlight(m_table);
    });
    connect(m_table, &QTableWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        if (QTableWidgetItem* item = m_table->itemAt(position)) {
            m_table->setCurrentItem(item);
        }

        QMenu menu(this);
        menu.addAction("复制单元格", this, &PasswordsPage::copySelectedCell);
        menu.addAction("复制整行", this, &PasswordsPage::copySelectedRow);
        menu.exec(m_table->viewport()->mapToGlobal(position));
    });
    connect(m_table, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        editSelectedPassword();
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

void PasswordsPage::reload()
{
    m_records = m_repository.list(m_search->text());
    if (m_currentPage > totalPages()) {
        m_currentPage = totalPages();
    }
    renderPage();
}

void PasswordsPage::renderPage()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);

    const int size = pageSize();
    const int start = size == 0 ? 0 : (m_currentPage - 1) * size;
    const int end = size == 0 ? m_records.size() : qMin(start + size, m_records.size());
    for (int index = start; index < end; ++index) {
        const int row = m_table->rowCount();
        const PasswordRecord& record = m_records.at(index);
        m_table->insertRow(row);
        m_table->setItem(row, 0, numericItem(record.id));
        m_table->setItem(row, 1, new QTableWidgetItem(record.password));
        m_table->setItem(row, 2, new QTableWidgetItem(record.category));
        m_table->setItem(row, 3, new QTableWidgetItem(record.favorite ? "是" : "否"));
        m_table->setItem(row, 4, numericItem(record.successCount));
        m_table->setItem(row, 5, numericItem(record.failureCount));
        m_table->setItem(row, 6, new QTableWidgetItem(record.note));
        m_table->setItem(row, 7, new QTableWidgetItem(record.updatedAt.toString("yyyy-MM-dd HH:mm:ss")));
    }
    m_table->setSortingEnabled(true);
    m_emptyState->setVisible(m_records.isEmpty());
    applyCurrentCellHighlight(m_table);
    updatePaginationControls();
}

void PasswordsPage::updatePaginationControls()
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

int PasswordsPage::pageSize() const
{
    return m_pageSize ? m_pageSize->currentData().toInt() : 20;
}

int PasswordsPage::totalPages() const
{
    const int size = pageSize();
    if (size == 0 || m_records.isEmpty()) {
        return 1;
    }
    return (m_records.size() + size - 1) / size;
}

void PasswordsPage::focusPassword(int passwordId, const QString& password)
{
    m_search->setText(password);
    reload();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem* idItem = m_table->item(row, 0);
        if (idItem && idItem->text().toInt() == passwordId) {
            m_table->setCurrentCell(row, 1);
            applyCurrentCellHighlight(m_table);
            return;
        }
    }

    for (int row = 0; row < m_table->rowCount(); ++row) {
        const QTableWidgetItem* passwordItem = m_table->item(row, 1);
        if (passwordItem && passwordItem->text() == password) {
            m_table->setCurrentCell(row, 1);
            applyCurrentCellHighlight(m_table);
            return;
        }
    }
}

void PasswordsPage::addPassword()
{
    PasswordDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString error;
    if (!m_repository.add(dialog.record(), &error)) {
        showError(error);
        return;
    }
    reload();
}

void PasswordsPage::editSelectedPassword()
{
    if (selectedRecordId() <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一条密码记录。");
        return;
    }

    PasswordDialog dialog(this);
    dialog.setRecord(selectedRecord());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString error;
    if (!m_repository.update(dialog.record(), &error)) {
        showError(error.isEmpty() ? "更新失败。" : error);
        return;
    }
    reload();
}

void PasswordsPage::deleteSelectedPassword()
{
    const int id = selectedRecordId();
    if (id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一条密码记录。");
        return;
    }

    if (QMessageBox::question(this, "PasswordManager", "确定删除选中的密码记录吗？") != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!m_repository.remove(id, &error)) {
        showError(error.isEmpty() ? "删除失败。" : error);
        return;
    }
    reload();
}

void PasswordsPage::importPasswordsCsv()
{
    const QString filePath = QFileDialog::getOpenFileName(
        this,
        "导入密码库 CSV",
        QString(),
        "CSV 文件 (*.csv);;所有文件 (*.*)");
    if (filePath.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, "PasswordManager", "CSV 文件会按明文密码导入本地密码库。确认导入吗？") != QMessageBox::Yes) {
        return;
    }

    PasswordImportResult result;
    QString error;
    if (!PasswordLibraryTransferService(m_repository).importCsv(filePath, &result, &error)) {
        showError(error.isEmpty() ? "导入失败。" : error);
        return;
    }

    reload();
    QMessageBox::information(
        this,
        "PasswordManager",
        QString("导入完成：新增 %1 条，跳过 %2 行。重复 %3 行，无效 %4 行。")
            .arg(result.importedCount)
            .arg(result.skippedCount)
            .arg(result.duplicateCount)
            .arg(result.invalidCount));
}

void PasswordsPage::exportPasswordsCsv()
{
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        "导出密码库 CSV",
        "passwords.csv",
        "CSV 文件 (*.csv);;所有文件 (*.*)");
    if (filePath.isEmpty()) {
        return;
    }

    if (QMessageBox::question(this, "PasswordManager", "导出的 CSV 会包含明文密码，请确认保存位置安全。继续导出吗？") != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!PasswordLibraryTransferService(m_repository).exportCsv(filePath, &error)) {
        showError(error.isEmpty() ? "导出失败。" : error);
        return;
    }

    QMessageBox::information(this, "PasswordManager", "导出完成：\n" + filePath);
}

void PasswordsPage::copySelectedCell() const
{
    const QTableWidgetItem* item = m_table->currentItem();
    if (!item) {
        QMessageBox::information(const_cast<PasswordsPage*>(this), "PasswordManager", "请先选择一个单元格。");
        return;
    }
    QApplication::clipboard()->setText(item->text());
}

void PasswordsPage::copySelectedRow() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(const_cast<PasswordsPage*>(this), "PasswordManager", "请先选择一行密码记录。");
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

PasswordRecord PasswordsPage::selectedRecord() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return PasswordRecord();
    }

    const QTableWidgetItem* idItem = m_table->item(row, 0);
    const int id = idItem ? idItem->text().toInt() : 0;
    for (const PasswordRecord& record : m_records) {
        if (record.id == id) {
            return record;
        }
    }
    return PasswordRecord();
}

int PasswordsPage::selectedRecordId() const
{
    return selectedRecord().id;
}

void PasswordsPage::showError(const QString& message)
{
    QMessageBox::critical(this, "PasswordManager", message.isEmpty() ? "数据库操作失败。" : message);
}

} // namespace PasswordManager
