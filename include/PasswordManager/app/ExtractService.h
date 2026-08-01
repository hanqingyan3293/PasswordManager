#pragma once

#include <QString>

namespace PasswordManager {

enum class ExtractStatus {
    Success,
    WrongPassword,
    ArchiveError,
    MissingSevenZip,
    Timeout,
    ProcessError
};

struct ExtractResult {
    ExtractStatus status = ExtractStatus::ProcessError;
    int exitCode = -1;
    QString output;
    QString errorMessage;
};

class ExtractService {
public:
    explicit ExtractService(QString sevenZipExecutable);

    ExtractResult extract(const QString& archivePath, const QString& password, const QString& outputDirectory, int timeoutMs = 30000) const;

private:
    ExtractResult classifyResult(int exitCode, const QString& output) const;

    QString m_sevenZipExecutable;
};

QString extractStatusText(ExtractStatus status);

} // namespace PasswordManager

