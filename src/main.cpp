#include "PasswordManager/app/AppLogger.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/app/ExtractService.h"
#include "PasswordManager/app/MainWindow.h"
#include "PasswordManager/app/PerformanceBenchmarkService.h"
#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/app/ShellActionService.h"
#include "PasswordManager/data/ArchivePasswordRepository.h"
#include "PasswordManager/data/ArchiveRepository.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/ExtractLogRepository.h"
#include "PasswordManager/data/PasswordRepository.h"
#include "PasswordManager/data/PasswordTestTaskRepository.h"
#include "PasswordManager/domain/ArchivePasswordRecord.h"
#include "PasswordManager/domain/ArchiveRecord.h"
#include "PasswordManager/domain/PasswordRecord.h"
#include "PasswordManager/ui/UiStyle.h"

#include <QApplication>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QLocalServer>
#include <QLocalSocket>
#include <QMessageBox>
#include <QProcess>
#include <QStringList>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

QString shellActionArgument(const QStringList& arguments)
{
    const int shellActionIndex = arguments.indexOf("--shell-action");
    return shellActionIndex >= 0 && shellActionIndex + 1 < arguments.size() ? arguments.at(shellActionIndex + 1) : QString();
}

QString shellPathArgument(const QStringList& arguments)
{
    const int shellActionIndex = arguments.indexOf("--shell-action");
    return shellActionIndex >= 0 && shellActionIndex + 2 < arguments.size() ? arguments.at(shellActionIndex + 2) : QString();
}

QString singleInstanceServerName(const QString& applicationDir)
{
    const QByteArray hash = QCryptographicHash::hash(applicationDir.toUtf8(), QCryptographicHash::Sha256).toHex().left(16);
    return "PasswordManager-" + QString::fromLatin1(hash);
}

bool shouldForwardToExistingInstance(const QString& shellAction)
{
    return shellAction.isEmpty()
        || shellAction == "open-main"
        || shellAction == "use-password-library-test"
        || shellAction == "add-test-queue"
        || shellAction == "scan-folder";
}

bool sendToExistingInstance(const QString& serverName, const QString& shellAction, const QString& shellPath)
{
    QLocalSocket socket;
    socket.connectToServer(serverName, QIODevice::WriteOnly);
    if (!socket.waitForConnected(250)) {
        return false;
    }

    const QString action = shellAction.isEmpty() ? "open-main" : shellAction;
    socket.write((action + "\n" + shellPath).toUtf8());
    socket.flush();
    socket.waitForBytesWritten(500);
    socket.disconnectFromServer();
    return true;
}

void showShellMessage(QMessageBox::Icon icon, const QString& title, const QString& message)
{
    QMessageBox box(icon, title, message, QMessageBox::Ok);
    box.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    box.raise();
    box.activateWindow();
    box.exec();
}

QString formatKnownPasswords(const QList<PasswordManager::ArchivePasswordRecord>& records, const QString& archivePath)
{
    if (records.isEmpty()) {
        return "密码库无该压缩包的已知密码。\n\n" + archivePath;
    }

    QStringList lines;
    const int visibleCount = qMin(records.size(), 5);
    lines << QString("找到 %1 条已知密码记录。").arg(records.size());
    lines << QString("下方只显示前 %1 条，完整记录请打开主程序查看。").arg(visibleCount);
    lines << "";
    for (int i = 0; i < visibleCount; ++i) {
        const auto& record = records.at(i);
        lines << QString("%1. 密码：%2").arg(i + 1).arg(record.password);
        lines << QString("   成功次数：%1").arg(record.successCount);
        if (record.lastSuccessAt.isValid()) {
            lines << "   最近成功：" + record.lastSuccessAt.toLocalTime().toString("yyyy-MM-dd HH:mm:ss");
        }
    }
    lines << "" << archivePath;
    return lines.join("\n");
}

