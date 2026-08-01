#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ArchiveFingerprintService.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/domain/ArchiveRecord.h"

#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppPaths;
using PasswordManager::ArchiveFingerprintService;
using PasswordManager::ArchiveRecord;
using PasswordManager::ArchiveRepository;
using PasswordManager::DatabaseService;

class ArchiveFingerprintServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void backfillsMissingFullHashesAndKeepsMissingFiles();
};

void ArchiveFingerprintServiceTests::backfillsMissingFullHashesAndKeepsMissingFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString archivePath = dir.filePath("one.zip");
    QFile archive(archivePath);
    QVERIFY(archive.open(QIODevice::WriteOnly));
    archive.write("archive-content");
    archive.close();

    AppPaths paths(dir.filePath("runtime"));
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    ArchiveRepository repository(database.connectionName());
    ArchiveRecord existing;
    existing.path = archivePath;
    existing.fileName = "one.zip";
    existing.extension = "zip";
    existing.sizeBytes = QFileInfo(archivePath).size();
    existing.modifiedAt = QFileInfo(archivePath).lastModified();
    existing.quickHash = "quick";
    existing.scannedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);
    QString error;
    QVERIFY2(repository.upsert(existing, &error), qPrintable(error));

    ArchiveRecord missing = existing;
    missing.path = dir.filePath("missing.zip");
    missing.fileName = "missing.zip";
    QVERIFY2(repository.upsert(missing, &error), qPrintable(error));

    const auto result = ArchiveFingerprintService(repository).backfillMissingFullHashes();
    QCOMPARE(result.totalMissing, 2);
    QCOMPARE(result.updated, 1);
    QCOMPARE(result.missingFiles, 1);
    QCOMPARE(result.failed, 0);

    const ArchiveRecord updated = repository.findByPath(archivePath);
    QCOMPARE(updated.fullHash, QString::fromLatin1(QCryptographicHash::hash("archive-content", QCryptographicHash::Sha256).toHex()));
    QCOMPARE(repository.findByPath(missing.path).id > 0, true);
}

QTEST_MAIN(ArchiveFingerprintServiceTests)
#include "ArchiveFingerprintServiceTests.moc"
