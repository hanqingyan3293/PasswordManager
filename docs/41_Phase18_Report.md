# Phase 18 Report - Database Backup And Restore

## Completed

- Added `DatabaseBackupService`.
- Backup uses SQLite `VACUUM INTO` to generate a consistent `.sqlite3` copy.
- Backup files are written under the existing `backup/` runtime directory.
- Restore validates that the selected file is a PasswordManager SQLite database.
- Restore creates a safety backup of the current database before replacing it.
- Restore closes the active database connection, replaces the database file, and exits the app so the next launch opens the restored data cleanly.
- Settings page now includes:
  - `立即备份`
  - `从备份恢复`
- Added `DatabaseBackupServiceTests`.

## Current Behavior

- Backup and restore operate on the whole local SQLite database.
- Passwords remain plaintext, matching the locked project decision.
- Restore does not merge records; it replaces the current database with the selected backup.
- The current database is saved as `passwordmanager-before-restore-*.sqlite3` before replacement.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 10/10 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 10/10 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
