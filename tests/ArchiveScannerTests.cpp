#include "PasswordManager/app/ArchiveScanner.h"

#include <QCryptographicHash>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using PasswordManager::ArchiveScanner;

class ArchiveScannerTests final : public QObject {
    Q_OBJECT

private slots:
    void supportsExpectedExtensions();
    void scansFilesAndSkipsUnsupported();
    void scansDirectoryRecursively();
    void canSkipFullHashForFastScan();

private:
    bool writeFile(const QString& path, const QByteArray& content);
};

bool ArchiveScannerTests::writeFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    return file.write(content) == content.size();
}

void ArchiveScannerTests::supportsExpectedExtensions()
{
    QVERIFY(ArchiveScanner::isSupportedArchive("a.zip"));
    QVERIFY(ArchiveScanner::isSupportedArchive("a.RAR"));
    QVERIFY(ArchiveScanner::isSupportedArchive("a.7z"));
    QVERIFY(!ArchiveScanner::isSupportedArchive("a.txt"));
}

void ArchiveScannerTests::scansFilesAndSkipsUnsupported()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString zip = dir.filePath("one.zip");
    const QString text = dir.filePath("skip.txt");
    QVERIFY(writeFile(zip, "zip-content"));
    QVERIFY(writeFile(text, "text-content"));

    const auto result = ArchiveScanner().scanFiles({zip, text});
    QCOMPARE(result.archives.size(), 1);
    QCOMPARE(result.skippedCount, 1);
    QCOMPARE(result.archives.first().fileName, QString("one.zip"));
    QCOMPARE(result.archives.first().extension, QString("zip"));
    QVERIFY(!result.archives.first().quickHash.isEmpty());
    QCOMPARE(result.archives.first().fullHash, QString::fromLatin1(QCryptographicHash::hash("zip-content", QCryptographicHash::Sha256).toHex()));
}

void ArchiveScannerTests::scansDirectoryRecursively()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QDir(dir.path()).mkpath("nested"));

    QVERIFY(writeFile(dir.filePath("root.7z"), "7z-content"));
    QVERIFY(writeFile(dir.filePath("nested/two.rar"), "rar-content"));
    QVERIFY(writeFile(dir.filePath("nested/skip.md"), "skip-content"));

    const auto result = ArchiveScanner().scanDirectory(dir.path());
    QCOMPARE(result.archives.size(), 2);
    QCOMPARE(result.skippedCount, 1);
}

void ArchiveScannerTests::canSkipFullHashForFastScan()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString zip = dir.filePath("fast.zip");
    QVERIFY(writeFile(zip, "fast-content"));

    const auto result = ArchiveScanner(false).scanFiles({zip});
    QCOMPARE(result.archives.size(), 1);
    QVERIFY(!result.archives.first().quickHash.isEmpty());
    QVERIFY(result.archives.first().fullHash.isEmpty());
    QVERIFY(!result.fullHashCalculated);
    QVERIFY(result.elapsedMs >= 0);
}

QTEST_MAIN(ArchiveScannerTests)
#include "ArchiveScannerTests.moc"
