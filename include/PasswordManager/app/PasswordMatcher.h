#pragma once

#include "PasswordManager/domain/ArchivePasswordRecord.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QList>
#include <QStringList>

namespace PasswordManager {

class PasswordMatcher {
public:
    QStringList buildCandidates(const QList<PasswordRecord>& passwords, int limit = 100) const;
    QStringList buildCandidates(const QList<PasswordRecord>& passwords, const QStringList& extraPasswords, int limit = 100) const;
    QStringList buildLayeredCandidates(
        const QList<ArchivePasswordRecord>& exactHistory,
        const QList<ArchivePasswordRecord>& fullHashHistory,
        const QList<ArchivePasswordRecord>& directoryHistory,
        const QList<PasswordRecord>& categoryPasswords,
        const QList<PasswordRecord>& passwords,
        const QStringList& extraPasswords,
        int limit = 100) const;
    QStringList extractDescriptionPasswordsFromText(const QString& text, int limit = 20) const;
    QStringList extractLocalDescriptionPasswords(const QString& archivePath, int limit = 20, qint64 maxFileBytes = 256 * 1024) const;
};

} // namespace PasswordManager
