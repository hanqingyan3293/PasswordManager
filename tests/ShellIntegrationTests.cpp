#include "PasswordManager/app/ShellIntegration.h"

#include <QTest>

using PasswordManager::ShellIntegration;

class ShellIntegrationTests final : public QObject {
    Q_OBJECT

private slots:
    void exposesSupportedExtensions();
    void buildsRegistryRootKeys();
    void buildsMenuClassKeys();
    void buildsUninstallRegistryKeys();
    void buildsQuotedCommands();
    void buildsCompressCommand();
    void matchesCommandsForExecutableAndAction();
    void buildsNativeWindowsCommands();
};

void ShellIntegrationTests::exposesSupportedExtensions()
{
    QCOMPARE(ShellIntegration::supportedExtensions(), QStringList({".zip", ".rar", ".7z"}));
}

void ShellIntegrationTests::buildsRegistryRootKeys()
{
    QCOMPARE(
        ShellIntegration::menuRootKey(".zip"),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\.zip\\shell\\PasswordManager"));
    QCOMPARE(
        ShellIntegration::directoryMenuRootKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\PasswordManager"));
    QCOMPARE(
        ShellIntegration::fileMenuRootKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\PasswordManager"));
    QCOMPARE(
        ShellIntegration::directoryBackgroundMenuRootKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\PasswordManager"));
}

void ShellIntegrationTests::buildsMenuClassKeys()
{
    QCOMPARE(
        ShellIntegration::archiveMenuClassKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.ArchiveMenu"));
    QCOMPARE(
        ShellIntegration::directoryMenuClassKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.DirectoryMenu"));
    QCOMPARE(
        ShellIntegration::fileMenuClassKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.FileMenu"));
    QCOMPARE(
        ShellIntegration::directoryBackgroundMenuClassKey(),
        QString("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.DirectoryBackgroundMenu"));
}

void ShellIntegrationTests::buildsUninstallRegistryKeys()
{
    const QStringList keys = ShellIntegration::uninstallRegistryKeys();
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\PasswordManager"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\PasswordManager"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\PasswordManager"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\.zip\\shell\\PasswordManager"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.ArchiveMenu"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.FileMenu"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.DirectoryMenu"));
    QVERIFY(keys.contains("HKEY_CURRENT_USER\\Software\\Classes\\PasswordManager.DirectoryBackgroundMenu"));
}

void ShellIntegrationTests::buildsQuotedCommands()
{
    QCOMPARE(
        ShellIntegration::commandFor("C:/Apps/PasswordManager.exe", "view-results"),
        QString("\"C:\\Apps\\PasswordManager.exe\" --shell-action view-results \"%1\""));
}

void ShellIntegrationTests::buildsCompressCommand()
{
    QCOMPARE(
        ShellIntegration::compressCommandFor("C:/Apps/PasswordManager.exe"),
        QString("\"C:\\Apps\\tools\\7zip\\7zFM.exe\" \"%1\""));
    QCOMPARE(
        ShellIntegration::compressCommandFor("C:/Apps/PasswordManager.exe", "%V"),
        QString("\"C:\\Apps\\tools\\7zip\\7zFM.exe\" \"%V\""));
    QCOMPARE(
        ShellIntegration::commandFor("C:/Apps/PasswordManager.exe", "compress-archive"),
        QString("\"C:\\Apps\\PasswordManager.exe\" --shell-action compress-archive \"%1\""));
}

void ShellIntegrationTests::matchesCommandsForExecutableAndAction()
{
    QVERIFY(ShellIntegration::commandMatches(
        "\"C:\\Apps\\PasswordManager.exe\" --shell-action use-password-library-test \"%1\"",
        "C:/Apps/PasswordManager.exe",
        "use-password-library-test"));
    QVERIFY(!ShellIntegration::commandMatches(
        "\"C:\\Old\\PasswordManager.exe\" --shell-action use-password-library-test \"%1\"",
        "C:/Apps/PasswordManager.exe",
        "use-password-library-test"));
    QVERIFY(!ShellIntegration::commandMatches(
        "\"C:\\Apps\\PasswordManager.exe\" --shell-action view-results \"%1\"",
        "C:/Apps/PasswordManager.exe",
        "use-password-library-test"));
}

void ShellIntegrationTests::buildsNativeWindowsCommands()
{
    const QString command = ShellIntegration::commandFor(
        "D:/File/GitHub/PasswordManager/out/PasswordManager-portable/PasswordManager.exe",
        "open-main");
    QVERIFY(command.startsWith("\"D:\\File\\GitHub\\PasswordManager\\out\\PasswordManager-portable\\PasswordManager.exe\""));
    QVERIFY(command.contains("--shell-action open-main"));
    QVERIFY(command.endsWith("\"%1\""));
}

QTEST_MAIN(ShellIntegrationTests)
#include "ShellIntegrationTests.moc"
