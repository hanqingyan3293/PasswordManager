#include "PasswordManager/ui/PasswordDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QIntValidator>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

namespace PasswordManager {

PasswordDialog::PasswordDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("密码记录");
    resize(460, 320);

    m_password = new QLineEdit(this);
    m_category = new QLineEdit(this);
    m_note = new QPlainTextEdit(this);
    m_favorite = new QCheckBox("收藏", this);
    m_successCount = new QLineEdit(this);
    m_failureCount = new QLineEdit(this);

    auto* countValidator = new QIntValidator(0, 1000000, this);
    m_successCount->setValidator(countValidator);
    m_failureCount->setValidator(countValidator);
    m_successCount->setText("0");
    m_failureCount->setText("0");
    m_successCount->setPlaceholderText("0");
    m_failureCount->setPlaceholderText("0");
    m_note->setMaximumHeight(90);

    auto* form = new QFormLayout;
    form->addRow("密码", m_password);
    form->addRow("分类", m_category);
    form->addRow("备注", m_note);
    form->addRow("", m_favorite);
    form->addRow("成功次数", m_successCount);
    form->addRow("失败次数", m_failureCount);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        if (m_password->text().isEmpty()) {
            QMessageBox::warning(this, "PasswordManager", "密码不能为空。");
            return;
        }
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* layout = new QVBoxLayout(this);
    layout->addLayout(form);
    layout->addWidget(buttons);
}

void PasswordDialog::setRecord(const PasswordRecord& record)
{
    m_id = record.id;
    m_password->setText(record.password);
    m_category->setText(record.category);
    m_note->setPlainText(record.note);
    m_favorite->setChecked(record.favorite);
    m_successCount->setText(QString::number(record.successCount));
    m_failureCount->setText(QString::number(record.failureCount));
}

PasswordRecord PasswordDialog::record() const
{
    PasswordRecord record;
    record.id = m_id;
    record.password = m_password->text();
    record.category = m_category->text();
    record.note = m_note->toPlainText();
    record.favorite = m_favorite->isChecked();
    record.successCount = m_successCount->text().toInt();
    record.failureCount = m_failureCount->text().toInt();
    return record;
}

} // namespace PasswordManager
