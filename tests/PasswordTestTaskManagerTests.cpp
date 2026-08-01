#include "PasswordManager/app/PasswordTestTaskManager.h"
#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordTestTaskRepository.h"

#include <QCoreApplication>
#include <QDir>
#include <QSignalSpy>
#include <QTest>

using PasswordManager::PasswordTestTaskManager;
using PasswordManager::PasswordTestTaskStatus;
using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordTestTask;
using PasswordManager::PasswordTestTaskRepository;

class PasswordTestTaskManagerTests final : public QObject {
    Q_OBJECT

private slots:
    void completesCorrectPassword();
    void completesNoPasswordArchive();
    void doesNotAcceptPasswordForNoPasswordArchive();
    void failsWrongPassword();
    void cancelsWaitingTask();
    void persistsFailedTaskAndLoadsIt();
    void loadsWaitingTasksAndRunsThemOnStartup();
    void marksInterruptedRunningTasksFailedOnStartup();
    void retryCreatesNewTaskFromFinishedTask();
    void removesFinishedTasks();

private:
    QString projectPath(const QString& relativePath) const;
};

QString PasswordTestTaskManagerTests::projectPath(const QString& relativePath) const
{
    const QDir root(QCoreApplication::applicationDirPath() + "/..");
    return root.absoluteFilePath(relativePath);
}

void PasswordTestTaskManagerTests::completesCorrectPassword()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    QSignalSpy spy(&manager, &PasswordTestTaskManager::tasksChanged);
    manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "pm-fixture-01");

    QTRY_VERIFY_WITH_TIMEOUT(!manager.tasks().isEmpty() && manager.tasks().first().status == PasswordTestTaskStatus::Completed, 5000);
    QVERIFY(spy.count() > 0);
}

void PasswordTestTaskManagerTests::completesNoPasswordArchive()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_21_no_password.zip"), "");

    QTRY_VERIFY_WITH_TIMEOUT(!manager.tasks().isEmpty() && manager.tasks().first().status == PasswordTestTaskStatus::Completed, 5000);
}

void PasswordTestTaskManagerTests::doesNotAcceptPasswordForNoPasswordArchive()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_21_no_password.zip"), "any-password");

    QVERIFY(!manager.tasks().isEmpty());
    QCOMPARE(manager.tasks().first().status, PasswordTestTaskStatus::Completed);
    QCOMPARE(manager.tasks().first().testStatus, PasswordManager::SevenZipTestStatus::NoPasswordRequired);
    QVERIFY(manager.tasks().first().password.isEmpty());
}

void PasswordTestTaskManagerTests::failsWrongPassword()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "wrong-password");

    QTRY_VERIFY_WITH_TIMEOUT(!manager.tasks().isEmpty() && manager.tasks().first().status == PasswordTestTaskStatus::Failed, 5000);
}

void PasswordTestTaskManagerTests::cancelsWaitingTask()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_20_password_pm-fixture-20.7z"), "pm-fixture-20");
    const int secondId = manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_19_password_pm-fixture-19.zip"), "pm-fixture-19");
    manager.cancelTask(secondId);

    const auto tasks = manager.tasks();
    QCOMPARE(tasks.size(), 2);
    QCOMPARE(tasks.at(1).status, PasswordTestTaskStatus::Cancelled);
}

void PasswordTestTaskManagerTests::persistsFailedTaskAndLoadsIt()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    {
        PasswordTestTaskManager manager(dir.filePath("missing-7z.exe"), &repository);
        manager.enqueuePasswordTest("C:/tmp/archive.zip", "secret");
        QCOMPARE(manager.tasks().size(), 1);
        QCOMPARE(manager.tasks().first().status, PasswordTestTaskStatus::Failed);
    }

    PasswordTestTaskManager loaded(dir.filePath("missing-7z.exe"), &repository);
    QCOMPARE(loaded.tasks().size(), 1);
    QCOMPARE(loaded.tasks().first().status, PasswordTestTaskStatus::Failed);
    QCOMPARE(loaded.tasks().first().archivePath, QString("C:/tmp/archive.zip"));
    QCOMPARE(loaded.tasks().first().password, QString("secret"));
}

void PasswordTestTaskManagerTests::loadsWaitingTasksAndRunsThemOnStartup()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    PasswordTestTask task;
    task.archivePath = projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip");
    task.password = "pm-fixture-01";
    task.status = PasswordTestTaskStatus::Waiting;
    QString addError;
    QVERIFY2(repository.add(task, &addError) > 0, qPrintable(addError));

    PasswordTestTaskManager loaded(projectPath("tools/7zip/7z.exe"), &repository);
    QCOMPARE(loaded.tasks().size(), 1);
    QTRY_COMPARE_WITH_TIMEOUT(loaded.tasks().first().status, PasswordTestTaskStatus::Completed, 5000);

    const auto persistedTasks = repository.list();
    QCOMPARE(persistedTasks.size(), 1);
    QCOMPARE(persistedTasks.first().status, PasswordTestTaskStatus::Completed);
}

void PasswordTestTaskManagerTests::marksInterruptedRunningTasksFailedOnStartup()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    PasswordTestTask task;
    task.archivePath = "C:/tmp/archive.zip";
    task.password = "secret";
    task.status = PasswordTestTaskStatus::Running;
    QString addError;
    QVERIFY2(repository.add(task, &addError) > 0, qPrintable(addError));

    PasswordTestTaskManager loaded(dir.filePath("missing-7z.exe"), &repository);
    QCOMPARE(loaded.tasks().size(), 1);
    QCOMPARE(loaded.tasks().first().status, PasswordTestTaskStatus::Failed);
    QCOMPARE(loaded.tasks().first().testStatus, PasswordManager::SevenZipTestStatus::ProcessError);
    QCOMPARE(loaded.tasks().first().message, QString("Interrupted before application restart."));
}

void PasswordTestTaskManagerTests::retryCreatesNewTaskFromFinishedTask()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    const int failedId = manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "wrong-password");

    QTRY_VERIFY_WITH_TIMEOUT(!manager.tasks().isEmpty() && manager.tasks().first().status == PasswordTestTaskStatus::Failed, 5000);

    const int retryId = manager.retryTask(failedId);
    QVERIFY(retryId > failedId);
    QCOMPARE(manager.tasks().size(), 2);
    QCOMPARE(manager.retryTask(retryId), 0);

    QTRY_VERIFY_WITH_TIMEOUT(manager.tasks().at(1).status == PasswordTestTaskStatus::Failed, 5000);
}

void PasswordTestTaskManagerTests::removesFinishedTasks()
{
    PasswordTestTaskManager manager(projectPath("tools/7zip/7z.exe"));
    const int failedId = manager.enqueuePasswordTest(projectPath("testdata/archives/fixture_01_password_pm-fixture-01.zip"), "wrong-password");

    QTRY_VERIFY_WITH_TIMEOUT(!manager.tasks().isEmpty() && manager.tasks().first().status == PasswordTestTaskStatus::Failed, 5000);

    QVERIFY(manager.removeFinishedTask(failedId));
    QCOMPARE(manager.tasks().size(), 0);
    QCOMPARE(manager.clearFinishedTasks(), 0);
}

QTEST_MAIN(PasswordTestTaskManagerTests)
#include "PasswordTestTaskManagerTests.moc"
