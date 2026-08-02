#include "PasswordManager/ui/UiStyle.h"

#include <QApplication>
#include <QFont>
#include <QHeaderView>
#include <QTableWidget>

namespace PasswordManager::UiStyle {

void applyApplicationFont(QApplication& app)
{
    QFont font("Microsoft YaHei UI");
    font.setPointSize(10);
    app.setFont(font);
}

void applyTableStyle(QTableWidget* table)
{
    if (!table) {
        return;
    }

    QFont tableFont("Microsoft YaHei UI");
    tableFont.setPointSize(10);
    table->setFont(tableFont);
    table->setAlternatingRowColors(true);
    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);
    table->verticalHeader()->setDefaultSectionSize(34);
    table->horizontalHeader()->setStretchLastSection(false);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->horizontalHeader()->setMinimumSectionSize(56);
    table->horizontalHeader()->setHighlightSections(false);
    table->setStyleSheet(R"(
        QTableWidget {
            background: #ffffff;
            alternate-background-color: #f8fafc;
            border: 1px solid #d9e0e8;
            border-radius: 6px;
            gridline-color: #e4e9f0;
            color: #111827;
            selection-background-color: #dbeafe;
            selection-color: #0f172a;
        }
        QTableWidget::item {
            padding: 5px 7px;
        }
        QTableWidget::item:selected {
            background: #dbeafe;
            color: #0f172a;
        }
        QHeaderView::section {
            background: #f1f5f9;
            color: #334155;
            border: none;
            border-right: 1px solid #d9e0e8;
            border-bottom: 1px solid #d9e0e8;
            padding: 7px 8px;
            font-weight: 600;
        }
    )");
}

} // namespace PasswordManager::UiStyle
