#include "PasswordManager/app/SevenZipRunner.h"

#include <QFileInfo>
#include <QProcess>

#include <utility>

namespace PasswordManager {

SevenZipRunner::SevenZipRunner(QString executablePath)
    : m_executablePath(std::move(executablePath))
{
}

SevenZipTestResult SevenZipRunner::testPassword(const QString& archivePath, const QString& password, int timeoutMs) const
{
    if (!QFileInfo::exists(m_executablePath)) {
        SevenZipTestResult result;
        result.status = SevenZipTestStatus::MissingSevenZip;
        result.errorMessage = "Bundled 7-Zip does not exist.";
        return result;
    }

    if (!password.isEmpty()) {
        const SevenZipTestResult noPasswordResult = testPassword(archivePath, QString(), timeoutMs);
        if (noPasswordResult.status == SevenZipTestStatus::Success) {
            SevenZipTestResult result = noPasswordResult;
            result.status = SevenZipTestStatus::NoPasswordRequired;
            result.errorMessage = "Archive does not require a password.";
            return result;
        }
        if (noPasswordResult.status != SevenZipTestStatus::WrongPassword) {
            return noPasswordResult;
        }
    }

    const QStringList arguments = {"t", "-y", "-p" + password, archivePath};

    QProcess process;
    process.setProgram(m_executablePath);
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted(3000)) {
        SevenZipTestResult result;
        result.status = SevenZipTestStatus::ProcessError;
        result.errorMessage = "7-Zip process could not start.";
        return result;
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(3000);

        SevenZipTestResult result;
        result.status = SevenZipTestStatus::Timeout;
        result.errorMessage = "7-Zip password test timed out.";
        result.output = QString::fromLocal8Bit(process.readAllStandardOutput())
            + QString::fromLocal8Bit(process.readAllStandardError());
        return result;
    }

    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput())
        + QString::fromLocal8Bit(process.readAllStandardError());
    SevenZipTestResult result = classifyResult(process.exitCode(), output);
    result.exitCode = process.exitCode();
    result.output = output;
    return result;
}

SevenZipTestResult SevenZipRunner::classifyResult(int exitCode, const QString& output) const
{
    SevenZipTestResult result;
    result.exitCode = exitCode;

    const QString lower = output.toLower();
    if (exitCode == 0 && lower.contains("everything is ok")) {
        result.status = SevenZipTestStatus::Success;
        return result;
    }

    if (lower.contains("wrong password") || lower.contains("can not open encrypted archive")) {
        result.status = SevenZipTestStatus::WrongPassword;
        result.errorMessage = "Password is wrong.";
        return result;
    }

    if (lower.contains("is not archive") || lower.contains("headers error") || lower.contains("data error")) {
        result.status = SevenZipTestStatus::ArchiveError;
        result.errorMessage = "Archive is damaged or unsupported.";
        return result;
    }

    result.status = SevenZipTestStatus::ProcessError;
    result.errorMessage = "7-Zip test failed.";
    return result;
}

QString sevenZipTestStatusText(SevenZipTestStatus status)
{
    switch (status) {
    case SevenZipTestStatus::Success:
        return "SUCCESS";
    case SevenZipTestStatus::WrongPassword:
        return "WRONG_PASSWORD";
    case SevenZipTestStatus::NoPasswordRequired:
        return "NO_PASSWORD_REQUIRED";
    case SevenZipTestStatus::ArchiveError:
        return "ARCHIVE_ERROR";
    case SevenZipTestStatus::MissingSevenZip:
        return "MISSING_7ZIP";
    case SevenZipTestStatus::Timeout:
        return "TIMEOUT";
    case SevenZipTestStatus::ProcessError:
        return "PROCESS_ERROR";
    }
    return "UNKNOWN";
}

} // namespace PasswordManager
