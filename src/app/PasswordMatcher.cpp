#include "PasswordManager/app/PasswordMatcher.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSet>
#include <algorithm>

namespace PasswordManager {

namespace {

QString normalizedCandidate(QString candidate)
{
    candidate = candidate.trimmed();
    static const QString wrappers = QStringLiteral("\"'`“”‘’[](){}<>【】");
    static const QString trailing = QStringLiteral(",，.。;；");

    while (!candidate.isEmpty() && wrappers.contains(candidate.front())) {
        candidate.remove(0, 1);
        candidate = candidate.trimmed();
    }

    while (!candidate.isEmpty() && (wrappers.contains(candidate.back()) || trailing.contains(candidate.back()))) {
        candidate.chop(1);
        candidate = candidate.trimmed();
    }

    return candidate;
}

void appendCandidate(QStringList& candidates, QSet<QString>& seen, const QString& raw, int limit)
{
    QString candidate = normalizedCandidate(raw);
    if (candidate.isEmpty() || candidate.size() > 128 || seen.contains(candidate)) {
        return;
    }

    seen.insert(candidate);
    candidates.append(candidate);
    if (limit > 0 && candidates.size() > limit) {
        candidates.removeLast();
        seen.remove(candidate);
    }
}

QList<ArchivePasswordRecord> sortedHistoryRecords(QList<ArchivePasswordRecord> records)
{
    std::sort(records.begin(), records.end(), [](const ArchivePasswordRecord& left, const ArchivePasswordRecord& right) {
        if (left.successCount != right.successCount) {
            return left.successCount > right.successCount;
        }
        if (left.lastSuccessAt != right.lastSuccessAt) {
            return left.lastSuccessAt > right.lastSuccessAt;
        }
        return left.id > right.id;
    });
    return records;
}

QList<PasswordRecord> sortedPasswordRecords(QList<PasswordRecord> records)
{
    std::sort(records.begin(), records.end(), [](const PasswordRecord& left, const PasswordRecord& right) {
        if (left.favorite != right.favorite) {
            return left.favorite;
        }
        if (left.successCount != right.successCount) {
            return left.successCount > right.successCount;
        }
        if (left.failureCount != right.failureCount) {
            return left.failureCount < right.failureCount;
        }
        if (left.updatedAt != right.updatedAt) {
            return left.updatedAt > right.updatedAt;
        }
        return left.id > right.id;
    });
    return records;
}

bool isUsefulDescriptionFileName(const QFileInfo& fileInfo, const QString& archiveBaseName)
{
    const QString lowerName = fileInfo.fileName().toLower();
    if (fileInfo.completeBaseName().compare(archiveBaseName, Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QStringList keywords = {
        QStringLiteral("readme"),
        QStringLiteral("password"),
        QStringLiteral("pass"),
        QStringLiteral("pwd"),
        QStringLiteral("密码"),
        QStringLiteral("说明"),
        QStringLiteral("解压"),
        QStringLiteral("提取")
    };
    for (const QString& keyword : keywords) {
        if (lowerName.contains(keyword)) {
            return true;
        }
    }

    return false;
}

QString readSmallTextFile(const QFileInfo& fileInfo)
{
    constexpr qint64 maxDescriptionBytes = 256 * 1024;
    if (!fileInfo.exists() || !fileInfo.isFile() || fileInfo.size() > maxDescriptionBytes) {
        return {};
    }

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray bytes = file.readAll();
    QString text = QString::fromUtf8(bytes);
    if (text.contains(QChar::ReplacementCharacter)) {
        text = QString::fromLocal8Bit(bytes);
    }
    return text;
}

} // namespace

QStringList PasswordMatcher::buildCandidates(const QList<PasswordRecord>& passwords, int limit) const
{
    return buildCandidates(passwords, {}, limit);
}

QStringList PasswordMatcher::buildCandidates(const QList<PasswordRecord>& passwords, const QStringList& extraPasswords, int limit) const
{
    return buildLayeredCandidates({}, {}, passwords, extraPasswords, limit);
}

QStringList PasswordMatcher::buildLayeredCandidates(
    const QList<ArchivePasswordRecord>& exactHistory,
    const QList<ArchivePasswordRecord>& directoryHistory,
    const QList<PasswordRecord>& passwords,
    const QStringList& extraPasswords,
    int limit) const
{
    QStringList candidates;
    QSet<QString> seen;

    for (const ArchivePasswordRecord& record : sortedHistoryRecords(exactHistory)) {
        appendCandidate(candidates, seen, record.password, limit);
        if (limit > 0 && candidates.size() >= limit) {
            return candidates;
        }
    }

    for (const ArchivePasswordRecord& record : sortedHistoryRecords(directoryHistory)) {
        appendCandidate(candidates, seen, record.password, limit);
        if (limit > 0 && candidates.size() >= limit) {
            return candidates;
        }
    }

    for (const PasswordRecord& record : sortedPasswordRecords(passwords)) {
        const QString password = record.password.trimmed();
        if (password.isEmpty() || seen.contains(password)) {
            continue;
        }

        seen.insert(password);
        candidates.append(password);
        if (limit > 0 && candidates.size() >= limit) {
            return candidates;
        }
    }

    for (const QString& password : extraPasswords) {
        appendCandidate(candidates, seen, password, limit);
        if (limit > 0 && candidates.size() >= limit) {
            break;
        }
    }

    return candidates;
}

QStringList PasswordMatcher::extractDescriptionPasswordsFromText(const QString& text, int limit) const
{
    QStringList candidates;
    QSet<QString> seen;
    const QList<QRegularExpression> patterns = {
        QRegularExpression(QStringLiteral(R"((?:解压密码|压缩包密码|压缩密码|密码|提取码)\s*[:：=]\s*([^\s,，;；。]+))")),
        QRegularExpression(
            QStringLiteral(R"((?:archive\s+password|extract\s+password|password|pass|pwd)\s*[:：=]\s*([^\s,，;；。]+))"),
            QRegularExpression::CaseInsensitiveOption)
    };

    for (const QRegularExpression& pattern : patterns) {
        QRegularExpressionMatchIterator matches = pattern.globalMatch(text);
        while (matches.hasNext()) {
            const QRegularExpressionMatch match = matches.next();
            appendCandidate(candidates, seen, match.captured(1), limit);
            if (limit > 0 && candidates.size() >= limit) {
                return candidates;
            }
        }
    }

    return candidates;
}

QStringList PasswordMatcher::extractLocalDescriptionPasswords(const QString& archivePath, int limit) const
{
    const QFileInfo archiveInfo(archivePath);
    const QDir directory(archiveInfo.absolutePath());
    if (!directory.exists()) {
        return {};
    }

    const QString archiveBaseName = archiveInfo.completeBaseName();
    const QStringList filters = {
        QStringLiteral("*.txt"),
        QStringLiteral("*.md"),
        QStringLiteral("*.nfo"),
        QStringLiteral("*.url")
    };

    QList<QFileInfo> descriptionFiles;
    QSet<QString> seenPaths;
    for (const QFileInfo& fileInfo : directory.entryInfoList(filters, QDir::Files | QDir::Readable, QDir::Name)) {
        if (!isUsefulDescriptionFileName(fileInfo, archiveBaseName)) {
            continue;
        }

        const QString path = fileInfo.absoluteFilePath();
        if (seenPaths.contains(path)) {
            continue;
        }

        seenPaths.insert(path);
        descriptionFiles.append(fileInfo);
    }

    std::sort(descriptionFiles.begin(), descriptionFiles.end(), [archiveBaseName](const QFileInfo& left, const QFileInfo& right) {
        const bool leftSameBase = left.completeBaseName().compare(archiveBaseName, Qt::CaseInsensitive) == 0;
        const bool rightSameBase = right.completeBaseName().compare(archiveBaseName, Qt::CaseInsensitive) == 0;
        if (leftSameBase != rightSameBase) {
            return leftSameBase;
        }
        return left.fileName().compare(right.fileName(), Qt::CaseInsensitive) < 0;
    });

    QStringList candidates;
    QSet<QString> seen;
    for (const QFileInfo& fileInfo : descriptionFiles) {
        const QStringList extracted = extractDescriptionPasswordsFromText(readSmallTextFile(fileInfo), limit);
        for (const QString& password : extracted) {
            appendCandidate(candidates, seen, password, limit);
            if (limit > 0 && candidates.size() >= limit) {
                return candidates;
            }
        }
    }

    return candidates;
}

} // namespace PasswordManager
