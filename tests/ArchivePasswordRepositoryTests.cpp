#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/domain/ArchiveRecord.h"

#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::ArchivePasswordRepository;
using PasswordManager::ArchiveRecord;
using PasswordManager::ArchiveRepository;
using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;

class ArchivePasswordRepositoryTests final : public QObject {
    Q_OBJECT

private slots:
    void recordsSuccessAndUpdatesPasswordStats();
    void filtersByArchivePath();
    void listsByArchiveId();
    void removesHistoryRecordOnly();
    void clearsHistoryWhenArchiveHashChangesForSamePath();
};

void ArchivePasswordRepositoryTests::recordsSuccessAndUpdatesPasswordStats()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    QSqlDatabase db = QSqlDatabase::database(database.connectionName());
    QSqlQuery archive(db);
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/test.zip', 'test.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash', '2026-01-01T00:00:00')
    )"));
    const int archiveId = archive.lastInsertId().toInt();

    PasswordRepository passwordRepository(database.connectionName());
    PasswordRecord password;
    password.password = "secret";
    QString passwordError;
    QVERIFY2(passwordRepository.add(password, &passwordError), qPrintable(passwordError));
    const PasswordRecord savedPassword = passwordRepository.list("secret").first();

    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    QVERIFY(archivePasswordRepository.recordSuccess(archiveId, savedPassword.id, savedPassword.password));
    QVERIFY(passwordRepository.incrementStats(savedPassword.id, true));

    const auto history = archivePasswordRepository.list();
    QCOMPARE(history.size(), 1);
    QCOMPARE(history.first().archiveName, QString("test.zip"));
    QCOMPARE(history.first().password, QString("secret"));

    const PasswordRecord updatedPassword = passwordRepository.list("secret").first();
    QCOMPARE(updatedPassword.successCount, 1);
}

void ArchivePasswordRepositoryTests::filtersByArchivePath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    QSqlDatabase db = QSqlDatabase::database(database.connectionName());
    QSqlQuery archive(db);
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/one.zip', 'one.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash1', '2026-01-01T00:00:00')
    )"));
    const int firstArchiveId = archive.lastInsertId().toInt();
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/two.zip', 'two.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash2', '2026-01-01T00:00:00')
    )"));
    const int secondArchiveId = archive.lastInsertId().toInt();

    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    QVERIFY(archivePasswordRepository.recordSuccess(firstArchiveId, 0, "one-secret"));
    QVERIFY(archivePasswordRepository.recordSuccess(secondArchiveId, 0, "two-secret"));

    const auto filtered = archivePasswordRepository.list("C:/tmp/two.zip");
    QCOMPARE(filtered.size(), 1);
    QCOMPARE(filtered.first().archiveId, secondArchiveId);
    QCOMPARE(filtered.first().archivePath, QString("C:/tmp/two.zip"));
    QCOMPARE(filtered.first().password, QString("two-secret"));
}

void ArchivePasswordRepositoryTests::listsByArchiveId()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    QSqlDatabase db = QSqlDatabase::database(database.connectionName());
    QSqlQuery archive(db);
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/exact-one.zip', 'exact-one.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash1', '2026-01-01T00:00:00')
    )"));
    const int firstArchiveId = archive.lastInsertId().toInt();
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/exact-two.zip', 'exact-two.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash2', '2026-01-01T00:00:00')
    )"));
    const int secondArchiveId = archive.lastInsertId().toInt();

    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    QVERIFY(archivePasswordRepository.recordSuccess(firstArchiveId, 0, "one-secret"));
    QVERIFY(archivePasswordRepository.recordSuccess(secondArchiveId, 0, "two-secret"));

    const auto firstHistory = archivePasswordRepository.listForArchive(firstArchiveId);
    QCOMPARE(firstHistory.size(), 1);
    QCOMPARE(firstHistory.first().archiveId, firstArchiveId);
    QCOMPARE(firstHistory.first().password, QString("one-secret"));
}

void ArchivePasswordRepositoryTests::removesHistoryRecordOnly()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    QSqlDatabase db = QSqlDatabase::database(database.connectionName());
    QSqlQuery archive(db);
    QVERIFY(archive.exec(R"(
        INSERT INTO archives(path, file_name, extension, size_bytes, modified_at, quick_hash, scanned_at)
        VALUES('C:/tmp/remove.zip', 'remove.zip', 'zip', 10, '2026-01-01T00:00:00', 'hash', '2026-01-01T00:00:00')
    )"));
    const int archiveId = archive.lastInsertId().toInt();

    PasswordRepository passwordRepository(database.connectionName());
    PasswordRecord password;
    password.password = "remove-secret";
    QVERIFY(passwordRepository.add(password));
    const PasswordRecord savedPassword = passwordRepository.findByPassword("remove-secret");
    QVERIFY(savedPassword.id > 0);

    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    QVERIFY(archivePasswordRepository.recordSuccess(archiveId, savedPassword.id, savedPassword.password));

    const auto history = archivePasswordRepository.list();
    QCOMPARE(history.size(), 1);
    QVERIFY(archivePasswordRepository.remove(history.first().id));
    QCOMPARE(archivePasswordRepository.list().size(), 0);
    QCOMPARE(passwordRepository.findByPassword("remove-secret").id, savedPassword.id);
}

void ArchivePasswordRepositoryTests::clearsHistoryWhenArchiveHashChangesForSamePath()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchiveRecord first;
    first.path = "C:/tmp/same-name.rar";
    first.fileName = "same-name.rar";
    first.extension = "rar";
    first.sizeBytes = 10;
    first.modifiedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);
    first.quickHash = "old-hash";
    first.scannedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);

    QString error;
    QVERIFY2(archiveRepository.upsert(first, &error), qPrintable(error));
    const ArchiveRecord savedFirst = archiveRepository.findByPath(first.path);
    QVERIFY(savedFirst.id > 0);

    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    QVERIFY(archivePasswordRepository.recordSuccess(savedFirst.id, 0, "old-password"));
    QCOMPARE(archivePasswordRepository.list(first.path).size(), 1);

    ArchiveRecord second = first;
    second.sizeBytes = 20;
    second.quickHash = "new-hash";
    second.modifiedAt = QDateTime::fromString("2026-01-02T00:00:00", Qt::ISODate);
    second.scannedAt = QDateTime::fromString("2026-01-02T00:00:00", Qt::ISODate);
    QVERIFY2(archiveRepository.upsert(second, &error), qPrintable(error));

    const ArchiveRecord savedSecond = archiveRepository.findByPath(first.path);
    QCOMPARE(savedSecond.id, savedFirst.id);
    QCOMPARE(savedSecond.quickHash, QString("new-hash"));
    QCOMPARE(archivePasswordRepository.list(first.path).size(), 0);
}

QTEST_MAIN(ArchivePasswordRepositoryTests)
#include "ArchivePasswordRepositoryTests.moc"
