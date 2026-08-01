#pragma once

#include "PasswordManager/domain/PasswordRecord.h"

#include <QWidget>

class QLabel;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;

namespace PasswordManager {

class PasswordRepository;

class PasswordsPage final : public QWidget {
    Q_OBJECT

public:
    explicit PasswordsPage(const PasswordRepository& repository, QWidget* parent = nullptr);
    void reload();
    void focusPassword(int passwordId, const QString& password);

private:
    void buildUi();
    void renderPage();
    void updatePaginationControls();
    int pageSize() const;
    int totalPages() const;
    void addPassword();
    void editSelectedPassword();
    void deleteSelectedPassword();
    void importPasswordsCsv();
    void exportPasswordsCsv();
    void copySelectedCell() const;
    void copySelectedRow() const;
    PasswordRecord selectedRecord() const;
    int selectedRecordId() const;
    void showError(const QString& message);

    const PasswordRepository& m_repository;
    QLineEdit* m_search = nullptr;
    QLabel* m_emptyState = nullptr;
    QTableWidget* m_table = nullptr;
    QComboBox* m_pageSize = nullptr;
    QSpinBox* m_pageNumber = nullptr;
    QLabel* m_pageSummary = nullptr;
    QPushButton* m_previousPage = nullptr;
    QPushButton* m_nextPage = nullptr;
    int m_currentPage = 1;
    QList<PasswordRecord> m_records;
};

} // namespace PasswordManager
