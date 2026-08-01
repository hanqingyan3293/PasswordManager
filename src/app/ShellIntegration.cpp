#include "PasswordManager/app/ShellIntegration.h"

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
    if (!installArchiveMenuClass(executablePath, errorMessage)) {
        return false;
    }
    if (!installFileMenuClass(executablePath, errorMessage)) {
        return false;
    }
    if (!installDirectoryMenuClass(executablePath, errorMessage)) {
        return false;
    }
    if (!installDirectoryBackgroundMenuClass(executablePath, errorMessage)) {
        return false;
    }
    for (const QString& extension : supportedExtensions()) {
        if (!installForExtension(extension, executablePath, errorMessage)) {
            return false;
        }
    }
    if (!installForFile(executablePath, errorMessage)) {
        return false;
    }
    if (!installForDirectory(executablePath, errorMessage)) {
        return false;
    }
    return installForDirectoryBackground(executablePath, errorMessage);
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

QList<ShellIntegrationExtensionStatus> ShellIntegration::status(const QString& executablePath) const
{
    QList<ShellIntegrationExtensionStatus> entries;
    for (const QString& extension : supportedExtensions()) {
        ShellIntegrationExtensionStatus entry;
        entry.extension = extension;

        entry.rootInstalled = registryValueEquals(menuRootKey(extension), "MUIVerb", kMenuText)
            && registryValueEquals(menuRootKey(extension), "ExtendedSubCommandsKey", kArchiveMenuClass);

        entry.addQueueInstalled = registryValueEquals(archiveMenuClassKey() + "\\Shell\\library_test", "MUIVerb", "使用密码库测试");

        entry.addQueueCommand = registryStringValue(archiveMenuClassKey() + "\\Shell\\library_test\\command", ".");

        entry.viewResultsInstalled = registryValueEquals(archiveMenuClassKey() + "\\Shell\\view_results", "MUIVerb", "查看结果");

        entry.viewResultsCommand = registryStringValue(archiveMenuClassKey() + "\\Shell\\view_results\\command", ".");

        entry.commandsPointToExecutable = executablePath.isEmpty()
            ? (!entry.addQueueCommand.isEmpty() && !entry.viewResultsCommand.isEmpty())
            : (commandMatches(entry.addQueueCommand, executablePath, "use-password-library-test")
                && commandMatches(entry.viewResultsCommand, executablePath, "view-results"));

        entries.append(entry);
    }

    ShellIntegrationExtensionStatus fileEntry;
    fileEntry.extension = "普通文件";
    fileEntry.rootInstalled = registryValueEquals(fileMenuRootKey(), "MUIVerb", kMenuText)
        && registryValueEquals(fileMenuRootKey(), "ExtendedSubCommandsKey", kFileMenuClass);
    fileEntry.addQueueInstalled = registryValueEquals(fileMenuClassKey() + "\\Shell\\compress_archive", "MUIVerb", "打包压缩包");
    fileEntry.addQueueCommand = registryStringValue(fileMenuClassKey() + "\\Shell\\compress_archive\\command", ".");
    fileEntry.viewResultsInstalled = fileEntry.addQueueInstalled;
    fileEntry.viewResultsCommand = fileEntry.addQueueCommand;
    fileEntry.commandsPointToExecutable = executablePath.isEmpty()
        ? !fileEntry.addQueueCommand.isEmpty()
        : commandMatches(fileEntry.addQueueCommand, executablePath, "compress-archive");
    entries.append(fileEntry);

    ShellIntegrationExtensionStatus directoryEntry;
    directoryEntry.extension = "文件夹";
    directoryEntry.rootInstalled = registryValueEquals(directoryMenuRootKey(), "MUIVerb", kMenuText)
        && registryValueEquals(directoryMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryMenuClass);

    directoryEntry.addQueueInstalled = registryValueEquals(directoryMenuClassKey() + "\\Shell\\scan_folder", "MUIVerb", "扫描文件夹");

    directoryEntry.addQueueCommand = registryStringValue(directoryMenuClassKey() + "\\Shell\\scan_folder\\command", ".");

    directoryEntry.viewResultsInstalled = registryValueEquals(directoryMenuClassKey() + "\\Shell\\open_main", "MUIVerb", "打开主程序");

    directoryEntry.viewResultsCommand = registryStringValue(directoryMenuClassKey() + "\\Shell\\open_main\\command", ".");

    directoryEntry.commandsPointToExecutable = executablePath.isEmpty()
        ? (!directoryEntry.addQueueCommand.isEmpty() && !directoryEntry.viewResultsCommand.isEmpty())
        : (commandMatches(directoryEntry.addQueueCommand, executablePath, "scan-folder")
            && commandMatches(directoryEntry.viewResultsCommand, executablePath, "open-main"));

    entries.append(directoryEntry);

    ShellIntegrationExtensionStatus directoryBackgroundEntry;
    directoryBackgroundEntry.extension = "文件夹空白处";
    directoryBackgroundEntry.rootInstalled = registryValueEquals(directoryBackgroundMenuRootKey(), "MUIVerb", kMenuText)
        && registryValueEquals(directoryBackgroundMenuRootKey(), "ExtendedSubCommandsKey", kDirectoryBackgroundMenuClass);
    directoryBackgroundEntry.addQueueInstalled = registryValueEquals(directoryBackgroundMenuClassKey() + "\\Shell\\compress_archive", "MUIVerb", "打包压缩包");
    directoryBackgroundEntry.addQueueCommand = registryStringValue(directoryBackgroundMenuClassKey() + "\\Shell\\compress_archive\\command", ".");
    directoryBackgroundEntry.viewResultsInstalled = registryValueEquals(directoryBackgroundMenuClassKey() + "\\Shell\\open_main", "MUIVerb", "打开主程序");
    directoryBackgroundEntry.viewResultsCommand = registryStringValue(directoryBackgroundMenuClassKey() + "\\Shell\\open_main\\command", ".");
    directoryBackgroundEntry.commandsPointToExecutable = executablePath.isEmpty()
        ? (!directoryBackgroundEntry.addQueueCommand.isEmpty() && !directoryBackgroundEntry.viewResultsCommand.isEmpty())
        : (directoryBackgroundEntry.addQueueCommand == compressCommandFor(executablePath, "%V")
            && commandMatches(directoryBackgroundEntry.viewResultsCommand, executablePath, "open-main", "%V"));
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

bool ShellIntegration::installArchiveMenuClass(const QString& executablePath, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kArchiveMenuClass,
            errorMessage,
            "清理压缩包右键菜单定义失败。")) {
        return false;
    }

    const bool ok = writeMenuCommand(archiveMenuClassKey(), "lookup_password", "自动查找密码", commandFor(executablePath, "lookup-password"))
        && writeMenuCommand(archiveMenuClassKey(), "library_test", "使用密码库测试", commandFor(executablePath, "use-password-library-test"))
        && writeMenuCommand(archiveMenuClassKey(), "view_results", "查看结果", commandFor(executablePath, "view-results"))
        && writeMenuCommand(archiveMenuClassKey(), "extract_archive", "解压", commandFor(executablePath, "extract-archive"))
        && writeMenuCommand(archiveMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath))
        && writeMenuCommand(archiveMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main"));

    if (!ok && errorMessage) {
        *errorMessage = "写入压缩包右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installFileMenuClass(const QString& executablePath, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kFileMenuClass,
            errorMessage,
            "清理普通文件右键菜单定义失败。")) {
        return false;
    }

    const bool ok = writeMenuCommand(fileMenuClassKey(), "compress_archive", "打包压缩包", commandFor(executablePath, "compress-archive"));
    if (!ok && errorMessage) {
        *errorMessage = "写入普通文件右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installDirectoryMenuClass(const QString& executablePath, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kDirectoryMenuClass,
            errorMessage,
            "清理文件夹右键菜单定义失败。")) {
        return false;
    }

    const bool ok = writeMenuCommand(directoryMenuClassKey(), "scan_folder", "扫描文件夹", commandFor(executablePath, "scan-folder"))
        && writeMenuCommand(directoryMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath))
        && writeMenuCommand(directoryMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main"));

    if (!ok && errorMessage) {
        *errorMessage = "写入文件夹右键菜单定义失败。";
    }
    return ok;
}

bool ShellIntegration::installDirectoryBackgroundMenuClass(const QString& executablePath, QString* errorMessage) const
{
    if (!removeRegistrySubtree(
            "HKEY_CURRENT_USER\\Software\\Classes",
            kDirectoryBackgroundMenuClass,
            errorMessage,
            "清理文件夹空白处右键菜单定义失败。")) {
        return false;
    }

    const bool ok = writeMenuCommand(directoryBackgroundMenuClassKey(), "compress_archive", "打包压缩包", compressCommandFor(executablePath, "%V"))
        && writeMenuCommand(directoryBackgroundMenuClassKey(), "open_main", "打开主程序", commandFor(executablePath, "open-main", "%V"));

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
