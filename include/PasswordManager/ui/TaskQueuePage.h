#pragma once

#include <QWidget>

class QLabel;
class QTableWidget;
class QComboBox;
class QPushButton;
class QSpinBox;

namespace PasswordManager {

class PasswordTestTaskManager;
struct PasswordTestTask;

class TaskQueuePage final : public QWidget {
    Q_OBJECT

public:
    explicit TaskQueuePage(PasswordTestTaskManager& taskManager, QWidget* parent = nullptr);
    void reload();

private:
    void buildUi();
    void renderPage();
    void updatePaginationControls();
    int pageSize() const;
    int totalPages() const;
    void cancelSelectedTask();
    void retrySelectedTask();
    void deleteSelectedTaskRecord();
    void clearFinishedTaskRecords();
    void copySelectedCell() const;
    void copySelectedRow() const;
    bool matchesFilters(const PasswordTestTask& task) const;
    int selectedTaskId() const;

    PasswordTestTaskManager& m_taskManager;
    QTableWidget* m_table = nullptr;
    QLabel* m_emptyState = nullptr;
    QComboBox* m_statusFilter = nullptr;
    QComboBox* m_resultFilter = nullptr;
    QComboBox* m_pageSize = nullptr;
    QSpinBox* m_pageNumber = nullptr;
    QLabel* m_pageSummary = nullptr;
    QPushButton* m_previousPage = nullptr;
    QPushButton* m_nextPage = nullptr;
    int m_currentPage = 1;
    QList<PasswordTestTask> m_visibleTasks;
};

} // namespace PasswordManager
