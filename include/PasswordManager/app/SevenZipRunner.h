#pragma once

#include <QString>

namespace PasswordManager {

enum class SevenZipTestStatus {
    Success,
    WrongPassword,
    NoPasswordRequired,
    ArchiveError,
    MissingSevenZip,
    Timeout,
    ProcessError
};

struct SevenZipTestResult {
    SevenZipTestStatus status = SevenZipTestStatus::ProcessError;
    int exitCode = -1;
    QString output;
    QString errorMessage;
};

class SevenZipRunner {
public:
    explicit SevenZipRunner(QString executablePath);

    SevenZipTestResult testPassword(const QString& archivePath, const QString& password, int timeoutMs = 10000) const;

private:
    SevenZipTestResult classifyResult(int exitCode, const QString& output) const;

    QString m_executablePath;
};

QString sevenZipTestStatusText(SevenZipTestStatus status);

} // namespace PasswordManager
