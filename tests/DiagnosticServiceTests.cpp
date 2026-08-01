#include "PasswordManager/app/AppLogger.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/DiagnosticService.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/domain/PasswordRecord.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppLogger;
using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::DiagnosticService;
using PasswordManager::PasswordRecord;
using PasswordManager::PasswordRepository;

class DiagnosticServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void exportsDiagnosticDirectory();
};

void DiagnosticServiceTests::exportsDiagnosticDirectory()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    const AppLogger logger(paths.logsDir());
    logger.info("diagnostic test log");
    logger.error("diagnostic error log");
    logger.archive("diagnostic archive log");
    logger.extract("diagnostic extract log");
    logger.database("diagnostic database log");

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordRepository repository(database.connectionName());
    PasswordRecord record;
    record.password = "diagnostic-password";
    QVERIFY(repository.add(record));

    QString outputDirectory;
    QString error;
    QVERIFY2(DiagnosticService(paths, database.connectionName()).exportDiagnosticPackage(&outputDirectory, &error), qPrintable(error));
    QVERIFY(QDir(outputDirectory).exists());
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("diagnostic.txt")));
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("app.log")));
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("error.log")));
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("archive.log")));
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("extract.log")));
    QVERIFY(QFile::exists(QDir(outputDirectory).filePath("database.log")));
}

QTEST_MAIN(DiagnosticServiceTests)
#include "DiagnosticServiceTests.moc"
