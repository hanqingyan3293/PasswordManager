# Phase 15 Plan - Unfinished Task Recovery

## Goal

Make persisted password-test tasks recover predictably after application restart:

- Keep persisted `WAITING` tasks and automatically continue them on startup.
- Convert persisted `RUNNING` tasks from the previous process to `FAILED`.
- Continue using the local SQLite `password_test_tasks` table.
- Keep the current plaintext password storage decision unchanged.

## Scope

- `PasswordTestTaskRepository`: add startup preparation behavior.
- `PasswordTestTaskManager`: load persisted tasks and continue waiting tasks after signal wiring is ready.
- Tests: cover startup recovery and interrupted running task handling.
- Docs: update README, changelog, and phase report.

## Out Of Scope

- GPU acceleration.
- Multi-threaded queue execution.
- Retry count/backoff policy.
- Password encryption or migration.
- Right-click command format changes.

## Acceptance

- `WAITING` tasks continue after restart.
- `RUNNING` tasks become `FAILED` with an interruption message after restart.
- Debug and Release CTest suites pass.
- Portable release package, smoke-test, and ZIP integrity checks pass.
