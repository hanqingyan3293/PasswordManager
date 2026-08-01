# Phase 14 Plan - Persistent Task History

## Goal

Persist password-test task history to SQLite so the task queue page can show previous task states after the application restarts.

## Scope

- Add SQLite table `password_test_tasks`.
- Add `PasswordTestTaskRepository`.
- Persist task records when they are enqueued.
- Update persisted task records when they start, finish, fail, time out, or are cancelled.
- Load persisted task history at application startup.
- Mark previously `WAITING` or `RUNNING` tasks as `CANCELLED` on startup to avoid accidental automatic reruns.

## Out of Scope

- Automatically resuming unfinished tasks after restart.
- Multi-threaded testing.
- GPU acceleration.
- Queue scheduling policy changes.
- Password encryption.

## Acceptance Criteria

1. Database schema version becomes 5.
2. New tasks are inserted into `password_test_tasks`.
3. Task status updates are written to SQLite.
4. A new `PasswordTestTaskManager` can load previous task records.
5. Interrupted `WAITING` or `RUNNING` tasks become `CANCELLED` on startup.
6. Existing automated tests still pass.

## Rollback

Revert the Phase 14 source changes and remove `docs/32_Phase14_Plan.md` / `docs/33_Phase14_Report.md`.
