#include "PasswordManager/app/PasswordTestTaskManager.h"

#include "PasswordManager/app/SevenZipRunner.h"
#include "PasswordManager/data/PasswordTestTaskRepository.h"

#include <QFileInfo>

#include <utility>

namespace PasswordManager {

namespace {

bool isFinishedTask(PasswordTestTaskStatus status)
{
    return status == PasswordTestTaskStatus::Completed
        || status == PasswordTestTaskStatus::Failed
        || status == PasswordTestTaskStatus::Cancelled
        || status == PasswordTestTaskStatus::Timeout;
}

} // namespace

PasswordTestTaskManager::PasswordTestTaskManager(QString sevenZipExecutable, QObject* parent)
    : PasswordTestTaskManager(std::move(sevenZipExecutable), nullptr, parent)
{
}

PasswordTestTaskManager::PasswordTestTaskManager(QString sevenZipExecutable, const PasswordTestTaskRepository* repository, QObject* parent)
    : QObject(parent)
    , m_sevenZipExecutable(std::move(sevenZipExecutable))
    , m_process(new QProcess(this))
    , m_timeout(new QTimer(this))
    , m_repository(repository)
{
    m_timeout->setSingleShot(true);

    connect(m_timeout, &QTimer::timeout, this, [this]() {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
        }
        finishRunningTask(PasswordTestTaskStatus::Timeout, SevenZipTestStatus::Timeout, "7-Zip password test timed out.");
    });

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
        if (runningTaskIndex() < 0) {
            return;
        }
        m_timeout->stop();

        const QString output = QString::fromLocal8Bit(m_process->readAllStandardOutput())
            + QString::fromLocal8Bit(m_process->readAllStandardError());
        SevenZipTestStatus testStatus = SevenZipTestStatus::ProcessError;
        PasswordTestTaskStatus taskStatus = PasswordTestTaskStatus::Failed;
        QString message = output.left(800);

        const QString lower = output.toLower();
        if (exitStatus == QProcess::NormalExit && exitCode == 0 && lower.contains("everything is ok")) {
            testStatus = SevenZipTestStatus::Success;
            taskStatus = PasswordTestTaskStatus::Completed;
            message = "Password is correct.";
        } else if (lower.contains("wrong password") || lower.contains("can not open encrypted archive")) {
            testStatus = SevenZipTestStatus::WrongPassword;
            message = "Password is wrong.";
        } else if (lower.contains("is not archive") || lower.contains("headers error") || lower.contains("data error")) {
            testStatus = SevenZipTestStatus::ArchiveError;
            message = "Archive is damaged or unsupported.";
        } else if (exitStatus != QProcess::NormalExit) {
            message = "7-Zip process exited abnormally.";
        }

        finishRunningTask(taskStatus, testStatus, message);
    });

    loadPersistedTasks();
    startNext();
}

int PasswordTestTaskManager::enqueuePasswordTest(const QString& archivePath, const QString& password, int timeoutMs)
{
    return enqueuePasswordTest(0, 0, archivePath, password, timeoutMs);
}

int PasswordTestTaskManager::enqueuePasswordTest(int archiveId, int passwordId, const QString& archivePath, const QString& password, int timeoutMs)
{
    PasswordTestTask task;
    task.id = m_nextId++;
    task.archiveId = archiveId;
    task.passwordId = passwordId;
    task.archivePath = archivePath;
    task.password = password;
    task.timeoutMs = timeoutMs;

    if (!QFileInfo::exists(m_sevenZipExecutable)) {
        task.status = PasswordTestTaskStatus::Failed;
        task.testStatus = SevenZipTestStatus::MissingSevenZip;
        task.message = "Bundled 7-Zip does not exist.";
        persistNewTask(&task);
        m_tasks.append(task);
        emit tasksChanged();
        return task.id;
    }

    if (!password.isEmpty() && isNoPasswordArchive(archivePath, timeoutMs)) {
        task.password.clear();
        task.passwordId = 0;
        task.status = PasswordTestTaskStatus::Completed;
        task.testStatus = SevenZipTestStatus::NoPasswordRequired;
        task.message = "Archive does not require a password.";
        persistNewTask(&task);
        m_tasks.append(task);
        emit tasksChanged();
        emit taskFinished(task);
        return task.id;
    }

    persistNewTask(&task);
    m_tasks.append(task);
    emit tasksChanged();
    startNext();
    return task.id;
}

SevenZipTestResult PasswordTestTaskManager::testPasswordNow(const QString& archivePath, const QString& password, int timeoutMs) const
{
    return SevenZipRunner(m_sevenZipExecutable).testPassword(archivePath, password, timeoutMs);
}

bool PasswordTestTaskManager::isNoPasswordArchive(const QString& archivePath, int timeoutMs) const
{
    return testPasswordNow(archivePath, QString(), timeoutMs).status == SevenZipTestStatus::Success;
}

QList<PasswordTestTask> PasswordTestTaskManager::tasks() const
{
    return m_tasks;
}

void PasswordTestTaskManager::cancelTask(int id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id != id) {
            continue;
        }

        if (m_tasks[i].status == PasswordTestTaskStatus::Waiting) {
            m_tasks[i].status = PasswordTestTaskStatus::Cancelled;
            m_tasks[i].message = "Task cancelled.";
            persistTaskUpdate(m_tasks[i]);
            emit tasksChanged();
            return;
        }

        if (m_tasks[i].status == PasswordTestTaskStatus::Running) {
            if (m_process->state() != QProcess::NotRunning) {
                m_process->kill();
            }
            finishRunningTask(PasswordTestTaskStatus::Cancelled, SevenZipTestStatus::ProcessError, "Task cancelled.");
            return;
        }
    }
}

