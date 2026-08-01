#pragma once

#include "PasswordManager/domain/PasswordRecord.h"

#include <QDialog>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;

namespace PasswordManager {

class PasswordDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PasswordDialog(QWidget* parent = nullptr);

    void setRecord(const PasswordRecord& record);
    PasswordRecord record() const;

private:
    QLineEdit* m_password = nullptr;
    QLineEdit* m_category = nullptr;
    QPlainTextEdit* m_note = nullptr;
    QCheckBox* m_favorite = nullptr;
    QLineEdit* m_successCount = nullptr;
    QLineEdit* m_failureCount = nullptr;
    int m_id = 0;
};

} // namespace PasswordManager
