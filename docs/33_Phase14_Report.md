# Phase 14 Report - Persistent Task History

## Completed

- Added SQLite table `password_test_tasks`.
- Database schema version is now `5`.
- Added `PasswordTestTaskRepository`.
- `PasswordTestTaskManager` now supports optional persistence.
- The main application now constructs `PasswordTestTaskManager` with `PasswordTestTaskRepository`.
- Task records are inserted when enqueued.
- Task records are updated when they start, finish, fail, time out, or are cancelled.
- Persisted task records are loaded on startup.
- Previously persisted `WAITING` or `RUNNING` tasks are marked `CANCELLED` on startup to avoid accidental reruns.
- Added `PasswordTestTaskRepositoryTests`.
- Expanded `PasswordTestTaskManagerTests` with persistence coverage.

## Behavior

The task queue page now displays persisted task history after restart.

Unfinished tasks from a previous process are not automatically resumed. They are marked:

```text
CANCELLED
Interrupted before application restart.
```

This is intentional for the current phase. Automatic recovery/retry needs a separate scheduling decision.

## Validation Results

| Check | Result |
| --- | --- |
| Debug CTest in `build/` | Passed, 9/9 |
| Release package build | Passed |
| Release CTest in `build-release/` | Passed, 9/9 |
| Release smoke test | Passed, exit code 0 |
| ZIP integrity test | Passed |

## Notes

- Passwords are still plaintext in SQLite by locked user decision.
- GPU and multi-thread acceleration remain future topics.
- Automatic queue resume is not implemented in this phase.
