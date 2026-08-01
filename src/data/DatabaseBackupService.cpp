#include "PasswordManager/data/DatabaseBackupService.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

#include <utility>

namespace PasswordManager {

DatabaseBackupService::DatabaseBackupService(QString connectionName, QString databasePath, QString backupDir)
    : m_connectionName(std::move(connectionName))
    , m_databasePath(std::move(databasePath))
    , m_backupDir(std::move(backupDir))
{
}

bool DatabaseBackupService::createBackup(QString* backupPath, QString* errorMessage) const
{
    if (!QFileInfo::exists(m_databasePath)) {
        if (errorMessage) {
            *errorMessage = "数据库文件不存在。";
        }
        return false;
    }

    if (!QDir().mkpath(m_backupDir)) {
        if (errorMessage) {
            *errorMessage = "无法创建备份目录。";
        }
        return false;
    }

    const QString destination = timestampedBackupPath("passwordmanager-backup");
    QFile::remove(destination);

    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec("VACUUM INTO " + sqliteStringLiteral(destination))) {
        if (errorMessage) {
            *errorMessage = query.lastError().text();
        }
        return false;
    }

    if (backupPath) {
        *backupPath = destination;
    }
    return true;
}

bool DatabaseBackupService::restoreFromBackup(const QString& sourcePath, QString* safetyBackupPath, QString* errorMessage) const
{
    if (!validateBackupFile(sourcePath, errorMessage)) {
        return false;
    }

    if (!QDir().mkpath(m_backupDir)) {
        if (errorMessage) {
            *errorMessage = "无法创建备份目录。";
        }
        return false;
    }

    const QString safetyPath = timestampedBackupPath("passwordmanager-before-restore");
    if (QFileInfo::exists(m_databasePath) && !QFile::copy(m_databasePath, safetyPath)) {
        if (errorMessage) {
            *errorMessage = "恢复前备份当前数据库失败。";
        }
        return false;
    }

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    db.close();
    db = QSqlDatabase();

    QFile::remove(m_databasePath + "-wal");
    QFile::remove(m_databasePath + "-shm");
    QFile::remove(m_databasePath + "-journal");
    if (QFileInfo::exists(m_databasePath) && !QFile::remove(m_databasePath)) {
        if (errorMessage) {
            *errorMessage = "无法替换当前数据库文件，请关闭占用后重试。";
        }
        return false;
    }

    if (!QFile::copy(sourcePath, m_databasePath)) {
        if (errorMessage) {
            *errorMessage = "复制备份文件到数据目录失败。";
        }
        return false;
    }

    if (safetyBackupPath) {
        *safetyBackupPath = safetyPath;
    }
    return true;
}

bool DatabaseBackupService::validateBackupFile(const QString& sourcePath, QString* errorMessage) const
{
    const QFileInfo sourceInfo(sourcePath);
    if (!sourceInfo.exists() || !sourceInfo.isFile()) {
        if (errorMessage) {
            *errorMessage = "备份文件不存在。";
        }
        return false;
    }

    if (sourceInfo.absoluteFilePath() == QFileInfo(m_databasePath).absoluteFilePath()) {
        if (errorMessage) {
            *errorMessage = "不能选择当前正在使用的数据库文件作为恢复源。";
        }
        return false;
    }

    const QString connectionName = "PasswordManagerBackupValidation_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(sourceInfo.absoluteFilePath());
    if (!db.open()) {
        if (errorMessage) {
            *errorMessage = "备份文件不是可打开的 SQLite 数据库。";
        }
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return false;
    }

    QSqlQuery query(db);
    const bool valid = query.exec("SELECT value FROM database_info WHERE key = 'schema_version'") && query.next();
    db.close();
    db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);

    if (!valid && errorMessage) {
        *errorMessage = "备份文件不是 PasswordManager 数据库。";
    }
    return valid;
}

QString DatabaseBackupService::timestampedBackupPath(const QString& prefix) const
{
    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss");
    return QDir(m_backupDir).filePath(prefix + "-" + timestamp + ".sqlite3");
}

QString DatabaseBackupService::sqliteStringLiteral(const QString& value) const
{
    QString escaped = QDir::toNativeSeparators(value);
    escaped.replace("'", "''");
    return "'" + escaped + "'";
}

} // namespace PasswordManager
