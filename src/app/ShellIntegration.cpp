#include "PasswordManager/app/ShellIntegration.h"

#include "PasswordManager/app/AppConfig.h"

#include <QDir>
#include <QFileInfo>
#include <QPair>
#include <QSettings>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace PasswordManager {

namespace {

constexpr auto kMenuText = "压缩包密码管理器";
constexpr auto kArchiveMenuClass = "PasswordManager.ArchiveMenu";
constexpr auto kFileMenuClass = "PasswordManager.FileMenu";
constexpr auto kDirectoryMenuClass = "PasswordManager.DirectoryMenu";
constexpr auto kDirectoryBackgroundMenuClass = "PasswordManager.DirectoryBackgroundMenu";

#ifdef Q_OS_WIN
QString hkcuSubKey(const QString& fullKey)
{
    const QString prefix = "HKEY_CURRENT_USER\\";
    if (!fullKey.startsWith(prefix, Qt::CaseInsensitive)) {
        return {};
    }
    return fullKey.mid(prefix.size());
}

bool nativeRegistryKeyExists(const QString& fullKey)
{
    const QString subKey = hkcuSubKey(fullKey);
    if (subKey.isEmpty()) {
        return false;
    }

    HKEY key = nullptr;
    const LSTATUS status = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        reinterpret_cast<LPCWSTR>(subKey.utf16()),
        0,
        KEY_READ,
        &key);
    if (status == ERROR_SUCCESS) {
        RegCloseKey(key);
        return true;
    }
    return false;
}

QString nativeRegistryStringValue(const QString& fullKey, const QString& valueName)
{
    const QString subKey = hkcuSubKey(fullKey);
    if (subKey.isEmpty()) {
        return {};
    }

    HKEY key = nullptr;
    LSTATUS status = RegOpenKeyExW(
        HKEY_CURRENT_USER,
        reinterpret_cast<LPCWSTR>(subKey.utf16()),
        0,
        KEY_READ,
        &key);
    if (status != ERROR_SUCCESS) {
        return {};
    }

    DWORD type = 0;
    DWORD size = 0;
    const LPCWSTR nativeValueName = valueName == "."
        ? nullptr
        : reinterpret_cast<LPCWSTR>(valueName.utf16());
    status = RegQueryValueExW(
        key,
        nativeValueName,
        nullptr,
        &type,
        nullptr,
        &size);
    if (status != ERROR_SUCCESS || type != REG_SZ || size == 0) {
        RegCloseKey(key);
        return {};
    }

    QString value;
    value.resize(static_cast<int>(size / sizeof(wchar_t)));
    status = RegQueryValueExW(
        key,
        nativeValueName,
        nullptr,
        nullptr,
        reinterpret_cast<LPBYTE>(value.data()),
        &size);
    RegCloseKey(key);
    if (status != ERROR_SUCCESS) {
        return {};
    }

    while (!value.isEmpty() && value.back() == QChar('\0')) {
        value.chop(1);
    }
    return value;
}

bool removeNativeRegistryTree(const QString& parentKey, const QString& childName, QString* errorMessage, const QString& failureMessage)
{
    const QString fullKey = parentKey + "\\" + childName;
    const QString subKey = hkcuSubKey(fullKey);
    if (subKey.isEmpty()) {
        return true;
    }

    const LSTATUS status = RegDeleteTreeW(HKEY_CURRENT_USER, reinterpret_cast<LPCWSTR>(subKey.utf16()));
    if (status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND) {
        return true;
    }

    if (errorMessage) {
        *errorMessage = failureMessage + QString(" Windows错误码：%1").arg(static_cast<unsigned long>(status));
    }
    return false;
}
#endif

bool registryKeyExists(const QString& fullKey)
{
#ifdef Q_OS_WIN
    return nativeRegistryKeyExists(fullKey);
#else
    QSettings key(fullKey, QSettings::NativeFormat);
    return !key.childKeys().isEmpty() || !key.childGroups().isEmpty();
#endif
}

