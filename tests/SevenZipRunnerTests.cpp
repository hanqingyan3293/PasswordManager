#include "PasswordManager/app/SevenZipRunner.h"

#include <QCoreApplication>
#include <QDir>
#include <QTest>

using PasswordManager::SevenZipRunner;
using PasswordManager::SevenZipTestStatus;

class SevenZipRunnerTests final : public QObject {
    Q_OBJECT

private slots:
    void acceptsCorrectZipPassword();
    void acceptsNoPasswordZip();
    void reportsNoPasswordZipWhenPasswordProvided();
    void rejectsWrongZipPassword();
    void acceptsCorrectSevenZipPassword();
    void reportsMissingSevenZip();

private:
    QString projectPath(const QString& relativePath) const;
};

QString SevenZipRunnerTests::projectPath(const QString& relativePath) const
{
    const QDir root(QCoreApplication::applicationDirPath() + "/..");
    return root.absoluteFilePath(relativePath);
}

void SevenZipRunnerTests::acceptsCorrectZipPassword()
{
    const SevenZipRunner runner(projectPath("tools/7zip/7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "pm-fixture-01");
    QCOMPARE(result.status, SevenZipTestStatus::Success);
}

void SevenZipRunnerTests::acceptsNoPasswordZip()
{
    const SevenZipRunner runner(projectPath("tools/7zip/7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_21_no_password.zip"), "");
    QCOMPARE(result.status, SevenZipTestStatus::Success);
}

void SevenZipRunnerTests::reportsNoPasswordZipWhenPasswordProvided()
{
    const SevenZipRunner runner(projectPath("tools/7zip/7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_21_no_password.zip"), "any-password");
    QCOMPARE(result.status, SevenZipTestStatus::NoPasswordRequired);
}

void SevenZipRunnerTests::rejectsWrongZipPassword()
{
    const SevenZipRunner runner(projectPath("tools/7zip/7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "wrong-password");
    QCOMPARE(result.status, SevenZipTestStatus::WrongPassword);
}

void SevenZipRunnerTests::acceptsCorrectSevenZipPassword()
{
    const SevenZipRunner runner(projectPath("tools/7zip/7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_02_password_pm-fixture-02.7z"), "pm-fixture-02");
    QCOMPARE(result.status, SevenZipTestStatus::Success);
}

void SevenZipRunnerTests::reportsMissingSevenZip()
{
    const SevenZipRunner runner(projectPath("tools/7zip/missing-7z.exe"));
    const auto result = runner.testPassword(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "pm-fixture-01");
    QCOMPARE(result.status, SevenZipTestStatus::MissingSevenZip);
}

QTEST_MAIN(SevenZipRunnerTests)
#include "SevenZipRunnerTests.moc"
