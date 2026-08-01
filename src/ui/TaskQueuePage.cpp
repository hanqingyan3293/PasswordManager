#include "PasswordManager/ui/TaskQueuePage.h"

#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/app/SevenZipRunner.h"

#include <QApplication>
#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QComboBox>
#include <QFileInfo>
#include <QHeaderView>
#include <QHBoxLayout>
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
    return new NumericTableWidgetItem(value);
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

QString taskStatusDisplayText(PasswordTestTaskStatus status)
{
    switch (status) {
    case PasswordTestTaskStatus::Waiting:
        return "等待中";
    case PasswordTestTaskStatus::Running:
        return "运行中";
    case PasswordTestTaskStatus::Completed:
        return "已完成";
    case PasswordTestTaskStatus::Failed:
        return "失败";
    case PasswordTestTaskStatus::Cancelled:
        return "已取消";
    case PasswordTestTaskStatus::Timeout:
        return "超时";
    }
    return "未知";
}

QString testStatusDisplayText(SevenZipTestStatus status)
{
    switch (status) {
    case SevenZipTestStatus::Success:
        return "成功";
    case SevenZipTestStatus::WrongPassword:
        return "密码错误";
    case SevenZipTestStatus::NoPasswordRequired:
        return "无需密码";
    case SevenZipTestStatus::ArchiveError:
        return "压缩包错误";
    case SevenZipTestStatus::MissingSevenZip:
        return "缺少 7-Zip";
    case SevenZipTestStatus::Timeout:
        return "超时";
    case SevenZipTestStatus::ProcessError:
        return "进程错误";
    }
    return "未知";
}

} // namespace

TaskQueuePage::TaskQueuePage(PasswordTestTaskManager& taskManager, QWidget* parent)
    : QWidget(parent)
    , m_taskManager(taskManager)
{
    buildUi();
    reload();
    connect(&m_taskManager, &PasswordTestTaskManager::tasksChanged, this, &TaskQueuePage::reload);
}

