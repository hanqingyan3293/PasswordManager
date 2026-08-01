#include "PasswordManager/app/ExtractService.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <utility>

namespace PasswordManager {

ExtractService::ExtractService(QString sevenZipExecutable)
    : m_sevenZipExecutable(std::move(sevenZipExecutable))
{
}

ExtractResult ExtractService::extract(const QString& archivePath, const QString& password, const QString& outputDirectory, int timeoutMs) const
{
    if (!QFileInfo::exists(m_sevenZipExecutable)) {
        ExtractResult result;
        result.status = ExtractStatus::MissingSevenZip;
        result.errorMessage = "内置 7-Zip 不存在。";
        return result;
    }

    if (!QDir().mkpath(outputDirectory)) {
        ExtractResult result;
        result.status = ExtractStatus::ProcessError;
        result.errorMessage = "无法创建输出目录。";
        return result;
    }

    QStringList arguments = {"x", "-y", "-o" + outputDirectory, archivePath};
    if (!password.isEmpty()) {
        arguments.insert(2, "-p" + password);
    }

    QProcess process;
    process.setProgram(m_sevenZipExecutable);
    process.setArguments(arguments);
    process.start();

    if (!process.waitForStarted(3000)) {
        ExtractResult result;
        result.status = ExtractStatus::ProcessError;
        result.errorMessage = "7-Zip 解压进程无法启动。";
        return result;
    }

    if (!process.waitForFinished(timeoutMs)) {
        process.kill();
        process.waitForFinished(3000);

        ExtractResult result;
        result.status = ExtractStatus::Timeout;
        result.errorMessage = "7-Zip 解压超时。";
        result.output = QString::fromLocal8Bit(process.readAllStandardOutput())
            + QString::fromLocal8Bit(process.readAllStandardError());
        return result;
    }

    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput())
        + QString::fromLocal8Bit(process.readAllStandardError());
    ExtractResult result = classifyResult(process.exitCode(), output);
    result.exitCode = process.exitCode();
    result.output = output;
    return result;
}

ExtractResult ExtractService::classifyResult(int exitCode, const QString& output) const
{
    ExtractResult result;
    result.exitCode = exitCode;

    const QString lower = output.toLower();
    if (exitCode == 0 && lower.contains("everything is ok")) {
        result.status = ExtractStatus::Success;
        return result;
    }

    if (lower.contains("wrong password") || lower.contains("can not open encrypted archive")) {
        result.status = ExtractStatus::WrongPassword;
        result.errorMessage = "密码错误。";
        return result;
    }

    if (lower.contains("is not archive") || lower.contains("headers error") || lower.contains("data error")) {
        result.status = ExtractStatus::ArchiveError;
        result.errorMessage = "压缩包损坏或格式不受支持。";
        return result;
    }

    result.status = ExtractStatus::ProcessError;
    result.errorMessage = "7-Zip 解压失败。";
    return result;
}

QString extractStatusText(ExtractStatus status)
{
    switch (status) {
    case ExtractStatus::Success:
        return "成功";
    case ExtractStatus::WrongPassword:
        return "密码错误";
    case ExtractStatus::ArchiveError:
        return "压缩包错误";
    case ExtractStatus::MissingSevenZip:
        return "7-Zip 缺失";
    case ExtractStatus::Timeout:
        return "超时";
    case ExtractStatus::ProcessError:
        return "进程错误";
    }
    return "未知";
}

} // namespace PasswordManager
