# Current Feature Overview

## Core Positioning

PasswordManager is a Windows local offline desktop tool for managing plaintext archive passwords and assisting archive extraction.

## Current Modules

- Archive scanning: scan single archive files or folders recursively.
- Password library: add, edit, delete, search, copy, import CSV, and export CSV.
- Password testing: use bundled 7-Zip under `tools/7zip/` to test candidate passwords.
- Task queue: queue password tests, persist task state, recover waiting tasks after restart, filter, retry, delete finished records, and clear finished records.
- History: record successful archive-password associations and extract known archives.
- Windows context menu: add archive tests or view archive results from Explorer.
- Backup and restore: back up or restore the whole local SQLite database.
- Release package: generate portable folder, ZIP archive, manifest, checklist, and SHA256 file.

## Locked Decisions

- Passwords are stored as plaintext in local SQLite.
- The app runs offline and does not upload data.
- 7-Zip is bundled in the project and release directory.
- GPU and multi-threaded acceleration are reserved for a later performance topic.

## Data Files

- Main database: `data/passwordmanager.sqlite3`.
- Backups: `backup/*.sqlite3`.
- Logs: `logs/`.
- Bundled 7-Zip: `tools/7zip/`.

## Current Release Validation

Release validation should include:

- `PasswordManager.exe --smoke-test`
- Release CTest
- ZIP integrity test with bundled 7-Zip
- SHA256 verification
- Manual UI testing in the portable release folder