QString registryStringValue(const QString& fullKey, const QString& valueName)
{
#ifdef Q_OS_WIN
    return nativeRegistryStringValue(fullKey, valueName);
#else
    QSettings key(fullKey, QSettings::NativeFormat);
    return key.value(valueName).toString();
#endif
}

bool registryValueEquals(const QString& fullKey, const QString& valueName, const QString& expected)
{
    return registryStringValue(fullKey, valueName) == expected;
}

bool removeRegistrySubtree(const QString& parentKey, const QString& childName, QString* errorMessage, const QString& failureMessage)
{
    QSettings parent(parentKey, QSettings::NativeFormat);
    parent.remove(childName);
    parent.sync();
    if (parent.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = failureMessage;
        }
        return false;
    }
#ifdef Q_OS_WIN
    return removeNativeRegistryTree(parentKey, childName, errorMessage, failureMessage);
#else
    return true;
#endif
}

bool writeMenuCommand(const QString& menuClassKey, const QString& itemKey, const QString& text, const QString& command)
{
    QSettings item(menuClassKey + "\\Shell\\" + itemKey, QSettings::NativeFormat);
    item.setValue("MUIVerb", text);
    item.sync();

    QSettings commandKey(menuClassKey + "\\Shell\\" + itemKey + "\\command", QSettings::NativeFormat);
    commandKey.setValue(".", command);
    commandKey.sync();

    return item.status() == QSettings::NoError && commandKey.status() == QSettings::NoError;
}

} // namespace

QStringList ShellIntegration::supportedExtensions()
{
    return {".zip", ".rar", ".7z"};
}

QString ShellIntegration::menuRootKey(const QString& extension)
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\" + extension + "\\shell\\PasswordManager";
}

QString ShellIntegration::fileMenuRootKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\PasswordManager";
}

QString ShellIntegration::directoryMenuRootKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell\\PasswordManager";
}

QString ShellIntegration::directoryBackgroundMenuRootKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell\\PasswordManager";
}

QString ShellIntegration::archiveMenuClassKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\" + QString(kArchiveMenuClass);
}

QString ShellIntegration::fileMenuClassKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\" + QString(kFileMenuClass);
}

QString ShellIntegration::directoryMenuClassKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\" + QString(kDirectoryMenuClass);
}

QString ShellIntegration::directoryBackgroundMenuClassKey()
{
    return "HKEY_CURRENT_USER\\Software\\Classes\\" + QString(kDirectoryBackgroundMenuClass);
}

QStringList ShellIntegration::uninstallRegistryKeys()
{
    QStringList keys;
    const auto appendUnique = [&keys](const QString& key) {
        if (!keys.contains(key)) {
            keys.append(key);
        }
    };

    for (const QString& extension : supportedExtensions()) {
        appendUnique(menuRootKey(extension));
    }

    appendUnique(fileMenuRootKey());
    appendUnique(directoryMenuRootKey());
    appendUnique(directoryBackgroundMenuRootKey());
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\Folder\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\CompressedFolder\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\.zip\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\.rar\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\.7z\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\zipfile\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\rarfile\\shell\\PasswordManager");
    appendUnique("HKEY_CURRENT_USER\\Software\\Classes\\7zfile\\shell\\PasswordManager");
    appendUnique(archiveMenuClassKey());
    appendUnique(fileMenuClassKey());
    appendUnique(directoryMenuClassKey());
    appendUnique(directoryBackgroundMenuClassKey());
    return keys;
}

QString ShellIntegration::commandFor(const QString& executablePath, const QString& action, const QString& filePlaceholder)
{
    return QString("\"%1\" --shell-action %2 \"%3\"").arg(QDir::toNativeSeparators(executablePath), action, filePlaceholder);
}

QString ShellIntegration::compressCommandFor(const QString& executablePath, const QString& filePlaceholder)
{
    const QString sevenZipFileManager = QDir(QFileInfo(executablePath).absolutePath()).filePath("tools/7zip/7zFM.exe");
    return QString("\"%1\" \"%2\"").arg(QDir::toNativeSeparators(sevenZipFileManager), filePlaceholder);
}

