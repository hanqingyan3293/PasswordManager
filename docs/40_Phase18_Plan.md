# Phase 18 Plan - Database Backup And Restore

## Goal

Add local SQLite backup and restore so users can protect plaintext password data before risky operations or manual migration.

## Scope

- Add a database backup service.
- Generate timestamped `.sqlite3` backup files under `backup/`.
- Add settings-page controls for immediate backup and restore from backup.
- Validate restore source before replacing the current database.
- Create a safety backup before restore.
- Exit the application after restore so the next launch opens the restored database cleanly.
- Add automated tests for backup and restore.

## Out Of Scope

- Cloud sync.
- Scheduled automatic backup.
- Password encryption.
- CSV/Excel import and export.
- Partial table-level restore.
- GPU or multi-threaded acceleration.

## Acceptance

- Backup creates a readable `.sqlite3` file in the configured backup directory.
- Restore rejects non-PasswordManager database files.
- Restore creates a safety backup of the current database before replacement.
- Restored data is available after reopening the database.
- Debug and Release tests pass.