QString outputDirectoryForArchive(const QString& archivePath)
{
    const QFileInfo archiveInfo(archivePath);
    QString folderName = archiveInfo.completeBaseName();
    if (folderName.isEmpty()) {
        folderName = archiveInfo.baseName();
    }
    return QDir(archiveInfo.absolutePath()).filePath(folderName);
}

QString extractResultMessage(const PasswordManager::ExtractResult& result)
{
    if (!result.errorMessage.isEmpty()) {
        return result.errorMessage;
    }
    return PasswordManager::extractStatusText(result.status);
}

void recordExtractLog(
    const PasswordManager::ExtractLogRepository& extractLogRepository,
    const PasswordManager::AppLogger& logger,
    int archiveId,
    const QString& archivePath,
    const QString& outputDirectory,
    const PasswordManager::ExtractResult& result)
{
    QString error;
    if (!extractLogRepository.add(archiveId, archivePath, outputDirectory, result.status, extractResultMessage(result), &error)) {
        logger.error("Extract log write failed: " + error);
    }
    logger.extract(QString("Extract completed from shell: archive_id=%1 status=%2 output=%3 path=%4")
            .arg(archiveId)
            .arg(PasswordManager::extractStatusText(result.status))
            .arg(outputDirectory)
            .arg(archivePath));
}

int recordSuccessfulPassword(
    const PasswordManager::PasswordRepository& passwordRepository,
    const PasswordManager::AppLogger& logger,
    const QString& password)
{
    if (password.isEmpty()) {
        return 0;
    }

    const PasswordManager::PasswordRecord existing = passwordRepository.findByPassword(password);
    if (existing.id > 0) {
        QString statsError;
        if (!passwordRepository.incrementStats(existing.id, true, &statsError)) {
            logger.error("Password stats update failed: " + statsError);
        }
        return existing.id;
    }

    PasswordManager::PasswordRecord learnedPassword;
    learnedPassword.password = password;
    learnedPassword.category = "右键解压";
    learnedPassword.note = "右键解压成功后自动添加";
    learnedPassword.successCount = 1;

    QString addError;
    if (!passwordRepository.add(learnedPassword, &addError)) {
        logger.error("Password auto add failed: " + addError);
        return 0;
    }

    return passwordRepository.findByPassword(password).id;
}

bool recordSuccessfulArchivePassword(
    const PasswordManager::ArchivePasswordRepository& archivePasswordRepository,
    const PasswordManager::PasswordRepository& passwordRepository,
    const PasswordManager::AppLogger& logger,
    int archiveId,
    const QString& password)
{
    if (password.isEmpty()) {
        return true;
    }

    const int passwordId = recordSuccessfulPassword(passwordRepository, logger, password);
    QString error;
    if (!archivePasswordRepository.recordSuccess(archiveId, passwordId, password, &error)) {
        logger.error("Archive password association failed: " + error);
        return false;
    }
    return true;
}

QString askPassword(const QString& archivePath)
{
    QInputDialog dialog;
    dialog.setWindowTitle("PasswordManager");
    dialog.setLabelText("请输入压缩包密码：\n" + archivePath);
    dialog.setTextEchoMode(QLineEdit::Password);
    dialog.setWindowFlag(Qt::WindowStaysOnTopHint, true);
    dialog.resize(520, dialog.height());
    if (dialog.exec() != QDialog::Accepted) {
        return QString();
    }
    return dialog.textValue();
}

int handleKnownPasswordPopup(
    const QString& action,
    const QString& archivePath,
    const PasswordManager::AppPaths& paths,
    const PasswordManager::ArchiveRepository& archiveRepository,
    const PasswordManager::ArchivePasswordRepository& archivePasswordRepository,
    const PasswordManager::PasswordRepository& passwordRepository,
    PasswordManager::PasswordTestTaskManager& taskManager)
{
    const auto result = PasswordManager::ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).lookupKnownPasswords(archivePath);

    if (!result.success) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", result.message + "\n" + archivePath);
        return 1;
    }

    if (result.noPasswordArchive) {
        const QString title = action == "lookup-password" ? "自动查找密码" : "查看结果";
        showShellMessage(QMessageBox::Information, title, "该压缩包无需密码。\n\n" + archivePath);
        return 0;
    }

    const QList<PasswordManager::ArchivePasswordRecord> records = archivePasswordRepository.list(QFileInfo(archivePath).absoluteFilePath());
    const QString title = action == "lookup-password" ? "自动查找密码" : "查看结果";
    showShellMessage(QMessageBox::Information, title, formatKnownPasswords(records, archivePath));
    return 0;
}

