#include "PasswordManager/app/AppPaths.h"

#include <QDir>
#include <QFileInfo>
#include <QSettings>

#include <utility>

namespace PasswordManager {

AppPaths::AppPaths(QString applicationDir)
    : m_applicationDir(std::move(applicationDir))
{
}

const QString& AppPaths::applicationDir() const
{
    return m_applicationDir;
}

QString AppPaths::dataDir() const
{
    return QDir(m_applicationDir).filePath("data");
}

QString AppPaths::configDir() const
{
    return QDir(m_applicationDir).filePath("config");
}

QString AppPaths::logsDir() const
{
    return QDir(m_applicationDir).filePath("logs");
}

QString AppPaths::backupDir() const
{
    return QDir(m_applicationDir).filePath("backup");
}

QString AppPaths::toolsDir() const
{
    return QDir(m_applicationDir).filePath("tools");
}

QString AppPaths::sevenZipDir() const
{
    return QDir(m_applicationDir).filePath("tools/7zip");
}

QString AppPaths::sevenZipExecutable() const
{
    return QDir(sevenZipDir()).filePath("7z.exe");
}

QString AppPaths::sevenZipGuiExecutable() const
{
    return QDir(sevenZipDir()).filePath("7zG.exe");
}

QString AppPaths::sevenZipFileManagerExecutable() const
{
    return QDir(sevenZipDir()).filePath("7zFM.exe");
}

QString AppPaths::sevenZipChineseLanguageFile() const
{
    return QDir(sevenZipDir()).filePath("Lang/zh-cn.txt");
}

bool AppPaths::ensureRuntimeDirectories() const
{
    QDir dir(m_applicationDir);
    return dir.mkpath("data")
        && dir.mkpath("config")
        && dir.mkpath("logs")
        && dir.mkpath("backup")
        && dir.mkpath("tools/7zip");
}

bool AppPaths::ensureSevenZipChineseLanguage(QString* errorMessage) const
{
    if (!QFileInfo::exists(sevenZipChineseLanguageFile())) {
        if (errorMessage) {
            *errorMessage = "内置 7-Zip 中文语言文件不存在：" + sevenZipChineseLanguageFile();
        }
        return false;
    }

    QSettings settings("HKEY_CURRENT_USER\\Software\\7-Zip", QSettings::NativeFormat);
    settings.setValue("Lang", "zh-cn");
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        if (errorMessage) {
            *errorMessage = "写入 7-Zip 中文语言设置失败。";
        }
        return false;
    }
    return true;
}

} // namespace PasswordManager
