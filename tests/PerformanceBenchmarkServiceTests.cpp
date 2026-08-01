#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/PerformanceBenchmarkService.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;
using PasswordManager::PerformanceBenchmarkService;

class PerformanceBenchmarkServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void createsBenchmarkReport();
};

void PerformanceBenchmarkServiceTests::createsBenchmarkReport()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordRecord record;
    record.password = "benchmark-password";
    QVERIFY(PasswordRepository(database.connectionName()).add(record));

    QString outputPath;
    QString error;
    QVERIFY2(PerformanceBenchmarkService(paths, database.connectionName()).run(QString(), &outputPath, &error), qPrintable(error));
    QVERIFY(QFile::exists(outputPath));
}

QTEST_MAIN(PerformanceBenchmarkServiceTests)
#include "PerformanceBenchmarkServiceTests.moc"
