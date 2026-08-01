#pragma once

#include "PasswordManager/domain/ArchivePasswordRecord.h"

#include <QWidget>

class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace PasswordManager {

class ArchivePasswordRepository;
class AppPaths;
class ExtractLogRepository;

class HistoryPage final : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPage(
        const AppPaths& paths,
        const ArchivePasswordRepository& repository,
        const ExtractLogRepository& extractLogRepository,
        QWidget* parent = nullptr);

    void focusArchivePath(const QString& archivePath);
    void reload();

signals:
    void passwordRecordRequested(int passwordId, const QString& password);

private:
    void buildUi();
    void renderPage();
    void updatePaginationControls();
    int pageSize() const;
    int totalPages() const;
    void extractSelected();
    void copySelectedPassword() const;
    void copySelectedRow() const;
    void requestSelectedPasswordRecord();
    void deleteSelectedHistory();
    ArchivePasswordRecord selectedRecord() const;

    const AppPaths& m_paths;
    const ArchivePasswordRepository& m_repository;
    const ExtractLogRepository& m_extractLogRepository;
    QLineEdit* m_search = nullptr;
    QLabel* m_emptyState = nullptr;
    QTableWidget* m_table = nullptr;
    QComboBox* m_pageSize = nullptr;
    QSpinBox* m_pageNumber = nullptr;
    QLabel* m_pageSummary = nullptr;
    QPushButton* m_previousPage = nullptr;
    QPushButton* m_nextPage = nullptr;
    int m_currentPage = 1;
    QList<ArchivePasswordRecord> m_records;
};

} // namespace PasswordManager
