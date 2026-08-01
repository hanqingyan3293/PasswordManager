#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordLibraryTransferService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>
#include <QTextStream>

using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordImportResult;
using PasswordManager::PasswordLibraryTransferService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;

class PasswordLibraryTransferServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void exportsAndImportsCsv();
    void findsPasswordByPlaintext();
};

void PasswordLibraryTransferServiceTests::exportsAndImportsCsv()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordRepository repository(database.connectionName());
    PasswordRecord record;
    record.password = "p,ass\"word";
    record.category = "分类";
    record.note = "备注,带逗号";
    record.favorite = true;
    record.successCount = 2;
    record.failureCount = 1;
    QVERIFY(repository.add(record));

    PasswordLibraryTransferService transfer(repository);
    const QString csvPath = dir.filePath("passwords.csv");
    QString error;
    QVERIFY2(transfer.exportCsv(csvPath, &error), qPrintable(error));

    QFile importedCsv(dir.filePath("import.csv"));
    QVERIFY(importedCsv.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&importedCsv);
    stream.setEncoding(QStringConverter::Utf8);
    stream << "password,category,note,favorite,success_count,failure_count\n";
    stream << "\"new,password\",导入,\"带\"\"引号\",1,3,4\n";
    stream << "\"new,password\",导入,duplicate,1,0,0\n";
    stream << "\"p,ass\"\"word\",导入,existing,1,0,0\n";
    stream << ",empty-password,skip,0,0,0\n";
    importedCsv.close();

    PasswordImportResult result;
    QVERIFY2(transfer.importCsv(importedCsv.fileName(), &result, &error), qPrintable(error));
    QCOMPARE(result.importedCount, 1);
    QCOMPARE(result.skippedCount, 3);
    QCOMPARE(result.duplicateCount, 2);
    QCOMPARE(result.invalidCount, 1);

    const auto records = repository.list();
    QCOMPARE(records.size(), 2);
    QVERIFY(records.at(0).password == "new,password" || records.at(1).password == "new,password");
}

void PasswordLibraryTransferServiceTests::findsPasswordByPlaintext()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordRepository repository(database.connectionName());
    PasswordRecord record;
    record.password = "manual-success";
    record.category = "自动学习";
    QVERIFY(repository.add(record));

    const PasswordRecord found = repository.findByPassword("manual-success");
    QVERIFY(found.id > 0);
    QCOMPARE(found.password, QString("manual-success"));
    QCOMPARE(found.category, QString("自动学习"));

    const PasswordRecord missing = repository.findByPassword("missing-password");
    QCOMPARE(missing.id, 0);
}

QTEST_MAIN(PasswordLibraryTransferServiceTests)
#include "PasswordLibraryTransferServiceTests.moc"