void TaskQueuePage::buildUi()
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);

    auto* actions = new QHBoxLayout;
    m_statusFilter = new QComboBox(this);
    m_statusFilter->addItem("全部状态", "ALL");
    m_statusFilter->addItem("等待中", "WAITING");
    m_statusFilter->addItem("运行中", "RUNNING");
    m_statusFilter->addItem("已完成", "COMPLETED");
    m_statusFilter->addItem("失败", "FAILED");
    m_statusFilter->addItem("已取消", "CANCELLED");
    m_statusFilter->addItem("超时", "TIMEOUT");

    m_resultFilter = new QComboBox(this);
    m_resultFilter->addItem("全部结果", "ALL");
    m_resultFilter->addItem("成功", "SUCCESS");
    m_resultFilter->addItem("密码错误", "WRONG_PASSWORD");
    m_resultFilter->addItem("无需密码", "NO_PASSWORD_REQUIRED");
    m_resultFilter->addItem("缺少 7-Zip", "MISSING_7ZIP");
    m_resultFilter->addItem("压缩包错误", "ARCHIVE_ERROR");
    m_resultFilter->addItem("超时", "TIMEOUT");
    m_resultFilter->addItem("进程错误", "PROCESS_ERROR");

    auto* retryButton = new QPushButton("重试任务", this);
    auto* cancelButton = new QPushButton("取消任务", this);
    auto* deleteButton = new QPushButton("删除记录", this);
    auto* clearFinishedButton = new QPushButton("清理已结束", this);
    auto* copyCellButton = new QPushButton("复制单元格", this);
    auto* copyRowButton = new QPushButton("复制整行", this);
    auto* refreshButton = new QPushButton("刷新", this);
    actions->addWidget(m_statusFilter);
    actions->addWidget(m_resultFilter);
    actions->addStretch();
    actions->addWidget(copyCellButton);
    actions->addWidget(copyRowButton);
    actions->addWidget(retryButton);
    actions->addWidget(cancelButton);
    actions->addWidget(deleteButton);
    actions->addWidget(clearFinishedButton);
    actions->addWidget(refreshButton);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(7);
    m_table->setHorizontalHeaderLabels({"ID", "状态", "测试结果", "文件", "密码", "消息", "路径"});
    m_table->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    m_table->setContextMenuPolicy(Qt::CustomContextMenu);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->setColumnWidth(0, 64);
    m_table->setColumnWidth(1, 88);
    m_table->setColumnWidth(2, 112);
    m_table->setColumnWidth(3, 220);
    m_table->setColumnWidth(4, 160);
    m_table->setColumnWidth(5, 280);
    m_table->setColumnWidth(6, 360);

    layout->addLayout(actions);
    m_emptyState = new QLabel("当前筛选条件下没有任务记录。可以在首页添加测试任务，或调整筛选条件。", this);
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

    connect(m_statusFilter, &QComboBox::currentIndexChanged, this, [this]() {
        m_currentPage = 1;
        reload();
    });
    connect(m_resultFilter, &QComboBox::currentIndexChanged, this, [this]() {
        m_currentPage = 1;
        reload();
    });
    connect(copyCellButton, &QPushButton::clicked, this, &TaskQueuePage::copySelectedCell);
    connect(copyRowButton, &QPushButton::clicked, this, &TaskQueuePage::copySelectedRow);
    connect(retryButton, &QPushButton::clicked, this, &TaskQueuePage::retrySelectedTask);
    connect(cancelButton, &QPushButton::clicked, this, &TaskQueuePage::cancelSelectedTask);
    connect(deleteButton, &QPushButton::clicked, this, &TaskQueuePage::deleteSelectedTaskRecord);
    connect(clearFinishedButton, &QPushButton::clicked, this, &TaskQueuePage::clearFinishedTaskRecords);
    connect(refreshButton, &QPushButton::clicked, this, &TaskQueuePage::reload);
    connect(m_table, &QTableWidget::currentCellChanged, this, [this](int, int, int, int) {
        applyCurrentCellHighlight(m_table);
    });
    connect(m_table, &QTableWidget::customContextMenuRequested, this, [this](const QPoint& position) {
        if (QTableWidgetItem* item = m_table->itemAt(position)) {
            m_table->setCurrentItem(item);
        }

        QMenu menu(this);
        menu.addAction("复制单元格", this, &TaskQueuePage::copySelectedCell);
        menu.addAction("复制整行", this, &TaskQueuePage::copySelectedRow);
        menu.addSeparator();
        menu.addAction("删除记录", this, &TaskQueuePage::deleteSelectedTaskRecord);
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

void TaskQueuePage::reload()
{
    const QList<PasswordTestTask> tasks = m_taskManager.tasks();
    m_visibleTasks.clear();
    for (const PasswordTestTask& task : tasks) {
        if (matchesFilters(task)) {
            m_visibleTasks.append(task);
        }
    }
    if (m_currentPage > totalPages()) {
        m_currentPage = totalPages();
    }
    renderPage();
}

void TaskQueuePage::renderPage()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);
    const int size = pageSize();
    const int start = size == 0 ? 0 : (m_currentPage - 1) * size;
    const int end = size == 0 ? m_visibleTasks.size() : qMin(start + size, m_visibleTasks.size());
    for (int index = start; index < end; ++index) {
        const PasswordTestTask& task = m_visibleTasks.at(index);
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, numericItem(task.id));
        m_table->setItem(row, 1, new QTableWidgetItem(taskStatusDisplayText(task.status)));
        m_table->setItem(row, 2, new QTableWidgetItem(testStatusDisplayText(task.testStatus)));
        m_table->setItem(row, 3, new QTableWidgetItem(QFileInfo(task.archivePath).fileName()));
        m_table->setItem(row, 4, new QTableWidgetItem(task.password));
        m_table->setItem(row, 5, new QTableWidgetItem(task.message));
        m_table->setItem(row, 6, new QTableWidgetItem(task.archivePath));
    }
    m_table->setSortingEnabled(true);
    m_emptyState->setVisible(m_table->rowCount() == 0);
    applyCurrentCellHighlight(m_table);
    updatePaginationControls();
}

