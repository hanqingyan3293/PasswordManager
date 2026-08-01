#include "PasswordManager/app/ShellActionService.h"

#include "PasswordManager/app/AppConfig.h"
#include "PasswordManager/app/AppLogger.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ArchiveScanner.h"
#include "PasswordManager/app/PasswordMatcher.h"
#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/ArchivePasswordRecord.h"
#include "PasswordManager/domain/ArchiveRecord.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QFileInfo>

namespace PasswordManager {

namespace {

QList<ArchivePasswordRecord> directoryHistoryForArchive(const QList<ArchivePasswordRecord>& allHistory, const ArchiveRecord& archive)
{
    QList<ArchivePasswordRecord> records;
    const QString archiveDirectory = QFileInfo(archive.path).absolutePath();
    for (const ArchivePasswordRecord& record : allHistory) {
        if (record.archiveId == archive.id) {
            continue;
        }
        if (QFileInfo(record.archivePath).absolutePath().compare(archiveDirectory, Qt::CaseInsensitive) == 0) {
            records.append(record);
        }
    }
    return records;
}

QString formatElapsed(qint64 elapsedMs)
{
    if (elapsedMs < 1000) {
        return QString("%1 ms").arg(elapsedMs);
    }
    return QString::number(elapsedMs / 1000.0, 'f', 2) + " s";
}

} // namespace

ShellActionService::ShellActionService(
    const AppPaths& paths,
    const ArchiveRepository& archiveRepository,
    const ArchivePasswordRepository& archivePasswordRepository,
    const PasswordRepository& passwordRepository,
    PasswordTestTaskManager& taskManager)
    : m_paths(paths)
    , m_archiveRepository(archiveRepository)
    , m_archivePasswordRepository(archivePasswordRepository)
    , m_passwordRepository(passwordRepository)
    , m_taskManager(taskManager)
{
}

ShellActionResult ShellActionService::enqueueArchivePasswordTests(const QString& archivePath) const
{
    ShellActionResult result = scanAndSaveArchive(archivePath);
    if (!result.success) {
        return result;
    }

    const ArchiveRecord archiveRecord = m_archiveRepository.findByPath(QFileInfo(archivePath).absoluteFilePath());
    if (archiveRecord.id <= 0) {
        result.message = "压缩包记录保存后无法重新读取。";
        result.success = false;
        return result;
    }

    if (m_taskManager.isNoPasswordArchive(archiveRecord.path)) {
        result.success = true;
        result.archiveId = archiveRecord.id;
        result.noPasswordArchive = true;
        result.message = "该压缩包无需密码，未加入密码库测试任务。";
        return result;
    }

    const QList<PasswordRecord> passwordRecords = m_passwordRepository.list();
    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    PasswordMatcher matcher;
    const QList<ArchivePasswordRecord> allHistory = m_archivePasswordRepository.list();
    const QStringList candidates = matcher.buildLayeredCandidates(
        settings.enableExactHistory ? m_archivePasswordRepository.listForArchive(archiveRecord.id) : QList<ArchivePasswordRecord>(),
        settings.enableExactHistory ? m_archivePasswordRepository.listForFullHash(archiveRecord.fullHash, archiveRecord.id) : QList<ArchivePasswordRecord>(),
        settings.enableDirectoryHistory ? directoryHistoryForArchive(allHistory, archiveRecord) : QList<ArchivePasswordRecord>(),
        settings.enableCategoryCandidates ? m_passwordRepository.listByCategory(archiveRecord.category) : QList<PasswordRecord>(),
        settings.enablePasswordLibrary ? passwordRecords : QList<PasswordRecord>(),
        settings.enableDescriptionFiles
            ? matcher.extractLocalDescriptionPasswords(archiveRecord.path, settings.maxDescriptionCandidates, settings.maxDescriptionFileBytes)
            : QStringList(),
        settings.maxCandidates);
    if (candidates.isEmpty()) {
        result.message = "密码库和同目录说明文件里都没有可用候选密码。";
        result.archiveId = archiveRecord.id;
        result.success = false;
        return result;
    }

    for (const QString& password : candidates) {
        int passwordId = 0;
        for (const PasswordRecord& passwordRecord : passwordRecords) {
            if (passwordRecord.password.trimmed() == password) {
                passwordId = passwordRecord.id;
                break;
            }
        }

        m_taskManager.enqueuePasswordTest(archiveRecord.id, passwordId, archiveRecord.path, password);
        ++result.enqueuedCount;
    }

    result.success = result.enqueuedCount > 0;
    result.archiveId = archiveRecord.id;
    result.message = QString("已加入 %1 个密码测试任务。").arg(result.enqueuedCount);
    return result;
}

ShellActionResult ShellActionService::lookupKnownPasswords(const QString& archivePath) const
{
    ShellActionResult result = scanAndSaveArchive(archivePath);
    if (!result.success) {
        return result;
    }

    const QList<ArchivePasswordRecord> known = m_archivePasswordRepository.list(QFileInfo(archivePath).absoluteFilePath());
    result.knownPasswordCount = known.size();
    result.noPasswordArchive = m_taskManager.isNoPasswordArchive(QFileInfo(archivePath).absoluteFilePath());
    result.message = result.noPasswordArchive
        ? "该压缩包无需密码。"
        : known.isEmpty()
        ? "没有找到这个压缩包的已知成功密码。"
        : QString("找到 %1 条这个压缩包的已知成功密码记录。").arg(known.size());
    return result;
}

ShellActionResult ShellActionService::scanFolder(const QString& folderPath) const
{
    ShellActionResult result;
    AppLogger(m_paths.logsDir()).archive("Folder scan requested: path=" + folderPath);
    const QFileInfo folderInfo(folderPath);
    if (!folderInfo.exists() || !folderInfo.isDir()) {
        result.message = "文件夹不存在或不受支持。";
        return result;
    }

    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    const ScanResult scanResult = ArchiveScanner(settings.calculateFullHashDuringScan).scanDirectory(folderInfo.absoluteFilePath());
    QString error;
    result.scannedCount = m_archiveRepository.upsertMany(scanResult.archives, &error);
    if (!error.isEmpty()) {
        result.message = "文件夹扫描结果保存失败：" + error;
        return result;
    }

    result.success = true;
    result.message = QString("已扫描 %1 个压缩包，跳过 %2 个文件。耗时：%3。模式：%4。")
        .arg(result.scannedCount)
        .arg(scanResult.skippedCount)
        .arg(formatElapsed(scanResult.elapsedMs))
        .arg(scanResult.fullHashCalculated ? "精确模式" : "快速模式");
    AppLogger(m_paths.logsDir()).archive(QString("Folder scan completed: scanned=%1 skipped=%2 elapsed_ms=%3 mode=%4 path=%5")
            .arg(result.scannedCount)
            .arg(scanResult.skippedCount)
            .arg(scanResult.elapsedMs)
            .arg(scanResult.fullHashCalculated ? "full_hash" : "quick_hash")
            .arg(folderInfo.absoluteFilePath()));
    return result;
}

ShellActionResult ShellActionService::scanAndSaveArchive(const QString& archivePath) const
{
    ShellActionResult result;

    const QFileInfo archiveInfo(archivePath);
    AppLogger(m_paths.logsDir()).archive("Archive scan requested: path=" + archiveInfo.absoluteFilePath());
    if (!archiveInfo.exists() || !archiveInfo.isFile() || !ArchiveScanner::isSupportedArchive(archivePath)) {
        result.message = "压缩包文件不存在或不受支持。";
        return result;
    }

    const SmartMatchSettings settings = AppConfig(m_paths).smartMatchSettings();
    const ScanResult scanResult = ArchiveScanner(settings.calculateFullHashDuringScan).scanFiles({archiveInfo.absoluteFilePath()});
    if (scanResult.archives.isEmpty()) {
        result.message = "压缩包扫描没有生成记录。";
        return result;
    }

    QString error;
    if (!m_archiveRepository.upsert(scanResult.archives.first(), &error)) {
        result.message = "压缩包记录保存失败：" + error;
        return result;
    }

    const ArchiveRecord archiveRecord = m_archiveRepository.findByPath(scanResult.archives.first().path);
    if (archiveRecord.id <= 0) {
        result.message = "压缩包记录保存后无法重新读取。";
        return result;
    }

    result.success = true;
    result.archiveId = archiveRecord.id;
    result.scannedCount = 1;
    result.message = QString("压缩包已扫描并保存。耗时：%1。模式：%2。")
        .arg(formatElapsed(scanResult.elapsedMs))
        .arg(scanResult.fullHashCalculated ? "精确模式" : "快速模式");
    AppLogger(m_paths.logsDir()).archive(QString("Archive scan completed: archive_id=%1 elapsed_ms=%2 mode=%3 path=%4")
            .arg(result.archiveId)
            .arg(scanResult.elapsedMs)
            .arg(scanResult.fullHashCalculated ? "full_hash" : "quick_hash")
            .arg(archiveRecord.path));
    return result;
}

} // namespace PasswordManager