bool ShellIntegration::commandMatches(
    const QString& command,
    const QString& executablePath,
    const QString& action,
    const QString& filePlaceholder)
{
    return command == commandFor(executablePath, action, filePlaceholder);
}

bool ShellIntegration::install(const QString& executablePath, QString* errorMessage) const
{
    return install(executablePath, ShellMenuSettings(), errorMessage);
}

bool ShellIntegration::install(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const
{
    const bool archiveEnabled = settings.enableArchiveLookup
        || settings.enableArchiveTest
        || settings.enableArchiveViewResults
        || settings.enableArchiveExtract
        || settings.enableArchiveCompress
        || settings.enableArchiveOpenMain;
    const bool directoryEnabled = settings.enableFolderScan || settings.enableFolderCompress || settings.enableFolderOpenMain;
    const bool directoryBackgroundEnabled = settings.enableFolderCompress || settings.enableFolderOpenMain;

    if (archiveEnabled) {
        if (!installArchiveMenuClass(executablePath, settings, errorMessage)) {
            return false;
        }
        for (const QString& extension : supportedExtensions()) {
            if (!installForExtension(extension, executablePath, errorMessage)) {
                return false;
            }
        }
    }
    if (settings.enableFileCompress) {
        if (!installFileMenuClass(executablePath, settings, errorMessage) || !installForFile(executablePath, errorMessage)) {
            return false;
        }
    }
    if (directoryEnabled) {
        if (!installDirectoryMenuClass(executablePath, settings, errorMessage) || !installForDirectory(executablePath, errorMessage)) {
            return false;
        }
    }
    if (directoryBackgroundEnabled) {
        if (!installDirectoryBackgroundMenuClass(executablePath, settings, errorMessage) || !installForDirectoryBackground(executablePath, errorMessage)) {
            return false;
        }
    }
    return true;
}

bool ShellIntegration::uninstall(QString* errorMessage) const
{
    if (!uninstallLegacyRoots(errorMessage)) {
        return false;
    }
    for (const QString& extension : supportedExtensions()) {
        if (!uninstallForExtension(extension, errorMessage)) {
            return false;
        }
    }
    if (!uninstallForFile(errorMessage)) {
        return false;
    }
    if (!uninstallForDirectory(errorMessage)) {
        return false;
    }
    if (!uninstallForDirectoryBackground(errorMessage)) {
        return false;
    }
    if (!uninstallMenuClass(archiveMenuClassKey(), errorMessage)) {
        return false;
    }
    if (!uninstallMenuClass(fileMenuClassKey(), errorMessage)) {
        return false;
    }
    if (!uninstallMenuClass(directoryMenuClassKey(), errorMessage)) {
        return false;
    }
    if (!uninstallMenuClass(directoryBackgroundMenuClassKey(), errorMessage)) {
        return false;
    }

    QStringList leftovers;
    for (const QString& key : uninstallRegistryKeys()) {
        if (registryKeyExists(key)) {
            leftovers.append(key);
        }
    }

    if (!leftovers.isEmpty()) {
        if (errorMessage) {
            *errorMessage = "右键菜单卸载后仍有残留注册表项：\n" + leftovers.join("\n");
        }
        return false;
    }

    return true;
}

bool ShellIntegration::isInstalled() const
{
    for (const QString& extension : supportedExtensions()) {
        if (!registryValueEquals(menuRootKey(extension), "MUIVerb", kMenuText)
            || !registryValueEquals(menuRootKey(extension), "ExtendedSubCommandsKey", kArchiveMenuClass)) {
            return false;
        }
    }

    if (!registryValueEquals(fileMenuRootKey(), "MUIVerb", kMenuText)
        || !registryValueEquals(fileMenuRootKey(), "ExtendedSubCommandsKey", kFileMenuClass)) {
        return false;
    }

    if (!registryValueEquals(directoryMenuRootKey(), "MUIVerb", kMenuText)
        || !registryValueEquals(directoryMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryMenuClass)) {
        return false;
    }

    return registryValueEquals(directoryBackgroundMenuRootKey(), "MUIVerb", kMenuText)
        && registryValueEquals(directoryBackgroundMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryBackgroundMenuClass);
}

bool ShellIntegration::isInstalledForExecutable(const QString& executablePath) const
{
    const QList<ShellIntegrationExtensionStatus> entries = status(executablePath);
    for (const ShellIntegrationExtensionStatus& entry : entries) {
        if (!entry.rootInstalled || !entry.addQueueInstalled || !entry.viewResultsInstalled || !entry.commandsPointToExecutable) {
            return false;
        }
    }
    return !entries.isEmpty();
}

QList<ShellIntegrationExtensionStatus> ShellIntegration::status(const QString& executablePath, const ShellMenuSettings& settings) const
{
    QList<ShellIntegrationExtensionStatus> entries;
    const auto commandOk = [&executablePath](const QString& classKey, const QString& itemKey, const QString& text, const QString& action, const QString& placeholder = "%1") {
        const QString itemPath = classKey + "\\Shell\\" + itemKey;
        const QString command = registryStringValue(itemPath + "\\command", ".");
        if (!registryValueEquals(itemPath, "MUIVerb", text)) {
            return false;
        }
        return executablePath.isEmpty() ? !command.isEmpty() : commandMatches(command, executablePath, action, placeholder);
    };
    const auto directCommandOk = [](const QString& classKey, const QString& itemKey, const QString& text, const QString& expectedCommand) {
        const QString itemPath = classKey + "\\Shell\\" + itemKey;
        return registryValueEquals(itemPath, "MUIVerb", text)
            && registryStringValue(itemPath + "\\command", ".") == expectedCommand;
    };
    const bool archiveEnabled = settings.enableArchiveLookup
        || settings.enableArchiveTest
        || settings.enableArchiveViewResults
        || settings.enableArchiveExtract
        || settings.enableArchiveCompress
        || settings.enableArchiveOpenMain;
    const bool directoryEnabled = settings.enableFolderScan || settings.enableFolderCompress || settings.enableFolderOpenMain;
    const bool directoryBackgroundEnabled = settings.enableFolderCompress || settings.enableFolderOpenMain;

    for (const QString& extension : supportedExtensions()) {
        ShellIntegrationExtensionStatus entry;
        entry.extension = extension;

        entry.rootInstalled = !archiveEnabled
            || (registryValueEquals(menuRootKey(extension), "MUIVerb", kMenuText)
                && registryValueEquals(menuRootKey(extension), "ExtendedSubCommandsKey", kArchiveMenuClass));

        entry.addQueueCommand = registryStringValue(archiveMenuClassKey() + "\\Shell\\library_test\\command", ".");
        entry.viewResultsCommand = registryStringValue(archiveMenuClassKey() + "\\Shell\\view_results\\command", ".");
        bool archiveCommandsOk = true;
        if (settings.enableArchiveLookup) {
            archiveCommandsOk = archiveCommandsOk && commandOk(archiveMenuClassKey(), "lookup_password", "自动查找密码", "lookup-password");
        }
        if (settings.enableArchiveTest) {
            archiveCommandsOk = archiveCommandsOk && commandOk(archiveMenuClassKey(), "library_test", "使用密码库测试", "use-password-library-test");
        }
        if (settings.enableArchiveViewResults) {
            archiveCommandsOk = archiveCommandsOk && commandOk(archiveMenuClassKey(), "view_results", "查看结果", "view-results");
        }
        if (settings.enableArchiveExtract) {
            archiveCommandsOk = archiveCommandsOk && commandOk(archiveMenuClassKey(), "extract_archive", "解压", "extract-archive");
        }
        if (settings.enableArchiveCompress) {
            archiveCommandsOk = archiveCommandsOk && directCommandOk(archiveMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath));
        }
        if (settings.enableArchiveOpenMain) {
            archiveCommandsOk = archiveCommandsOk && commandOk(archiveMenuClassKey(), "open_main", "打开主程序", "open-main");
        }
        entry.addQueueInstalled = archiveCommandsOk;
        entry.viewResultsInstalled = archiveCommandsOk;
        entry.commandsPointToExecutable = archiveCommandsOk;

        entries.append(entry);
    }

    ShellIntegrationExtensionStatus fileEntry;
    fileEntry.extension = "普通文件";
    fileEntry.rootInstalled = !settings.enableFileCompress
        || (registryValueEquals(fileMenuRootKey(), "MUIVerb", kMenuText)
            && registryValueEquals(fileMenuRootKey(), "ExtendedSubCommandsKey", kFileMenuClass));
    fileEntry.addQueueInstalled = !settings.enableFileCompress
        || commandOk(fileMenuClassKey(), "compress_archive", "打包压缩包", "compress-archive");
    fileEntry.addQueueCommand = registryStringValue(fileMenuClassKey() + "\\Shell\\compress_archive\\command", ".");
    fileEntry.viewResultsInstalled = fileEntry.addQueueInstalled;
    fileEntry.viewResultsCommand = fileEntry.addQueueCommand;
    fileEntry.commandsPointToExecutable = fileEntry.addQueueInstalled;
    entries.append(fileEntry);

    ShellIntegrationExtensionStatus directoryEntry;
    directoryEntry.extension = "文件夹";
    directoryEntry.rootInstalled = !directoryEnabled
        || (registryValueEquals(directoryMenuRootKey(), "MUIVerb", kMenuText)
            && registryValueEquals(directoryMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryMenuClass));

    directoryEntry.addQueueCommand = registryStringValue(directoryMenuClassKey() + "\\Shell\\scan_folder\\command", ".");
    directoryEntry.viewResultsCommand = registryStringValue(directoryMenuClassKey() + "\\Shell\\open_main\\command", ".");
    bool directoryCommandsOk = true;
    if (settings.enableFolderScan) {
        directoryCommandsOk = directoryCommandsOk && commandOk(directoryMenuClassKey(), "scan_folder", "扫描文件夹", "scan-folder");
    }
    if (settings.enableFolderCompress) {
        directoryCommandsOk = directoryCommandsOk && directCommandOk(directoryMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath));
    }
    if (settings.enableFolderOpenMain) {
        directoryCommandsOk = directoryCommandsOk && commandOk(directoryMenuClassKey(), "open_main", "打开主程序", "open-main");
    }
    directoryEntry.addQueueInstalled = directoryCommandsOk;
    directoryEntry.viewResultsInstalled = directoryCommandsOk;
    directoryEntry.commandsPointToExecutable = directoryCommandsOk;

    entries.append(directoryEntry);

    ShellIntegrationExtensionStatus directoryBackgroundEntry;
    directoryBackgroundEntry.extension = "文件夹空白处";
    directoryBackgroundEntry.rootInstalled = !directoryBackgroundEnabled
        || (registryValueEquals(directoryBackgroundMenuRootKey(), "MUIVerb", kMenuText)
            && registryValueEquals(directoryBackgroundMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryBackgroundMenuClass));
    directoryBackgroundEntry.addQueueInstalled = !settings.enableFolderCompress
        || directCommandOk(directoryBackgroundMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath, "%V"));
    directoryBackgroundEntry.addQueueCommand = registryStringValue(directoryBackgroundMenuClassKey() + "\\Shell\\compress_archive\\command", ".");
    directoryBackgroundEntry.viewResultsInstalled = !settings.enableFolderOpenMain
        || commandOk(directoryBackgroundMenuClassKey(), "open_main", "打开主程序", "open-main", "%V");
    directoryBackgroundEntry.viewResultsCommand = registryStringValue(directoryBackgroundMenuClassKey() + "\\Shell\\open_main\\command", ".");
    directoryBackgroundEntry.commandsPointToExecutable = directoryBackgroundEntry.addQueueInstalled && directoryBackgroundEntry.viewResultsInstalled;
    entries.append(directoryBackgroundEntry);
    return entries;
}

