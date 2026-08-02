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

void applyApplicationStyle(QApplication& app)
{
    app.setStyleSheet(R"(
        QWidget {
            color: #172033;
            background: #f4f6f8;
            font-family: "Microsoft YaHei UI";
            font-size: 10pt;
        }
        QLabel {
            background: transparent;
            color: #172033;
        }
        QLabel#value {
            color: #334155;
        }
        QFrame#card {
            background: #ffffff;
            border: 1px solid #dce3ea;
            border-radius: 8px;
        }
        QLineEdit,
        QComboBox,
        QSpinBox {
            min-height: 30px;
            padding: 4px 8px;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            background: #ffffff;
            color: #111827;
            selection-background-color: #bfdbfe;
            selection-color: #0f172a;
        }
        QLineEdit:focus,
        QComboBox:focus,
        QSpinBox:focus {
            border: 1px solid #2563eb;
        }
        QPushButton {
            min-height: 30px;
            padding: 5px 12px;
            border: 1px solid #b8c3d1;
            border-radius: 6px;
            background: #ffffff;
            color: #172033;
            font-weight: 500;
        }
        QPushButton:hover {
            background: #eef4ff;
            border-color: #8fb4f8;
        }
        QPushButton:pressed {
            background: #dbeafe;
        }
        QPushButton:disabled {
            color: #94a3b8;
            background: #f1f5f9;
            border-color: #d8e0ea;
        }
        QCheckBox {
            min-height: 26px;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
        }
        QScrollArea {
            background: #f4f6f8;
            border: none;
        }
        QStatusBar {
            background: #ffffff;
            border-top: 1px solid #dce3ea;
            color: #475569;
        }
        QMessageBox QLabel {
            min-width: 260px;
        }
        QMenu {
            background: #ffffff;
            border: 1px solid #dce3ea;
            padding: 5px;
        }
        QMenu::item {
            padding: 7px 24px 7px 12px;
            border-radius: 5px;
        }
        QMenu::item:selected {
            background: #eaf2ff;
            color: #174ea6;
        }
    )");
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