int handleCompressArchive(const QString& path, const PasswordManager::AppPaths& paths)
{
    if (!QFileInfo::exists(paths.sevenZipFileManagerExecutable())) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", "内置 7zFM.exe 不存在，无法打开 7-Zip File Manager。\n" + paths.sevenZipFileManagerExecutable());
        return 1;
    }

    QString languageError;
    if (!paths.ensureSevenZipChineseLanguage(&languageError)) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", languageError);
    }

    const QFileInfo targetInfo(path);
    const QString openPath = targetInfo.isFile() ? targetInfo.absolutePath() : targetInfo.absoluteFilePath();
    if (!QProcess::startDetached(paths.sevenZipFileManagerExecutable(), {openPath})) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", "无法启动 7-Zip File Manager。\n" + paths.sevenZipFileManagerExecutable());
        return 1;
    }
    return 0;
}

int handleExtractArchive(
    const QString& archivePath,
    const PasswordManager::AppPaths& paths,
    const PasswordManager::AppLogger& logger,
    const PasswordManager::ArchiveRepository& archiveRepository,
    const PasswordManager::ArchivePasswordRepository& archivePasswordRepository,
    const PasswordManager::ExtractLogRepository& extractLogRepository,
    const PasswordManager::PasswordRepository& passwordRepository,
    PasswordManager::PasswordTestTaskManager& taskManager)
{
    const auto lookupResult = PasswordManager::ShellActionService(
        paths,
        archiveRepository,
        archivePasswordRepository,
        passwordRepository,
        taskManager).lookupKnownPasswords(archivePath);
    if (!lookupResult.success) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", lookupResult.message + "\n" + archivePath);
        return 1;
    }

    const QString outputDirectory = outputDirectoryForArchive(archivePath);
    const PasswordManager::ExtractService extractService(paths.sevenZipExecutable());
    const auto noPasswordTest = taskManager.testPasswordNow(archivePath, QString());
    if (noPasswordTest.status == PasswordManager::SevenZipTestStatus::Success) {
        const PasswordManager::ExtractResult result = extractService.extract(archivePath, QString(), outputDirectory, 600000);
        recordExtractLog(extractLogRepository, logger, lookupResult.archiveId, archivePath, outputDirectory, result);
        if (result.status != PasswordManager::ExtractStatus::Success) {
            showShellMessage(QMessageBox::Warning, "PasswordManager", "解压失败：" + extractResultMessage(result) + "\n\n" + archivePath);
            return 1;
        }
        showShellMessage(QMessageBox::Information, "PasswordManager", "解压完成。\n\n输出目录：\n" + outputDirectory);
        return 0;
    }

    if (noPasswordTest.status != PasswordManager::SevenZipTestStatus::WrongPassword) {
        showShellMessage(QMessageBox::Warning, "PasswordManager", "压缩包测试失败：" + noPasswordTest.errorMessage + "\n\n" + archivePath);
        return 1;
    }

    const QList<PasswordManager::ArchivePasswordRecord> knownPasswords = archivePasswordRepository.list(QFileInfo(archivePath).absoluteFilePath());
    for (const auto& known : knownPasswords) {
        if (known.password.isEmpty()) {
            continue;
        }
        const auto passwordTest = taskManager.testPasswordNow(archivePath, known.password);
        if (passwordTest.status != PasswordManager::SevenZipTestStatus::Success) {
            continue;
        }

        const PasswordManager::ExtractResult result = extractService.extract(archivePath, known.password, outputDirectory, 600000);
        recordExtractLog(extractLogRepository, logger, lookupResult.archiveId, archivePath, outputDirectory, result);
        if (result.status == PasswordManager::ExtractStatus::Success) {
            recordSuccessfulArchivePassword(archivePasswordRepository, passwordRepository, logger, lookupResult.archiveId, known.password);
            showShellMessage(
                QMessageBox::Information,
                "PasswordManager",
                "解压完成，已使用密码库中的已知密码。\n\n密码：" + known.password + "\n输出目录：\n" + outputDirectory);
            return 0;
        }
        showShellMessage(QMessageBox::Warning, "PasswordManager", "解压失败：" + extractResultMessage(result) + "\n\n" + archivePath);
        return 1;
    }

    while (true) {
        const QString password = askPassword(archivePath);
        if (password.isNull() || password.isEmpty()) {
            showShellMessage(QMessageBox::Information, "PasswordManager", "已取消解压。\n" + archivePath);
            return 1;
        }

        const auto passwordTest = taskManager.testPasswordNow(archivePath, password);
        if (passwordTest.status == PasswordManager::SevenZipTestStatus::WrongPassword) {
            const auto retry = QMessageBox::question(
                nullptr,
                "PasswordManager",
                "密码错误，是否重新输入？\n" + archivePath,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes);
            if (retry != QMessageBox::Yes) {
                return 1;
            }
            continue;
        }
        if (passwordTest.status != PasswordManager::SevenZipTestStatus::Success) {
            showShellMessage(QMessageBox::Warning, "PasswordManager", "密码测试失败：" + passwordTest.errorMessage + "\n\n" + archivePath);
            return 1;
        }

        const PasswordManager::ExtractResult result = extractService.extract(archivePath, password, outputDirectory, 600000);
        recordExtractLog(extractLogRepository, logger, lookupResult.archiveId, archivePath, outputDirectory, result);
        if (result.status == PasswordManager::ExtractStatus::Success) {
            recordSuccessfulArchivePassword(archivePasswordRepository, passwordRepository, logger, lookupResult.archiveId, password);
            showShellMessage(
                QMessageBox::Information,
                "PasswordManager",
                "解压完成，密码已记录。\n\n密码：" + password + "\n输出目录：\n" + outputDirectory);
            return 0;
        }

        if (result.status != PasswordManager::ExtractStatus::WrongPassword) {
            showShellMessage(QMessageBox::Warning, "PasswordManager", "解压失败：" + extractResultMessage(result) + "\n\n" + archivePath);
            return 1;
        }

        const auto retry = QMessageBox::question(
            nullptr,
            "PasswordManager",
            "密码错误，是否重新输入？\n" + archivePath,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes);
        if (retry != QMessageBox::Yes) {
            return 1;
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    bool smokeTestRequested = false;
    bool benchmarkRequested = false;
#ifdef Q_OS_WIN
    const QString commandLine = QString::fromWCharArray(GetCommandLineW());
    smokeTestRequested = commandLine.contains("--smoke-test");
    benchmarkRequested = commandLine.contains("--benchmark");
#else
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--smoke-test") {
            smokeTestRequested = true;
        }
        if (QString::fromLocal8Bit(argv[i]) == "--benchmark") {
            benchmarkRequested = true;
        }
    }
