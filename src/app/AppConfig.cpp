#include "PasswordManager/app/AppConfig.h"

#include "PasswordManager/app/AppPaths.h"

#include <QDir>
#include <QSettings>
#include <algorithm>

namespace PasswordManager {

namespace {

int boundedInt(int value, int minValue, int maxValue)
{
    return std::max(minValue, std::min(value, maxValue));
}

} // namespace

AppConfig::AppConfig(const AppPaths& paths)
    : m_paths(paths)
{
}

SmartMatchSettings AppConfig::smartMatchSettings() const
{
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.beginGroup("SmartMatch");

    SmartMatchSettings result;
    result.enableExactHistory = settings.value("EnableExactHistory", result.enableExactHistory).toBool();
    result.enableDirectoryHistory = settings.value("EnableDirectoryHistory", result.enableDirectoryHistory).toBool();
    result.enableCategoryCandidates = settings.value("EnableCategoryCandidates", result.enableCategoryCandidates).toBool();
    result.enablePasswordLibrary = settings.value("EnablePasswordLibrary", result.enablePasswordLibrary).toBool();
    result.enableDescriptionFiles = settings.value("EnableDescriptionFiles", result.enableDescriptionFiles).toBool();
    result.calculateFullHashDuringScan = settings.value("CalculateFullHashDuringScan", result.calculateFullHashDuringScan).toBool();
    result.maxCandidates = boundedInt(settings.value("MaxCandidates", result.maxCandidates).toInt(), 1, 500);
    result.maxDescriptionCandidates = boundedInt(settings.value("MaxDescriptionCandidates", result.maxDescriptionCandidates).toInt(), 0, 100);
    result.maxDescriptionFileBytes = boundedInt(settings.value("MaxDescriptionFileBytes", result.maxDescriptionFileBytes).toInt(), 1024, 1024 * 1024);

    settings.endGroup();
    return result;
}

void AppConfig::saveSmartMatchSettings(const SmartMatchSettings& settingsValue) const
{
    QDir().mkpath(m_paths.configDir());
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.beginGroup("SmartMatch");
    settings.setValue("EnableExactHistory", settingsValue.enableExactHistory);
    settings.setValue("EnableDirectoryHistory", settingsValue.enableDirectoryHistory);
    settings.setValue("EnableCategoryCandidates", settingsValue.enableCategoryCandidates);
    settings.setValue("EnablePasswordLibrary", settingsValue.enablePasswordLibrary);
    settings.setValue("EnableDescriptionFiles", settingsValue.enableDescriptionFiles);
    settings.setValue("CalculateFullHashDuringScan", settingsValue.calculateFullHashDuringScan);
    settings.setValue("MaxCandidates", boundedInt(settingsValue.maxCandidates, 1, 500));
    settings.setValue("MaxDescriptionCandidates", boundedInt(settingsValue.maxDescriptionCandidates, 0, 100));
    settings.setValue("MaxDescriptionFileBytes", boundedInt(settingsValue.maxDescriptionFileBytes, 1024, 1024 * 1024));
    settings.endGroup();
    settings.sync();
}

ShellMenuSettings AppConfig::shellMenuSettings() const
{
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.beginGroup("ShellMenu");

    ShellMenuSettings result;
    result.enableArchiveLookup = settings.value("EnableArchiveLookup", result.enableArchiveLookup).toBool();
    result.enableArchiveTest = settings.value("EnableArchiveTest", result.enableArchiveTest).toBool();
    result.enableArchiveViewResults = settings.value("EnableArchiveViewResults", result.enableArchiveViewResults).toBool();
    result.enableArchiveExtract = settings.value("EnableArchiveExtract", result.enableArchiveExtract).toBool();
    result.enableArchiveCompress = settings.value("EnableArchiveCompress", result.enableArchiveCompress).toBool();
    result.enableArchiveOpenMain = settings.value("EnableArchiveOpenMain", result.enableArchiveOpenMain).toBool();
    result.enableFolderScan = settings.value("EnableFolderScan", result.enableFolderScan).toBool();
    result.enableFolderCompress = settings.value("EnableFolderCompress", result.enableFolderCompress).toBool();
    result.enableFolderOpenMain = settings.value("EnableFolderOpenMain", result.enableFolderOpenMain).toBool();
    result.enableFileCompress = settings.value("EnableFileCompress", result.enableFileCompress).toBool();

    settings.endGroup();
    return result;
}

void AppConfig::saveShellMenuSettings(const ShellMenuSettings& settingsValue) const
{
    QDir().mkpath(m_paths.configDir());
    QSettings settings(settingsPath(), QSettings::IniFormat);
    settings.beginGroup("ShellMenu");
    settings.setValue("EnableArchiveLookup", settingsValue.enableArchiveLookup);
    settings.setValue("EnableArchiveTest", settingsValue.enableArchiveTest);
    settings.setValue("EnableArchiveViewResults", settingsValue.enableArchiveViewResults);
    settings.setValue("EnableArchiveExtract", settingsValue.enableArchiveExtract);
    settings.setValue("EnableArchiveCompress", settingsValue.enableArchiveCompress);
    settings.setValue("EnableArchiveOpenMain", settingsValue.enableArchiveOpenMain);
    settings.setValue("EnableFolderScan", settingsValue.enableFolderScan);
    settings.setValue("EnableFolderCompress", settingsValue.enableFolderCompress);
    settings.setValue("EnableFolderOpenMain", settingsValue.enableFolderOpenMain);
    settings.setValue("EnableFileCompress", settingsValue.enableFileCompress);
    settings.endGroup();
    settings.sync();
}

QString AppConfig::settingsPath() const
{
    return QDir(m_paths.configDir()).filePath("settings.ini");
}

} // namespace PasswordManager
