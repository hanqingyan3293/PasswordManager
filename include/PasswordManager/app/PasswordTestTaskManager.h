#pragma once

#include "PasswordManager/app/SevenZipRunner.h"

#include <QList>
#include <QObject>
#include <QProcess>
#include <QTimer>

namespace PasswordManager {

class PasswordTestTaskRepository;

enum class PasswordTestTaskStatus {
    Waiting,
    Running,
    Completed,
    Failed,
    Cancelled,
    Timeout
};

struct PasswordTestTask {
    int id = 0;
    int archiveId = 0;
    int passwordId = 0;
    QString archivePath;
    QString password;
    PasswordTestTaskStatus status = PasswordTestTaskStatus::Waiting;
    SevenZipTestStatus testStatus = SevenZipTestStatus::ProcessError;
    QString message;
    int timeoutMs = 10000;
};

class PasswordTestTaskManager final : public QObject {
    Q_OBJECT

public:
    explicit PasswordTestTaskManager(QString sevenZipExecutable, QObject* parent = nullptr);
    PasswordTestTaskManager(QString sevenZipExecutable, const PasswordTestTaskRepository* repository, QObject* parent = nullptr);

    int enqueuePasswordTest(const QString& archivePath, const QString& password, int timeoutMs = 10000);
    int enqueuePasswordTest(int archiveId, int passwordId, const QString& archivePath, const QString& password, int timeoutMs = 10000);
    SevenZipTestResult testPasswordNow(const QString& archivePath, const QString& password, int timeoutMs = 10000) const;
    bool isNoPasswordArchive(const QString& archivePath, int timeoutMs = 10000) const;
    QList<PasswordTestTask> tasks() const;
    void cancelTask(int id);
    int retryTask(int id);
    bool removeFinishedTask(int id);
    int clearFinishedTasks();

signals:
    void tasksChanged();
    void taskFinished(const PasswordTestTask& task);

private:
    void loadPersistedTasks();
    void persistNewTask(PasswordTestTask* task);
    void persistTaskUpdate(const PasswordTestTask& task);
    void startNext();
    void finishRunningTask(PasswordTestTaskStatus status, SevenZipTestStatus testStatus, const QString& message);
    int runningTaskIndex() const;

    QString m_sevenZipExecutable;
    QList<PasswordTestTask> m_tasks;
    QProcess* m_process = nullptr;
    QTimer* m_timeout = nullptr;
    const PasswordTestTaskRepository* m_repository = nullptr;
    int m_nextId = 1;
};

QString passwordTestTaskStatusText(PasswordTestTaskStatus status);

} // namespace PasswordManager