#endif

    if (smokeTestRequested || benchmarkRequested) {
        QCoreApplication app(argc, argv);
        app.setApplicationName("PasswordManager");
        app.setApplicationVersion("0.1.0");
        app.setOrganizationName("PasswordManager");

        const PasswordManager::AppPaths paths(QCoreApplication::applicationDirPath());
        if (!paths.ensureRuntimeDirectories()) {
            return 1;
        }

        const PasswordManager::AppLogger logger(paths.logsDir());
        logger.info("Smoke test started.");

        PasswordManager::DatabaseService database(paths);
        if (!database.open()) {
            logger.database("Database open failed: " + database.lastError());
            logger.error("Smoke test failed: database open failed. " + database.lastError());
            return 1;
        }
        logger.database("Database opened: " + database.databasePath());

        if (!QFileInfo::exists(paths.sevenZipExecutable())) {
            logger.error("Smoke test failed: bundled 7-Zip was not found.");
            return 2;
        }

        if (benchmarkRequested) {
            QString benchmarkDirectory;
            const QStringList arguments = QCoreApplication::arguments();
            const int benchmarkIndex = arguments.indexOf("--benchmark");
            if (benchmarkIndex >= 0 && benchmarkIndex + 1 < arguments.size() && !arguments.at(benchmarkIndex + 1).startsWith("--")) {
                benchmarkDirectory = arguments.at(benchmarkIndex + 1);
            }

            QString outputPath;
            QString error;
            if (!PasswordManager::PerformanceBenchmarkService(paths, database.connectionName()).run(benchmarkDirectory, &outputPath, &error)) {
                logger.error("Benchmark failed: " + error);
                return 3;
            }
            logger.info("Benchmark report created: " + outputPath);
            return 0;
        }

        logger.info("Smoke test passed.");
        return 0;
    }

    QApplication app(argc, argv);
    app.setApplicationName("PasswordManager");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("PasswordManager");
    PasswordManager::UiStyle::applyApplicationFont(app);

    const PasswordManager::AppPaths paths(QCoreApplication::applicationDirPath());
    if (!paths.ensureRuntimeDirectories()) {
        showShellMessage(QMessageBox::Critical, "PasswordManager", "无法创建运行目录，请检查程序目录权限。");
        return 1;
    }
    QString sevenZipLanguageError;
    paths.ensureSevenZipChineseLanguage(&sevenZipLanguageError);

    const PasswordManager::AppLogger logger(paths.logsDir());
    logger.info("Application started.");
    if (!sevenZipLanguageError.isEmpty()) {
        logger.error("7-Zip language setup failed: " + sevenZipLanguageError);
    }

    const QStringList arguments = QCoreApplication::arguments();
    const QString shellAction = shellActionArgument(arguments);
    const QString shellPath = shellPathArgument(arguments);
    const QString instanceServerName = singleInstanceServerName(paths.applicationDir());
    if (shouldForwardToExistingInstance(shellAction) && sendToExistingInstance(instanceServerName, shellAction, shellPath)) {
        return 0;
    }

    PasswordManager::DatabaseService database(paths);
    if (!database.open()) {
        logger.database("Database open failed: " + database.lastError());
        logger.error("Database open failed: " + database.lastError());
        showShellMessage(QMessageBox::Critical, "PasswordManager", "数据库打开失败：" + database.lastError());
        return 1;
    }
    logger.database("Database opened: " + database.databasePath());

    PasswordManager::ArchiveRepository archiveRepository(database.connectionName());
    PasswordManager::ArchivePasswordRepository archivePasswordRepository(database.connectionName());
    PasswordManager::ExtractLogRepository extractLogRepository(database.connectionName());
    PasswordManager::PasswordRepository passwordRepository(database.connectionName());
    PasswordManager::PasswordTestTaskRepository passwordTestTaskRepository(database.connectionName());
    PasswordManager::PasswordTestTaskManager taskManager(paths.sevenZipExecutable(), &passwordTestTaskRepository, &logger);
    QObject::connect(&taskManager, &PasswordManager::PasswordTestTaskManager::taskFinished, &app, [&](const PasswordManager::PasswordTestTask& task) {
        int passwordId = task.passwordId;

        const bool passwordReallySucceeded = task.status == PasswordManager::PasswordTestTaskStatus::Completed
            && task.testStatus == PasswordManager::SevenZipTestStatus::Success
            && !task.password.isEmpty();

        if (passwordId <= 0 && passwordReallySucceeded) {
            const PasswordManager::PasswordRecord existingPassword = passwordRepository.findByPassword(task.password);
            if (existingPassword.id > 0) {
                passwordId = existingPassword.id;
                QString statsError;
                passwordRepository.incrementStats(passwordId, true, &statsError);
                if (!statsError.isEmpty()) {
                    logger.error("Password stats update failed: " + statsError);
                }
            } else {
                PasswordManager::PasswordRecord learnedPassword;
                learnedPassword.password = task.password;
                learnedPassword.category = "自动学习";
                learnedPassword.note = "密码测试成功后自动添加";
                learnedPassword.successCount = 1;

                QString addError;
                if (!passwordRepository.add(learnedPassword, &addError)) {
                    logger.error("Password auto add failed: " + addError);
                } else {
                    const PasswordManager::PasswordRecord addedPassword = passwordRepository.findByPassword(task.password);
                    passwordId = addedPassword.id;
                }
            }
        } else if (passwordId > 0) {
            QString statsError;
            passwordRepository.incrementStats(passwordId, passwordReallySucceeded, &statsError);
            if (!statsError.isEmpty()) {
                logger.error("Password stats update failed: " + statsError);
            }
        }

        if (task.archiveId > 0 && passwordReallySucceeded) {
            QString associationError;
            archivePasswordRepository.recordSuccess(task.archiveId, passwordId, task.password, &associationError);
            if (!associationError.isEmpty()) {
                logger.error("Archive password association failed: " + associationError);
            }
        }
    });

    if (!shellAction.isEmpty()) {
        logger.info("Shell action requested: " + shellAction + " path=" + shellPath);

        if (shellAction == "lookup-password" || shellAction == "view-results") {
            const int result = handleKnownPasswordPopup(
                shellAction,
                shellPath,
                paths,
                archiveRepository,
                archivePasswordRepository,
                passwordRepository,
                taskManager);
            logger.info("Shell action completed: " + shellAction + " path=" + shellPath);
            return result;
        }

        if (shellAction == "compress-archive") {
            const int result = handleCompressArchive(shellPath, paths);
            logger.info("Shell action completed: " + shellAction + " path=" + shellPath);
            return result;
        }

        if (shellAction == "extract-archive") {
            const int result = handleExtractArchive(
                shellPath,
                paths,
                logger,
                archiveRepository,
                archivePasswordRepository,
                extractLogRepository,
                passwordRepository,
                taskManager);
            logger.info("Shell action completed: " + shellAction + " path=" + shellPath);
            return result;
        }
    }

    PasswordManager::MainWindow window(
        paths,
        archiveRepository,
        archivePasswordRepository,
        extractLogRepository,
        passwordRepository,
        database.connectionName(),
        taskManager);
    window.show();

    QLocalServer instanceServer;
    QLocalServer::removeServer(instanceServerName);
    if (instanceServer.listen(instanceServerName)) {
        QObject::connect(&instanceServer, &QLocalServer::newConnection, &window, [&instanceServer, &window, &logger]() {
            while (QLocalSocket* socket = instanceServer.nextPendingConnection()) {
                QObject::connect(socket, &QLocalSocket::readyRead, &window, [socket, &window, &logger]() {
                    const QString payload = QString::fromUtf8(socket->readAll());
                    const int separator = payload.indexOf('\n');
                    const QString action = separator >= 0 ? payload.left(separator) : QString("open-main");
                    const QString path = separator >= 0 ? payload.mid(separator + 1) : QString();
                    logger.info("Single instance action received: " + action + " path=" + path);
                    window.showNormal();
                    window.raise();
                    window.activateWindow();
                    window.openShellAction(action, path);
                });
                QObject::connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
            }
        });
    } else {
        logger.error("Single instance server listen failed: " + instanceServer.errorString());
    }

    if (!shellAction.isEmpty()) {
        QTimer::singleShot(0, &window, [&window, &logger, shellAction, shellPath]() {
            logger.info("Shell action executing in main window: " + shellAction + " path=" + shellPath);
            window.showNormal();
            window.raise();
            window.activateWindow();
            window.openShellAction(shellAction, shellPath);
            logger.info("Shell action completed in main window: " + shellAction + " path=" + shellPath);
        });
    }

    const int result = app.exec();
    logger.info("Application stopped.");
    return result;
}
