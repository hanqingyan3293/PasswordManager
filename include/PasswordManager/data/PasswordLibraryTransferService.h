#pragma once

#include <QString>
#include <QStringList>

namespace PasswordManager {

class PasswordRepository;

struct PasswordImportResult {
    int importedCount = 0;
    int skippedCount = 0;
    int duplicateCount = 0;
    int invalidCount = 0;
};

class PasswordLibraryTransferService {
public:
    explicit PasswordLibraryTransferService(const PasswordRepository& repository);

    bool exportCsv(const QString& filePath, QString* errorMessage = nullptr) const;
    bool importCsv(const QString& filePath, PasswordImportResult* result = nullptr, QString* errorMessage = nullptr) const;

private:
    QStringList parseCsvLine(const QString& line) const;
    QString toCsvCell(const QString& value) const;

    const PasswordRepository& m_repository;
};

} // namespace PasswordManager
