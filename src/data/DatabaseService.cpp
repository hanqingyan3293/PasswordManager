#include "PasswordManager/data/DatabaseService.h"

#include "PasswordManager/app/AppPaths.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace PasswordManager {

DatabaseService::DatabaseService(const AppPaths& paths)
    : m_paths(paths)
    , m_connectionName("PasswordManagerMain_" + QUuid::createUuid().toString(QUuid::WithoutBraces))
{
}

DatabaseService::~DatabaseService()
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::database(m_connectionName).close();
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseService::open()
{
    QDir().mkpath(m_paths.dataDir());

    QSqlDatabase db = QSqlDatabase::contains(m_connectionName)
        ? QSqlDatabase::database(m_connectionName)
        : QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    db.setDatabaseName(databasePath());

    if (!db.open()) {
        m_lastError = db.lastError().text();
        return false;
    }

    return migrate();
}

QString DatabaseService::lastError() const
{
    return m_lastError;
}

QString DatabaseService::connectionName() const
{
    return m_connectionName;
}

QString DatabaseService::databasePath() const
{
    return QDir(m_paths.dataDir()).filePath("passwordmanager.sqlite3");
}

bool DatabaseService::migrate()
{
    return execute(R"(
        CREATE TABLE IF NOT EXISTS database_info (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        )
    )") && execute(R"(
        CREATE TABLE IF NOT EXISTS passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            password TEXT NOT NULL,
            category TEXT NOT NULL DEFAULT '',
            note TEXT NOT NULL DEFAULT '',
            favorite INTEGER NOT NULL DEFAULT 0,
            success_count INTEGER NOT NULL DEFAULT 0,
            failure_count INTEGER NOT NULL DEFAULT 0,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        )
    )") && execute(R"(
        CREATE TABLE IF NOT EXISTS archives (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            path TEXT NOT NULL UNIQUE,
            file_name TEXT NOT NULL,
            extension TEXT NOT NULL,
            category TEXT NOT NULL DEFAULT '',
            size_bytes INTEGER NOT NULL,
            modified_at TEXT NOT NULL,
            quick_hash TEXT NOT NULL,
            full_hash TEXT NOT NULL DEFAULT '',
            scanned_at TEXT NOT NULL
        )
    )") && execute(R"(
        CREATE TABLE IF NOT EXISTS archive_passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            archive_id INTEGER NOT NULL,
            password_id INTEGER,
            password TEXT NOT NULL,
            success_count INTEGER NOT NULL DEFAULT 0,
            last_success_at TEXT NOT NULL,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(archive_id, password)
        )
    )") && execute(R"(
        CREATE TABLE IF NOT EXISTS extract_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            archive_id INTEGER NOT NULL,
            archive_path TEXT NOT NULL,
            output_directory TEXT NOT NULL,
            status TEXT NOT NULL,
            message TEXT NOT NULL,
            created_at TEXT NOT NULL
        )
    )") && execute(R"(
        CREATE TABLE IF NOT EXISTS password_test_tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            archive_id INTEGER NOT NULL DEFAULT 0,
            password_id INTEGER NOT NULL DEFAULT 0,
            archive_path TEXT NOT NULL,
            password TEXT NOT NULL,
            status TEXT NOT NULL,
            test_status TEXT NOT NULL,
            message TEXT NOT NULL,
            timeout_ms INTEGER NOT NULL DEFAULT 10000,
            created_at TEXT NOT NULL,
            updated_at TEXT NOT NULL
        )
    )") && executeOptionalAlter("ALTER TABLE archives ADD COLUMN full_hash TEXT NOT NULL DEFAULT ''")
        && executeOptionalAlter("ALTER TABLE archives ADD COLUMN category TEXT NOT NULL DEFAULT ''")
        && execute(R"(
            INSERT OR REPLACE INTO database_info (key, value)
            VALUES ('schema_version', '7')
        )");
}

bool DatabaseService::execute(const QString& sql)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (!query.exec(sql)) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseService::executeOptionalAlter(const QString& sql)
{
    QSqlQuery query(QSqlDatabase::database(m_connectionName));
    if (query.exec(sql)) {
        return true;
    }

    const QString error = query.lastError().text();
    if (error.contains("duplicate column", Qt::CaseInsensitive)) {
        return true;
    }

    m_lastError = error;
    return false;
}

} // namespace PasswordManager
