#include "PasswordManager/app/AppPaths.h"
#include "PasswordManager/data/DatabaseService.h"
#include "PasswordManager/data/PasswordTestTaskRepository.h"

#include <QTemporaryDir>
#include <QTest>

using PasswordManager::AppPaths;
using PasswordManager::DatabaseService;
using PasswordManager::PasswordTestTask;
using PasswordManager::PasswordTestTaskRepository;
using PasswordManager::PasswordTestTaskStatus;
using PasswordManager::SevenZipTestStatus;

class PasswordTestTaskRepositoryTests final : public QObject {
    Q_OBJECT

private slots:
    void addsUpdatesAndListsTasks();
    void preparesTasksForStartup();
    void removesOnlyFinishedTasks();
};

void PasswordTestTaskRepositoryTests::addsUpdatesAndListsTasks()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    PasswordTestTask task;
    task.archiveId = 10;
    task.passwordId = 20;
    task.archivePath = "C:/tmp/archive.zip";
    task.password = "secret";

    QString error;
    const int id = repository.add(task, &error);
    QVERIFY2(id > 0, qPrintable(error));

    task.id = id;
    task.status = PasswordTestTaskStatus::Completed;
    task.testStatus = SevenZipTestStatus::Success;
    task.message = "done";
    QVERIFY2(repository.update(task, &error), qPrintable(error));

    const auto tasks = repository.list();
    QCOMPARE(tasks.size(), 1);
    QCOMPARE(tasks.first().id, id);
    QCOMPARE(tasks.first().archiveId, 10);
    QCOMPARE(tasks.first().passwordId, 20);
    QCOMPARE(tasks.first().status, PasswordTestTaskStatus::Completed);
    QCOMPARE(tasks.first().testStatus, SevenZipTestStatus::Success);
    QCOMPARE(tasks.first().message, QString("done"));
}

void PasswordTestTaskRepositoryTests::preparesTasksForStartup()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    PasswordTestTask waitingTask;
    waitingTask.archivePath = "C:/tmp/waiting.zip";
    waitingTask.password = "secret";
    waitingTask.status = PasswordTestTaskStatus::Waiting;

    PasswordTestTask runningTask;
    runningTask.archivePath = "C:/tmp/running.zip";
    runningTask.password = "secret";
    runningTask.status = PasswordTestTaskStatus::Running;

    QString error;
    QVERIFY2(repository.add(waitingTask, &error) > 0, qPrintable(error));
    QVERIFY2(repository.add(runningTask, &error) > 0, qPrintable(error));
    QVERIFY2(repository.prepareTasksForStartup(&error), qPrintable(error));

    const auto tasks = repository.list();
    QCOMPARE(tasks.size(), 2);
    QCOMPARE(tasks.at(0).status, PasswordTestTaskStatus::Waiting);
    QCOMPARE(tasks.at(1).status, PasswordTestTaskStatus::Failed);
    QCOMPARE(tasks.at(1).testStatus, SevenZipTestStatus::ProcessError);
    QCOMPARE(tasks.at(1).message, QString("Interrupted before application restart."));
}

void PasswordTestTaskRepositoryTests::removesOnlyFinishedTasks()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    AppPaths paths(dir.path());
    QVERIFY(paths.ensureRuntimeDirectories());

    DatabaseService database(paths);
    QVERIFY2(database.open(), qPrintable(database.lastError()));

    PasswordTestTaskRepository repository(database.connectionName());
    PasswordTestTask waitingTask;
    waitingTask.archivePath = "C:/tmp/waiting.zip";
    waitingTask.password = "secret";
    waitingTask.status = PasswordTestTaskStatus::Waiting;

    PasswordTestTask runningTask;
    runningTask.archivePath = "C:/tmp/running.zip";
    runningTask.password = "secret";
    runningTask.status = PasswordTestTaskStatus::Running;

    PasswordTestTask completedTask;
    completedTask.archivePath = "C:/tmp/completed.zip";
    completedTask.password = "secret";
    completedTask.status = PasswordTestTaskStatus::Completed;
    completedTask.testStatus = SevenZipTestStatus::Success;

    PasswordTestTask failedTask;
    failedTask.archivePath = "C:/tmp/failed.zip";
    failedTask.password = "secret";
    failedTask.status = PasswordTestTaskStatus::Failed;

    QString error;
    const int waitingId = repository.add(waitingTask, &error);
    QVERIFY2(waitingId > 0, qPrintable(error));
    QVERIFY2(repository.add(runningTask, &error) > 0, qPrintable(error));
    const int completedId = repository.add(completedTask, &error);
    QVERIFY2(completedId > 0, qPrintable(error));
    QVERIFY2(repository.add(failedTask, &error) > 0, qPrintable(error));

    QVERIFY(!repository.removeFinishedById(waitingId, &error));
    QVERIFY2(repository.removeFinishedById(completedId, &error), qPrintable(error));
    QCOMPARE(repository.removeFinished(&error), 1);

    const auto tasks = repository.list();
    QCOMPARE(tasks.size(), 2);
    QCOMPARE(tasks.at(0).status, PasswordTestTaskStatus::Waiting);
    QCOMPARE(tasks.at(1).status, PasswordTestTaskStatus::Running);
}

QTEST_MAIN(PasswordTestTaskRepositoryTests)
#include "PasswordTestTaskRepositoryTests.moc"