void TaskQueuePage::updatePaginationControls()
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
    const int start = m_visibleTasks.isEmpty() ? 0 : (size == 0 ? 1 : (m_currentPage - 1) * size + 1);
    const int end = m_visibleTasks.isEmpty() ? 0 : (size == 0 ? m_visibleTasks.size() : qMin(m_currentPage * size, m_visibleTasks.size()));
    m_pageSummary->setText(QString("第 %1/%2 页，显示 %3-%4，共 %5 条")
            .arg(m_currentPage)
            .arg(pages)
            .arg(start)
            .arg(end)
            .arg(m_visibleTasks.size()));
}

int TaskQueuePage::pageSize() const
{
    return m_pageSize ? m_pageSize->currentData().toInt() : 20;
}

int TaskQueuePage::totalPages() const
{
    const int size = pageSize();
    if (size == 0 || m_visibleTasks.isEmpty()) {
        return 1;
    }
    return (m_visibleTasks.size() + size - 1) / size;
}

bool TaskQueuePage::matchesFilters(const PasswordTestTask& task) const
{
    const QString statusFilter = m_statusFilter ? m_statusFilter->currentData().toString() : QString("ALL");
    if (statusFilter != "ALL" && statusFilter != passwordTestTaskStatusText(task.status)) {
        return false;
    }

    const QString resultFilter = m_resultFilter ? m_resultFilter->currentData().toString() : QString("ALL");
    if (resultFilter != "ALL" && resultFilter != sevenZipTestStatusText(task.testStatus)) {
        return false;
    }

    return true;
}

void TaskQueuePage::copySelectedCell() const
{
    const QTableWidgetItem* item = m_table->currentItem();
    if (!item) {
        QMessageBox::information(const_cast<TaskQueuePage*>(this), "PasswordManager", "请先选择一个单元格。");
        return;
    }
    QApplication::clipboard()->setText(item->text());
}

void TaskQueuePage::copySelectedRow() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::information(const_cast<TaskQueuePage*>(this), "PasswordManager", "请先选择一行任务。");
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

void TaskQueuePage::cancelSelectedTask()
{
    const int id = selectedTaskId();
    if (id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个任务。");
        return;
    }
    m_taskManager.cancelTask(id);
}

void TaskQueuePage::retrySelectedTask()
{
    const int id = selectedTaskId();
    if (id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个任务。");
        return;
    }

    const int newId = m_taskManager.retryTask(id);
    if (newId <= 0) {
        QMessageBox::information(this, "PasswordManager", "等待中或运行中的任务不能重试。");
    }
}

void TaskQueuePage::deleteSelectedTaskRecord()
{
    const int id = selectedTaskId();
    if (id <= 0) {
        QMessageBox::information(this, "PasswordManager", "请先选择一个任务。");
        return;
    }

    const auto answer = QMessageBox::question(
        this,
        "PasswordManager",
        "只会删除已完成、失败、已取消或超时的任务记录，不会删除密码库和成功历史。确认删除选中记录吗？");
    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!m_taskManager.removeFinishedTask(id)) {
        QMessageBox::information(this, "PasswordManager", "等待中或运行中的任务不能删除，请先取消或等待结束。");
    }
}

void TaskQueuePage::clearFinishedTaskRecords()
{
    const auto answer = QMessageBox::question(
        this,
        "PasswordManager",
        "将清理所有已完成、失败、已取消或超时的任务记录，不会删除密码库和成功历史。确认清理吗？");
    if (answer != QMessageBox::Yes) {
        return;
    }

    const int removed = m_taskManager.clearFinishedTasks();
    if (removed < 0) {
        QMessageBox::warning(this, "PasswordManager", "清理任务记录失败，请稍后重试。");
        return;
    }

    QMessageBox::information(this, "PasswordManager", QString("已清理 %1 条任务记录。").arg(removed));
}

int TaskQueuePage::selectedTaskId() const
{
    const int row = m_table->currentRow();
    if (row < 0) {
        return 0;
    }

    const QTableWidgetItem* item = m_table->item(row, 0);
    return item ? item->text().toInt() : 0;
}

} // namespace PasswordManager