bool ShellIntegration::installForExtension(const QString& extension, const QString& executablePath, QString* errorMessage) const
{
    Q_UNUSED(executablePath);
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\" + extension + "\\shell",
            "PasswordManager",
            errorMessage,
            "清理旧压缩包右键菜单注册表失败。")) {
        return false;
    }

    QSettings root(menuRootKey(extension), QSettings::NativeFormat);
    root.setValue(".", "");
    root.setValue("MUIVerb", kMenuText);
    root.setValue("ExtendedSubCommandsKey", kArchiveMenuClass);
    root.sync();

    if (root.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = "写入压缩包右键菜单注册表失败。";
        }
        return false;
    }
    return true;
}

bool ShellIntegration::installForFile(const QString& executablePath, QString* errorMessage) const
{
    Q_UNUSED(executablePath);
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes\\*\\shell",
            "PasswordManager",
            errorMessage,
            "清理旧普通文件右键菜单注册表失败。")) {
        return false;
    }

    QSettings root(fileMenuRootKey(), QSettings::NativeFormat);
    root.setValue(".", "");
    root.setValue("MUIVerb", kMenuText);
    root.setValue("ExtendedSubCommandsKey", kFileMenuClass);
    root.sync();

    if (root.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = "写入普通文件右键菜单注册表失败。";
        }
        return false;
    }
    return true;
}

