#include "PasswordManager/app/SevenZipProbe.h"

#include <QFileInfo>
#include <QProcess>
#include <QStringList>

namespace PasswordManager {

SevenZipStatus SevenZipProbe::probe(const QString& executablePath) const
{
    SevenZipStatus status;
    status.executablePath = executablePath;
    status.exists = QFileInfo::exists(executablePath);

    if (!status.exists) {
        status.errorMessage = "Bundled 7-Zip does not exist.";
        return status;
    }

    QProcess process;
    process.setProgram(executablePath);
    process.setArguments(QStringList() << "-version");
    process.start();

    if (!process.waitForStarted(3000)) {
        status.errorMessage = "Bundled 7-Zip could not start.";
        return status;
    }

    if (!process.waitForFinished(3000)) {
        process.kill();
        process.waitForFinished(1000);
        status.errorMessage = "Bundled 7-Zip probe timed out.";
        return status;
    }

    status.runnable = process.exitStatus() == QProcess::NormalExit;

    const QString output = QString::fromLocal8Bit(process.readAllStandardOutput())
        + QString::fromLocal8Bit(process.readAllStandardError());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    if (!lines.isEmpty()) {
        status.version = lines.first().trimmed();
    }

    if (!status.runnable) {
        status.errorMessage = "Bundled 7-Zip exited abnormally.";
    } else if (status.version.isEmpty()) {
        status.errorMessage = "Bundled 7-Zip is runnable, but version output was empty.";
    }

    return status;
}

} // namespace PasswordManager
