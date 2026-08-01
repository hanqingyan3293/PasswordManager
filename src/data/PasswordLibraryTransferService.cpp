#include "PasswordManager/data/PasswordLibraryTransferService.h"

#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QFile>
#include <QSet>
#include <QStringConverter>
#include <QTextStream>

namespace PasswordManager {

PasswordLibraryTransferService::PasswordLibraryTransferService(const PasswordRepository& repository)
    : m_repository(repository)
{
}

bool PasswordLibraryTransferService::exportCsv(const QString& filePath, QString* errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "password,category,note,favorite,success_count,failure_count\n";
    for (const PasswordRecord& record : m_repository.list()) {
        stream << toCsvCell(record.password) << ','
               << toCsvCell(record.category) << ','
               << toCsvCell(record.note) << ','
               << (record.favorite ? "1" : "0") << ','
               << record.successCount << ','
               << record.failureCount << '\n';
    }
    return true;
}

bool PasswordLibraryTransferService::importCsv(const QString& filePath, PasswordImportResult* result, QString* errorMessage) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (errorMessage) {
            *errorMessage = file.errorString();
        }
        return false;
    }

    PasswordImportResult localResult;
    QSet<QString> knownPasswords;
    for (const PasswordRecord& record : m_repository.list()) {
        knownPasswords.insert(record.password);
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    bool firstLine = true;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (line.trimmed().isEmpty()) {
            ++localResult.skippedCount;
            continue;
        }

        const QStringList cells = parseCsvLine(line);
        if (firstLine && !cells.isEmpty() && cells.first().trimmed().compare("password", Qt::CaseInsensitive) == 0) {
            firstLine = false;
            continue;
        }
        firstLine = false;

        if (cells.isEmpty() || cells.first().isEmpty()) {
            ++localResult.skippedCount;
            ++localResult.invalidCount;
            continue;
        }

        const QString password = cells.value(0);
        if (knownPasswords.contains(password)) {
            ++localResult.skippedCount;
            ++localResult.duplicateCount;
            continue;
        }

        PasswordRecord record;
        record.password = password;
        record.category = cells.value(1);
        record.note = cells.value(2);
        record.favorite = cells.value(3).trimmed() == "1" || cells.value(3).trimmed().compare("true", Qt::CaseInsensitive) == 0 || cells.value(3).trimmed() == "是";
        record.successCount = cells.value(4).toInt();
        record.failureCount = cells.value(5).toInt();

        QString addError;
        if (!m_repository.add(record, &addError)) {
            if (errorMessage) {
                *errorMessage = addError;
            }
            return false;
        }
        knownPasswords.insert(record.password);
        ++localResult.importedCount;
    }

    if (result) {
        *result = localResult;
    }
    return true;
}

QStringList PasswordLibraryTransferService::parseCsvLine(const QString& line) const
{
    QStringList cells;
    QString cell;
    bool inQuotes = false;

    for (int i = 0; i < line.size(); ++i) {
        const QChar ch = line.at(i);
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line.at(i + 1) == '"') {
                cell.append('"');
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
            continue;
        }

        if (ch == ',' && !inQuotes) {
            cells.append(cell);
            cell.clear();
            continue;
        }
        cell.append(ch);
    }
    cells.append(cell);
    return cells;
}

QString PasswordLibraryTransferService::toCsvCell(const QString& value) const
{
    QString escaped = value;
    escaped.replace('"', "\"\"");
    if (escaped.contains(',') || escaped.contains('"') || escaped.contains('\n') || escaped.contains('\r')) {
        return '"' + escaped + '"';
    }
    return escaped;
}

} // namespace PasswordManager
