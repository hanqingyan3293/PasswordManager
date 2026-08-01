#pragma once

#include <QString>

namespace PasswordManager {

struct SevenZipStatus {
    bool exists = false;
    bool runnable = false;
    QString executablePath;
    QString version;
    QString errorMessage;
};

class SevenZipProbe {
public:
    SevenZipStatus probe(const QString& executablePath) const;
};

} // namespace PasswordManager

