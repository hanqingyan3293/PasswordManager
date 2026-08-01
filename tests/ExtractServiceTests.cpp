#include "PasswordManager/app/ExtractService.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTest>

using PasswordManager::ExtractService;
using PasswordManager::ExtractStatus;

class ExtractServiceTests final : public QObject {
    Q_OBJECT

private slots:
    void extractsWithCorrectPassword();
    void rejectsWrongPassword();

private:
    QString projectPath(const QString& relativePath) const;
};

QString ExtractServiceTests::projectPath(const QString& relativePath) const
{
    const QDir root(QCoreApplication::applicationDirPath() + "/..");
    return root.absoluteFilePath(relativePath);
}

void ExtractServiceTests::extractsWithCorrectPassword()
{
    const QString output = projectPath("testdata/tmp/extract-test/correct");
    QDir(output).removeRecursively();

    const auto result = ExtractService(projectPath("tools/7zip/7z.exe"))
        .extract(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "pm-fixture-01", output);

    QCOMPARE(result.status, ExtractStatus::Success);
    QVERIFY(QFileInfo::exists(QDir(output).filePath("testdata/tmp/payload-01.bin")));
}

void ExtractServiceTests::rejectsWrongPassword()
{
    const QString output = projectPath("testdata/tmp/extract-test/wrong");
    QDir(output).removeRecursively();

    const auto result = ExtractService(projectPath("tools/7zip/7z.exe"))
        .extract(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "wrong-password", output);

    QCOMPARE(result.status, ExtractStatus::WrongPassword);
}

QTEST_MAIN(ExtractServiceTests)
#include "ExtractServiceTests.moc"

