# Phase 24 Plan - Logs And Diagnostics

## Goal

Add a local diagnostic export so real-environment issues can be inspected without manually collecting paths, logs, and database counts.

## Scope

- Add diagnostic export service.
- Export diagnostic text under `logs/diagnostic-*`.
- Copy existing `.log` files into the diagnostic directory.
- Include app version, Qt version, OS, runtime paths, 7-Zip status, and database table counts.
- Add a settings-page `导出诊断包` button.
- Add automated test coverage.

## Out Of Scope

- Exporting the full SQLite database.
- Uploading logs.
- Redacting plaintext CSV exports.
- Crash dump integration.

## Acceptance

- Diagnostic export creates a timestamped directory.
- `diagnostic.txt` is generated.
- Log files are copied when present.
- Database counts are included.
- Existing tests pass.
