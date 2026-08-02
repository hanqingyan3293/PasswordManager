#pragma once

class QApplication;
class QTableWidget;

namespace PasswordManager::UiStyle {

void applyApplicationFont(QApplication& app);
void applyTableStyle(QTableWidget* table);

} // namespace PasswordManager::UiStyle
