# Phase 12 Report - Shell Action Queue Flow

## Completed

- Added `ShellActionService`.
- `--shell-action add-test-queue <archive>` now performs real application work:
  - validates the selected file
  - scans the archive
  - saves or updates the archive database record
  - builds password candidates from the local password library
  - enqueues password test tasks
- Added `ArchiveRepository::findByPath()` so shell actions can reload the saved archive ID after upsert.
- `MainWindow::openShellAction()` now routes successful queue actions to the task queue page.
- Added `ShellActionServiceTests`.
- Automated test count increased from 7 to 8.

## Behavior

```powershell
PasswordManager.exe --shell-action add-test-queue "C:\path\archive.zip"
```

Expected result:

1. The app scans and registers the archive.
2. The app reads plaintext passwords from SQLite.
3. The app uses the existing password matcher to produce unique candidates.
4. The app creates one password-test task per candidate.
5. The app opens the task queue page.

If the file is missing, unsupported, or the password library is empty, no task is enqueued and the status bar shows the reason.

## Validation Results

| Check | Result |
| --- | --- |
| Debug CTest in `build/` | Passed, 8/8 |
| Release package build | Passed |
| Release CTest in `build-release/` | Passed, 8/8 |
| Release smoke test | Passed, exit code 0 |
| ZIP integrity test | Passed |

## Notes

- This phase does not change registry keys. Existing installed shell menu entries continue to call the same command.
- This phase does not add GPU or multi-thread acceleration.
- Passwords remain plaintext in SQLite by locked user decision.
