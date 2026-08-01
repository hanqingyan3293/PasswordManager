#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/data/DatabaseBackupService.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppPaths;
using PasswordManager::DatabaseBackupService;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;

class DatabaseBackupServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void createsBackupAndRestoresIt();
};

void DatabaseBackupServiceTests::createsBackupAndRestoresIt()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordRepository repository(database.connectionName());
    PasswordRecord original;
    original.password = "backup-original";
    original.category = "phase18";
    QVERIFY(repository.add(original));

    DatabaseBackupService backupService(database.connectionName(), database.databasePath(), paths.backupDir());
    QString backupPath;
    QString error;
    QVERIFY2(backupService.createBackup(&backupPath, &error), qPrintable(error));
    QVERIFY(QFileInfo::exists(backupPath));

    PasswordRecord changed;
    changed.password = "after-backup";
    changed.category = "phase18";
    QVERIFY(repository.add(changed));
    QCOMPARE(repository.list().size(), 2);

    QString safetyBackupPath;
    QVERIFY2(backupService.restoreFromBackup(backupPath, &safetyBackupPath, &error), qPrintable(error));
    QVERIFY(QFileInfo::exists(safetyBackupPath));

    QVERIFY2(database.open(), qPrintable(database.lastError()));
    const auto records = repository.list();
    QCOMPARE(records.size(), 1);
    QCOMPARE(records.first().password, QString("backup-original"));
}

QTEST_MAIN(DatabaseBackupServiceTests)
#include "DatabaseBackupServiceTests.moc"