bool ShellIntegration::installForDirectory(const QString& executablePath, QString* errorMessage) const
{
    Q_UNUSED(executablePath);
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell",
            "PasswordManager",
            errorMessage,
            "清理旧文件夹右键菜单注册表失败。")) {
        return false;
    }

    QSettings root(directoryMenuRootKey(), QSettings::NativeFormat);
    root.setValue(".", "");
    root.setValue("MUIVerb", kMenuText);
    root.setValue("ExtendedSubCommandsKey", kDirectoryMenuClass);
    root.sync();

    if (root.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = "写入文件夹右键菜单注册表失败。";
        }
        return false;
    }
    return true;
}

bool ShellIntegration::installForDirectoryBackground(const QString& executablePath, QString* errorMessage) const
{
    Q_UNUSED(executablePath);
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell",
            "PasswordManager",
            errorMessage,
            "清理旧文件夹空白处右键菜单注册表失败。")) {
        return false;
    }

    QSettings root(directoryBackgroundMenuRootKey(), QSettings::NativeFormat);
    root.setValue(".", "");
    root.setValue("MUIVerb", kMenuText);
    root.setValue("ExtendedSubCommandsKey", kDirectoryBackgroundMenuClass);
    root.sync();

    if (root.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = "写入文件夹空白处右键菜单注册表失败。";
        }
        return false;
    }
    return true;
}