int PasswordTestTaskManager::retryTask(int id)
{
    for (const PasswordTestTask& task : std::as_const(m_tasks)) {
        if (task.id != id) {
            continue;
        }

        if (task.status == PasswordTestTaskStatus::Waiting || task.status == PasswordTestTaskStatus::Running) {
            return 0;
        }

        return enqueuePasswordTest(task.archiveId, task.passwordId, task.archivePath, task.password, task.timeoutMs);
    }
    return 0;
}

bool PasswordTestTaskManager::removeFinishedTask(int id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).id != id) {
            continue;
        }

        if (!isFinishedTask(m_tasks.at(i).status)) {
            return false;
        }

        if (m_repository && !m_repository->removeFinishedById(id)) {
            return false;
        }

        m_tasks.removeAt(i);
        emit tasksChanged();
        return true;
    }
    return false;
}

int PasswordTestTaskManager::clearFinishedTasks()
{
    int removed = 0;
    if (m_repository) {
        removed = m_repository->removeFinished();
        if (removed < 0) {
            return -1;
        }
    }

    if (!m_repository) {
        for (int i = m_tasks.size() - 1; i >= 0; --i) {
            if (isFinishedTask(m_tasks.at(i).status)) {
                m_tasks.removeAt(i);
                ++removed;
            }
        }
    } else {
        for (int i = m_tasks.size() - 1; i >= 0; --i) {
            if (isFinishedTask(m_tasks.at(i).status)) {
                m_tasks.removeAt(i);
            }
        }
    }

    if (removed > 0) {
        emit tasksChanged();
    }
    return removed;
}

void PasswordTestTaskManager::loadPersistedTasks()
{
    if (!m_repository) {
        return;
    }

    m_repository->prepareTasksForStartup();
    m_tasks = m_repository->list();
    for (const PasswordTestTask& task : std::as_const(m_tasks)) {
        if (task.id >= m_nextId) {
            m_nextId = task.id + 1;
        }
    }
}

void PasswordTestTaskManager::persistNewTask(PasswordTestTask* task)
{
    if (!m_repository || !task) {
        return;
    }

    const int id = m_repository->add(*task);
    if (id > 0) {
        task->id = id;
        if (task->id >= m_nextId) {
            m_nextId = task->id + 1;
        }
    }
}

void PasswordTestTaskManager::persistTaskUpdate(const PasswordTestTask& task)
{
    if (!m_repository) {
        return;
    }
    m_repository->update(task);
}

void PasswordTestTaskManager::startNext()
{
    if (m_process->state() != QProcess::NotRunning || runningTaskIndex() >= 0) {
        return;
    }

    for (PasswordTestTask& task : m_tasks) {
        if (task.status != PasswordTestTaskStatus::Waiting) {
            continue;
        }

        if (!task.password.isEmpty() && isNoPasswordArchive(task.archivePath, task.timeoutMs)) {
            task.password.clear();
            task.passwordId = 0;
            task.status = PasswordTestTaskStatus::Completed;
            task.testStatus = SevenZipTestStatus::NoPasswordRequired;
            task.message = "Archive does not require a password.";
            persistTaskUpdate(task);
            const PasswordTestTask finishedTask = task;
            emit tasksChanged();
            emit taskFinished(finishedTask);
            continue;
        }

        task.status = PasswordTestTaskStatus::Running;
        task.message = "Running.";
        persistTaskUpdate(task);
        m_process->setProgram(m_sevenZipExecutable);
        const QStringList arguments = {"t", "-y", "-p" + task.password, task.archivePath};
        m_process->setArguments(arguments);
        m_process->start();
        m_timeout->start(task.timeoutMs > 0 ? task.timeoutMs : 10000);
        emit tasksChanged();
        return;
    }
}

void PasswordTestTaskManager::finishRunningTask(PasswordTestTaskStatus status, SevenZipTestStatus testStatus, const QString& message)
{
    const int index = runningTaskIndex();
    if (index < 0) {
        return;
    }

    m_timeout->stop();
    m_tasks[index].status = status;
    m_tasks[index].testStatus = testStatus;
    m_tasks[index].message = message;
    persistTaskUpdate(m_tasks[index]);
    const PasswordTestTask finishedTask = m_tasks[index];
    emit tasksChanged();
    emit taskFinished(finishedTask);
    startNext();
}

int PasswordTestTaskManager::runningTaskIndex() const
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks.at(i).status == PasswordTestTaskStatus::Running) {
            return i;
        }
    }
    return -1;
}

QString passwordTestTaskStatusText(PasswordTestTaskStatus status)
{
    switch (status) {
    case PasswordTestTaskStatus::Waiting:
        return "WAITING";
    case PasswordTestTaskStatus::Running:
        return "RUNNING";
    case PasswordTestTaskStatus::Completed:
        return "COMPLETED";
    case PasswordTestTaskStatus::Failed:
        return "FAILED";
    case PasswordTestTaskStatus::Cancelled:
        return "CANCELLED";
    case PasswordTestTaskStatus::Timeout:
        return "TIMEOUT";
    }
    return "UNKNOWN";
}

} // namespace PasswordManager
