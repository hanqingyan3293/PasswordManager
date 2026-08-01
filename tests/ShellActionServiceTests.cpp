#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ShellActionService.h"
#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/ArchiveRecord.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppPaths;
using PasswordManager::ArchivePasswordRepository;
using PasswordManager::ArchiveRecord;
using PasswordManager::ArchiveRepository;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;
using PasswordManager::PasswordTestTaskManager;
using PasswordManager::ShellActionService;

class ShellActionServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void enqueuesPasswordCandidatesForSupportedArchive();
    void rejectsUnsupportedFile();
    void rejectsWhenPasswordLibraryIsEmpty();
    void usesCategoryCandidatesBeforeGlobalLibrary();
    void skipsPasswordLibraryTestForNoPasswordArchive();
    void findsKnownPasswordsForArchive();
    void scansFolderArchives();

private:
    QString projectPath(const QString& relativePath) const;
};

QString ShellActionServiceTests::projectPath(const QString& relativePath) const
{
    const QDir root(QCoreApplication::applicationDirPath() + "/..");
    return root.absoluteFilePath(relativePath);
}

void ShellActionServiceTests::enqueuesPasswordCandidatesForSupportedArchive()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());

    PasswordRecord password;
    password.password = "pm-fixture-01";
    QString passwordError;
    QVERIFY2(passwordRepository.add(password, &passwordError), qPrintable(passwordError));

    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));
    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).enqueueArchivePasswordTests(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"));

    QVERIFY2(result.success, qPrintable(result.message));
    QCOMPARE(result.enqueuedCount, 1);
    QVERIFY(result.archiveId > 0);
    QCOMPARE(taskManager.tasks().size(), 1);
    QCOMPARE(taskManager.tasks().first().archiveId, result.archiveId);
    QVERIFY(taskManager.tasks().first().passwordId > 0);

    const auto savedArchive = archiveRepository.findByPath(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"));
    QCOMPARE(savedArchive.id, result.archiveId);
}

void ShellActionServiceTests::rejectsUnsupportedFile()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString textPath = dir.filePath("note.txt");
    QFile file(textPath);
    QVERIFY(file.open(QIODevice::WriteOnly));
    QVERIFY(file.write("not an archive") > 0);
    file.close();

    AppPaths paths(dir.filePath("runtime"));
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());
    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));

    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).enqueueArchivePasswordTests(textPath);

    QVERIFY(!result.success);
    QCOMPARE(result.enqueuedCount, 0);
    QCOMPARE(taskManager.tasks().size(), 0);
}

void ShellActionServiceTests::rejectsWhenPasswordLibraryIsEmpty()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());
    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));

    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).enqueueArchivePasswordTests(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"));

    QVERIFY(!result.success);
    QVERIFY(result.archiveId > 0);
    QCOMPARE(result.enqueuedCount, 0);
    QCOMPARE(taskManager.tasks().size(), 0);
}

void ShellActionServiceTests::usesCategoryCandidatesBeforeGlobalLibrary()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());

    PasswordRecord globalPassword;
    globalPassword.password = "global-password";
    globalPassword.successCount = 50;
    QVERIFY(passwordRepository.add(globalPassword));

    PasswordRecord categoryPassword;
    categoryPassword.password = "category-password";
    categoryPassword.category = "资源包";
    QVERIFY(passwordRepository.add(categoryPassword));

    const QString archivePath = projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip");
    ArchiveRecord archiveRecord;
    archiveRecord.path = archivePath;
    archiveRecord.fileName = "fixture_01_password_pm-fixture-01.zip";
    archiveRecord.extension = "zip";
    archiveRecord.category = "资源包";
    archiveRecord.sizeBytes = 1;
    archiveRecord.modifiedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);
    archiveRecord.quickHash = "quick";
    archiveRecord.fullHash = "full";
    archiveRecord.scannedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);
    QString archiveError;
    QVERIFY2(archiveRepository.upsert(archiveRecord, &archiveError), qPrintable(archiveError));

    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));
    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).enqueueArchivePasswordTests(archivePath);

    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(taskManager.tasks().size() >= 2);
    QCOMPARE(taskManager.tasks().at(0).password, QString("category-password"));
    QCOMPARE(taskManager.tasks().at(1).password, QString("global-password"));
}

void ShellActionServiceTests::skipsPasswordLibraryTestForNoPasswordArchive()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());

    PasswordRecord password;
    password.password = "any-password";
    QString passwordError;
    QVERIFY2(passwordRepository.add(password, &passwordError), qPrintable(passwordError));

    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));
    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).enqueueArchivePasswordTests(projectPath("testdata/archives/fixture_21_no_password.zip"));

    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(result.noPasswordArchive);
    QCOMPARE(result.enqueuedCount, 0);
    QCOMPARE(taskManager.tasks().size(), 0);
    QCOMPARE(archivePasswordRepository.list().size(), 0);
}

void ShellActionServiceTests::findsKnownPasswordsForArchive()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());
    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));

    const QString archivePath = projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip");
    const auto scanned = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).lookupKnownPasswords(archivePath);
    QVERIFY2(scanned.success, qPrintable(scanned.message));
    QCOMPARE(scanned.knownPasswordCount, 0);

    QVERIFY(archivePasswordRepository.recordSuccess(scanned.archiveId, 0, "pm-fixture-01"));
    const auto found = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).lookupKnownPasswords(archivePath);
    QVERIFY2(found.success, qPrintable(found.message));
    QCOMPARE(found.knownPasswordCount, 1);
}

void ShellActionServiceTests::scansFolderArchives()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository archiveRepository(database.connectionName());
    ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordRepository passwordRepository(database.connectionName());
    PasswordTestTaskManager taskManager(projectPath("tools/7zip/7z.exe"));

    const auto result = ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).scanFolder(projectPath("testdata/archives"));
    QVERIFY2(result.success, qPrintable(result.message));
    QVERIFY(result.scannedCount >= 21);
    QVERIFY(archiveRepository.list().size() >= 21);
}

QTEST_MAIN(ShellActionServiceTests)
#include "ShellActionServiceTests.moc"