bool ShellIntegration::installArchiveMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kArchiveMenuClass,
            errorMessage,
            "清理压缩包右键菜单定义失败。")) {
        return false;
    }

    bool ok = true;
    if (settings.enableArchiveLookup) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "lookup_password", "自动查找密码", commandFor(executablePath, "lookup-password"));
    }
    if (settings.enableArchiveTest) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "library_test", "使用密码库测试", commandFor(executablePath, "use-password-library-test"));
    }
    if (settings.enableArchiveViewResults) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "view_results", "查看结果", commandFor(executablePath, "view-results"));
    }
    if (settings.enableArchiveExtract) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "extract_archive", "解压", commandFor(executablePath, "extract-archive"));
    }
    if (settings.enableArchiveCompress) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath));
    }
    if (settings.enableArchiveOpenMain) {
        ok = ok && writeMenuCommand(archiveMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main"));
    }

    if (!ok && errorMessage) {
        *errorMessage = "写入压缩包右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installFileMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kFileMenuClass,
            errorMessage,
            "清理普通文件右键菜单定义失败。")) {
        return false;
    }

    bool ok = true;
    if (settings.enableFileCompress) {
        ok = ok && writeMenuCommand(fileMenuClassKey(), "compress_archive", "打包压缩包", commandFor(executablePath, "compress-archive"));
    }
    if (!ok && errorMessage) {
        *errorMessage = "写入普通文件右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installDirectoryMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kDirectoryMenuClass,
            errorMessage,
            "清理文件夹右键菜单定义失败。")) {
        return false;
    }

    bool ok = true;
    if (settings.enableFolderScan) {
        ok = ok && writeMenuCommand(directoryMenuClassKey(), "scan_folder", "扫描文件夹", commandFor(executablePath, "scan-folder"));
    }
    if (settings.enableFolderCompress) {
        ok = ok && writeMenuCommand(directoryMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath));
    }
    if (settings.enableFolderOpenMain) {
        ok = ok && writeMenuCommand(directoryMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main"));
    }

    if (!ok && errorMessage) {
        *errorMessage = "写入文件夹右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installDirectoryBackgroundMenuClass(const QString& executablePath, const ShellMenuSettings& settings, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kDirectoryBackgroundMenuClass,
            errorMessage,
            "清理文件夹空白处右键菜单定义失败。")) {
        return false;
    }

    bool ok = true;
    if (settings.enableFolderCompress) {
        ok = ok && writeMenuCommand(directoryBackgroundMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath, "%V"));
    }
    if (settings.enableFolderOpenMain) {
        ok = ok && writeMenuCommand(directoryBackgroundMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main", "%V"));
    }

    if (!ok && errorMessage) {
        *errorMessage = "写入文件夹空白处右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::uninstallForExtension(const QString& extension, QString* errorMessage) const
{
    return removeRegistrySubtree(
        "HKEY_CURRENT_USER\\Software\\Classes\\SystemFileAssociations\\" + extension + "\\shell",
        "PasswordManager",
        errorMessage,
        "删除压缩包右键菜单注册表失败。");
}

bool ShellIntegration::uninstallForFile(QString* errorMessage) const
{
    return removeRegistrySubtree(
        "HKEY_CURRENT_USER\\Software\\Classes\\*\\shell",
        "PasswordManager",
        errorMessage,
        "删除普通文件右键菜单注册表失败。");
}

bool ShellIntegration::uninstallForDirectory(QString* errorMessage) const
{
    return removeRegistrySubtree(
        "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\shell",
        "PasswordManager",
        errorMessage,
        "删除文件夹右键菜单注册表失败。");
}

bool ShellIntegration::uninstallForDirectoryBackground(QString* errorMessage) const
{
    return removeRegistrySubtree(
        "HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell",
        "PasswordManager",
        errorMessage,
        "删除文件夹空白处右键菜单注册表失败。");
}

bool ShellIntegration::uninstallMenuClass(const QString& classKey, QString* errorMessage) const
{
    return removeRegistrySubtree(
        "HKEY_CURRENT_USER\\Software\\Classes",
        classKey.mid(QString("HKEY_CURRENT_USER\\Software\\Classes\\").size()),
        errorMessage,
        "删除右键菜单定义失败。");
}

bool ShellIntegration::uninstallLegacyRoots(QString* errorMessage) const
{
    const QList<QPair<QString, QString>> roots = {
        {"HKEY_CURRENT_USER\\Software\\Classes\\*\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\Folder\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\Directory\\Background\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\CompressedFolder\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\.zip\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\.rar\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\.7z\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\zipfile\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\rarfile\\shell", "PasswordManager"},
        {"HKEY_CURRENT_USER\\Software\\Classes\\7zfile\\shell", "PasswordManager"},
    };

    for (const auto& root : roots) {
        if (!removeRegistrySubtree(root.first, root.second, errorMessage, "清理历史右键菜单残留失败。")) {
            return false;
        }
    }
    return true;
}

} // namespace PasswordManager
