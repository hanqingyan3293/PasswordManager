#include "PasswordManager/app/DiagnosticService.h"

#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/SevenZipProbe.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSysInfo>
#include <QTextStream>

#include <utility>

namespace PasswordManager {

DiagnosticService::DiagnosticService(const AppPaths& paths, QString databaseConnectionName)
    : m_paths(paths)
    , m_databaseConnectionName(std::move(databaseConnectionName))
{
}

bool DiagnosticService::exportDiagnosticPackage(QString* outputDirectory, QString* errorMessage) const
{
    const QString directoryName = "diagnostic-" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    const QString destination = QDir(m_paths.logsDir()).filePath(directoryName);
    if (!QDir().mkpath(destination)) {
        if (errorMessage) {
            *errorMessage = "无法创建诊断目录。";
        }
        return false;
    }

    QFile file(QDir(destination).filePath("diagnostic.txt"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << diagnosticText();
    file.close();

    if (!copyLogFiles(destination, errorMessage)) {
        return false;
    }

    if (outputDirectory) {
        *outputDirectory = destination;
    }
    return true;
}

int DiagnosticService::tableCount(const QString& tableName) const
{
    QSqlQuery query(QSqlDatabase::database(m_databaseConnectionName));
    if (!query.exec("SELECT COUNT(*) FROM " + tableName) || !query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

QString DiagnosticService::diagnosticText() const
{
    const SevenZipStatus sevenZip = SevenZipProbe().probe(m_paths.sevenZipExecutable());

    QString text;
    QTextStream stream(&text);
    stream << "PasswordManager Diagnostic\n";
    stream << "Generated: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "Application version: " << QCoreApplication::applicationVersion() << "\n";
    stream << "Qt version: " << qVersion() << "\n";
    stream << "OS: " << QSysInfo::prettyProductName() << "\n\n";

    stream << "Paths\n";
    stream << "Application: " << m_paths.applicationDir() << "\n";
    stream << "Data: " << m_paths.dataDir() << "\n";
    stream << "Backup: " << m_paths.backupDir() << "\n";
    stream << "Logs: " << m_paths.logsDir() << "\n";
    stream << "7-Zip: " << m_paths.sevenZipExecutable() << "\n\n";

    stream << "7-Zip Status\n";
    stream << "Exists: " << (sevenZip.exists ? "yes" : "no") << "\n";
    stream << "Runnable: " << (sevenZip.runnable ? "yes" : "no") << "\n";
    stream << "Version: " << sevenZip.version << "\n";
    if (!sevenZip.errorMessage.isEmpty()) {
        stream << "Error: " << sevenZip.errorMessage << "\n";
    }
    stream << "\n";

    stream << "Database Counts\n";
    stream << "passwords: " << tableCount("passwords") << "\n";
    stream << "archives: " << tableCount("archives") << "\n";
    stream << "archive_passwords: " << tableCount("archive_passwords") << "\n";
    stream << "password_test_tasks: " << tableCount("password_test_tasks") << "\n";
    stream << "extract_logs: " << tableCount("extract_logs") << "\n";
    return text;
}

bool DiagnosticService::copyLogFiles(const QString& outputDirectory, QString* errorMessage) const
{
    const QDir logDir(m_paths.logsDir());
    const QStringList logFiles = logDir.entryList({"*.log"}, QDir::Files);
    for (const QString& fileName : logFiles) {
        const QString source = logDir.filePath(fileName);
        const QString destination = QDir(outputDirectory).filePath(fileName);
        QFile::remove(destination);
        if (!QFile::copy(source, destination)) {
            if (errorMessage) {
                *errorMessage = "复制日志文件失败：" + source;
            }
            return false;
        }
    }
    return true;
}

} // namespace PasswordManager
