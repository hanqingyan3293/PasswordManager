#include "PasswordManager/app/PasswordMatcher.h"

#include <QFile>
#include <QTest>
#include <QTemporaryDir>
#include <QTextStream>

using PasswordManager::PasswordMatcher;
using PasswordManager::PasswordRecord;

class PasswordMatcherTests final : public QObject {
    Q_OBJECT

private slots:
    void sortsAndDeduplicatesCandidates();
    void respectsLimit();
    void ordersLayeredCandidates();
    void appendsDescriptionCandidatesAfterPasswordLibrary();
    void extractsPasswordsFromDescriptionText();
    void extractsLocalDescriptionPasswordsNearArchive();
};

void PasswordMatcherTests::sortsAndDeduplicatesCandidates()
{
    PasswordRecord plain;
    plain.id = 1;
    plain.password = "plain";
    plain.successCount = 10;
    plain.failureCount = 0;
    plain.updatedAt = QDateTime::fromString("2026-01-01T00:00:00", Qt::ISODate);

    PasswordRecord favorite;
    favorite.id = 2;
    favorite.password = "favorite";
    favorite.favorite = true;
    favorite.successCount = 1;
    favorite.failureCount = 99;

    PasswordRecord duplicate;
    duplicate.id = 3;
    duplicate.password = "plain";
    duplicate.favorite = true;

    PasswordRecord empty;
    empty.id = 4;
    empty.password = " ";

    const QStringList candidates = PasswordMatcher().buildCandidates({plain, favorite, duplicate, empty});
    QCOMPARE(candidates, QStringList({"favorite", "plain"}));
}

void PasswordMatcherTests::respectsLimit()
{
    PasswordRecord first;
    first.id = 1;
    first.password = "first";

    PasswordRecord second;
    second.id = 2;
    second.password = "second";

    const QStringList candidates = PasswordMatcher().buildCandidates({first, second}, 1);
    QCOMPARE(candidates.size(), 1);
    QCOMPARE(candidates.first(), QString("second"));
}

void PasswordMatcherTests::ordersLayeredCandidates()
{
    PasswordManager::ArchivePasswordRecord exact;
    exact.id = 1;
    exact.password = "exact";
    exact.successCount = 1;

    PasswordManager::ArchivePasswordRecord directory;
    directory.id = 2;
    directory.password = "directory";
    directory.successCount = 9;

    PasswordRecord favorite;
    favorite.id = 3;
    favorite.password = "favorite";
    favorite.favorite = true;

    PasswordRecord common;
    common.id = 4;
    common.password = "common";
    common.successCount = 20;

    PasswordRecord category;
    category.id = 6;
    category.password = "category";
    category.successCount = 30;

    PasswordManager::ArchivePasswordRecord fullHash;
    fullHash.id = 5;
    fullHash.password = "full-hash";
    fullHash.successCount = 5;

    const QStringList candidates = PasswordMatcher().buildLayeredCandidates({exact}, {fullHash}, {directory}, {category}, {common, favorite}, {"description", "exact"}, 10);
    QCOMPARE(candidates, QStringList({"exact", "full-hash", "directory", "category", "favorite", "common", "description"}));
}

void PasswordMatcherTests::appendsDescriptionCandidatesAfterPasswordLibrary()
{
    PasswordRecord libraryPassword;
    libraryPassword.id = 1;
    libraryPassword.password = "library";

    const QStringList candidates = PasswordMatcher().buildCandidates({libraryPassword}, {"sidecar", "library"}, 10);
    QCOMPARE(candidates, QStringList({"library", "sidecar"}));
}

void PasswordMatcherTests::extractsPasswordsFromDescriptionText()
{
    const QString text = QStringLiteral("解压密码：cn-123\npassword = \"en-456\"\npwd: `zip789`");
    const QStringList candidates = PasswordMatcher().extractDescriptionPasswordsFromText(text);

    QCOMPARE(candidates, QStringList({"cn-123", "en-456", "zip789"}));
}

void PasswordMatcherTests::extractsLocalDescriptionPasswordsNearArchive()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());

    QFile archive(directory.filePath("package.zip"));
    QVERIFY(archive.open(QIODevice::WriteOnly));
    archive.write("placeholder");
    archive.close();

    QFile sameBaseDescription(directory.filePath("package.txt"));
    QVERIFY(sameBaseDescription.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream sameBaseStream(&sameBaseDescription);
    sameBaseStream << QStringLiteral("密码：same-base");
    sameBaseDescription.close();

    QFile readmeDescription(directory.filePath("readme.md"));
    QVERIFY(readmeDescription.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream readmeStream(&readmeDescription);
    readmeStream << QStringLiteral("password: readme-pass");
    readmeDescription.close();

    const QStringList candidates = PasswordMatcher().extractLocalDescriptionPasswords(archive.fileName());
    QCOMPARE(candidates, QStringList({"same-base", "readme-pass"}));
}

QTEST_MAIN(PasswordMatcherTests)
#include "PasswordMatcherTests.moc"
