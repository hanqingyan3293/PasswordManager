# Phase 17 Plan - Task Queue Cleanup

## Goal

Add conservative task queue cleanup so the queue history can be maintained without touching password data.

## Scope

- Add a selected-row delete action for finished task records.
- Add a bulk cleanup action for finished task records.
- Treat only `COMPLETED`, `FAILED`, `CANCELLED`, and `TIMEOUT` as removable.
- Keep `WAITING` and `RUNNING` records protected.
- Add repository and manager tests for cleanup behavior.

## Out Of Scope

- Password library deletion or cleanup.
- `archive_passwords` success-history cleanup.
- Automatic date/count retention rules.
- Database schema migration.
- Password encryption changes.
- GPU or multi-threaded acceleration.

## Acceptance

- Finished task records can be deleted one by one.
- Finished task records can be cleared in bulk.
- Waiting and running task records are preserved.
- The task queue table refreshes after deletion.
- Debug and Release tests pass.
- Release package, ZIP integrity, and SHA256 verification pass.
