#include "PasswordManager/app/ShellActionService.h"

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

} // namespace

ShellActionService::ShellActionService(
    const ArchiveRepository& archiveRepository,
    const ArchivePasswordRepository& archivePasswordRepository,
    const PasswordRepository& passwordRepository,
    PasswordTestTaskManager& taskManager)
    : m_archiveRepository(archiveRepository)
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
    PasswordMatcher matcher;
    const QList<ArchivePasswordRecord> allHistory = m_archivePasswordRepository.list();
    const QStringList candidates = matcher.buildLayeredCandidates(
        m_archivePasswordRepository.listForArchive(archiveRecord.id),
        directoryHistoryForArchive(allHistory, archiveRecord),
        passwordRecords,
        matcher.extractLocalDescriptionPasswords(archiveRecord.path, 20),
        100);
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
    const QFileInfo folderInfo(folderPath);
    if (!folderInfo.exists() || !folderInfo.isDir()) {
        result.message = "文件夹不存在或不受支持。";
        return result;
    }

    const ScanResult scanResult = ArchiveScanner().scanDirectory(folderInfo.absoluteFilePath());
    QString error;
    result.scannedCount = m_archiveRepository.upsertMany(scanResult.archives, &error);
    if (!error.isEmpty()) {
        result.message = "文件夹扫描结果保存失败：" + error;
        return result;
    }

    result.success = true;
    result.message = QString("已扫描 %1 个压缩包，跳过 %2 个文件。").arg(result.scannedCount).arg(scanResult.skippedCount);
    return result;
}

ShellActionResult ShellActionService::scanAndSaveArchive(const QString& archivePath) const
{
    ShellActionResult result;

    const QFileInfo archiveInfo(archivePath);
    if (!archiveInfo.exists() || !archiveInfo.isFile() || !ArchiveScanner::isSupportedArchive(archivePath)) {
        result.message = "压缩包文件不存在或不受支持。";
        return result;
    }

    const ScanResult scanResult = ArchiveScanner().scanFiles({archiveInfo.absoluteFilePath()});
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
    result.message = "压缩包已扫描并保存。";
    return result;
}

} // namespace PasswordManager
