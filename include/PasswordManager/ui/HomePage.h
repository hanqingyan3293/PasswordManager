#pragma once

#include "PasswordManager/domain/ArchiveRecord.h"

#include <QWidget>

class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace PasswordManager {

class ArchiveRepository;
class ArchivePasswordRepository;
class AppPaths;
class PasswordRepository;
class PasswordTestTaskManager;

class HomePage final : public QWidget {
    Q_OBJECT

public:
    explicit HomePage(
        const AppPaths& paths,
        const ArchiveRepository& archiveRepository,
        const ArchivePasswordRepository& archivePasswordRepository,
        const PasswordRepository& passwordRepository,
        PasswordTestTaskManager& taskManager,
        QWidget* parent = nullptr);
    void reload();

private:
    void buildUi();
    void renderPage();
    void updatePaginationControls();
    int pageSize() const;
    int totalPages() const;
    void scanFiles();
    void scanDirectory();
    void setSelectedArchiveCategory();
    void testSelectedArchivePassword();
    void matchSelectedArchivePasswords();
    void persistScanResult(const struct ScanResult& result);
    ArchiveRecord selectedRecord() const;
    QList<ArchiveRecord> selectedRecords() const;
    QString formatSize(qint64 bytes) const;

    const AppPaths& m_paths;
    const ArchiveRepository& m_archiveRepository;
    const ArchivePasswordRepository& m_archivePasswordRepository;
    const PasswordRepository& m_passwordRepository;
    PasswordTestTaskManager& m_taskManager;
    QLineEdit* m_search = nullptr;
    QLineEdit* m_passwordInput = nullptr;
    QLabel* m_summary = nullptr;
    QLabel* m_emptyState = nullptr;
    QTableWidget* m_table = nullptr;
    QComboBox* m_pageSize = nullptr;
    QSpinBox* m_pageNumber = nullptr;
    QLabel* m_pageSummary = nullptr;
    QPushButton* m_previousPage = nullptr;
    QPushButton* m_nextPage = nullptr;
    int m_currentPage = 1;
    QList<ArchiveRecord> m_records;
};

} // namespace PasswordManager
