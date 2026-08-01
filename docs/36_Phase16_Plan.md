# Phase 16 Plan - Queue Filtering And Manual Retry

## Goal

Improve the task queue so failed work is easier to diagnose and retry without losing history.

## Scope

- Add status filtering to the task queue page.
- Add result filtering based on `SevenZipTestStatus`.
- Add manual retry for finished tasks.
- Keep retry behavior conservative: retry creates a new task record and keeps the old task history.

## Out Of Scope

- Automatic retry count or backoff.
- Multi-threaded execution.
- GPU acceleration.
- Database schema changes.
- Password encryption or migration.

## Acceptance

- Users can filter task rows by task status.
- Users can filter task rows by test result/failure reason.
- Users can select a finished task and create a retry task.
- Waiting or running tasks cannot be retried.
- Existing task history remains intact.
- Debug and Release test suites pass.
