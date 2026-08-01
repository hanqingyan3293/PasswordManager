#include "PasswordManager/app/PerformanceBenchmarkService.h"

#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ArchiveScanner.h"
#include "PasswordManager/app/PasswordMatcher.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/data/PasswordTestTaskRepository.h"

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

#include <utility>

namespace PasswordManager {

PerformanceBenchmarkService::PerformanceBenchmarkService(const AppPaths& paths, QString databaseConnectionName)
    : m_paths(paths)
    , m_databaseConnectionName(std::move(databaseConnectionName))
{
}

bool PerformanceBenchmarkService::run(const QString& scanDirectory, QString* outputPath, QString* errorMessage) const
{
    if (!QDir().mkpath(m_paths.logsDir())) {
        if (errorMessage) {
            *errorMessage = "无法创建日志目录。";
        }
        return false;
    }

    const QString path = QDir(m_paths.logsDir()).filePath("benchmark-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".txt");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << report(scanDirectory);

    if (outputPath) {
        *outputPath = path;
    }
    return true;
}

QString PerformanceBenchmarkService::report(const QString& scanDirectory) const
{
    QString text;
    QTextStream stream(&text);
    stream << "PasswordManager Performance Baseline\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "Mode: single-thread baseline\n\n";

    QElapsedTimer timer;

    timer.start();
    const auto passwords = PasswordRepository(m_databaseConnectionName).list();
    const qint64 passwordListMs = timer.elapsed();

    timer.restart();
    const auto archives = ArchiveRepository(m_databaseConnectionName).list();
    const qint64 archiveListMs = timer.elapsed();

    timer.restart();
    const auto history = ArchivePasswordRepository(m_databaseConnectionName).list();
    const qint64 historyListMs = timer.elapsed();

    timer.restart();
    const auto tasks = PasswordTestTaskRepository(m_databaseConnectionName).list();
    const qint64 taskListMs = timer.elapsed();

    timer.restart();
    const QStringList candidates = PasswordMatcher().buildCandidates(passwords, 1000);
    const qint64 matchMs = timer.elapsed();

    stream << "Database Read\n";
    stream << "passwords: " << passwords.size() << " rows, " << passwordListMs << " ms\n";
    stream << "archives: " << archives.size() << " rows, " << archiveListMs << " ms\n";
    stream << "archive_passwords: " << history.size() << " rows, " << historyListMs << " ms\n";
    stream << "password_test_tasks: " << tasks.size() << " rows, " << taskListMs << " ms\n\n";

    stream << "Password Matching\n";
    stream << "input password rows: " << passwords.size() << "\n";
    stream << "candidate limit: 1000\n";
    stream << "generated candidates: " << candidates.size() << "\n";
    stream << "elapsed: " << matchMs << " ms\n\n";

    if (!scanDirectory.trimmed().isEmpty()) {
        timer.restart();
        const ScanResult scanResult = ArchiveScanner().scanDirectory(scanDirectory);
        const qint64 scanMs = timer.elapsed();
        stream << "Archive Scan\n";
        stream << "directory: " << scanDirectory << "\n";
        stream << "archives found: " << scanResult.archives.size() << "\n";
        stream << "files skipped: " << scanResult.skippedCount << "\n";
        stream << "elapsed: " << scanMs << " ms\n\n";
    } else {
        stream << "Archive Scan\n";
        stream << "directory: not provided\n\n";
    }

    stream << "Notes\n";
    stream << "- This is a single-thread baseline.\n";
    stream << "- It does not test GPU acceleration.\n";
    stream << "- It does not run password cracking; 7-Zip test throughput needs separate archive/password workload testing.\n";
    return text;
}

} // namespace PasswordManager
