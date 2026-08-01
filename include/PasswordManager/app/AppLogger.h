#pragma once

#include <QString>

namespace PasswordManager {

class AppLogger {
public:
    explicit AppLogger(QString logDirectory);

    void info(const QString& message) const;
    void error(const QString& message) const;

private:
    void writeLine(const QString& fileName, const QString& level, const QString& message) const;

    QString m_logDirectory;
};

} // namespace PasswordManager

