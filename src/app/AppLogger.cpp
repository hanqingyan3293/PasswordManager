#include "PasswordManager/app/AppLogger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include <utility>

namespace PasswordManager {

AppLogger::AppLogger(QString logDirectory)
    : m_logDirectory(std::move(logDirectory))
{
}

void AppLogger::info(const QString& message) const
{
    writeLine("app.log", "INFO", message);
}

void AppLogger::error(const QString& message) const
{
    writeLine("error.log", "ERROR", message);
}

void AppLogger::archive(const QString& message) const
{
    writeLine("archive.log", "INFO", message);
}

void AppLogger::extract(const QString& message) const
{
    writeLine("extract.log", "INFO", message);
}

void AppLogger::database(const QString& message) const
{
    writeLine("database.log", "INFO", message);
}

void AppLogger::writeLine(const QString& fileName, const QString& level, const QString& message) const
{
    QDir().mkpath(m_logDirectory);

    QFile file(QDir(m_logDirectory).filePath(fileName));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    stream << QDateTime::currentDateTime().toString(Qt::ISODate)
           << " [" << level << "] "
           << message << '\n';
}

} // namespace PasswordManager
