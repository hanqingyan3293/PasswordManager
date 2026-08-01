# Phase 16 Report - Queue Filtering And Manual Retry

## Completed

- Added task status filtering in the task queue page.
- Added result/failure-reason filtering in the task queue page:
  - `SUCCESS`
  - `WRONG_PASSWORD`
  - `MISSING_7ZIP`
  - `ARCHIVE_ERROR`
  - `TIMEOUT`
  - `PROCESS_ERROR`
- Added `PasswordTestTaskManager::retryTask(int id)`.
- Manual retry creates a new task from the selected finished task and preserves the previous task row.
- Waiting and running tasks are rejected for retry.
- Added automated coverage for retry behavior.
- Refined the task queue table after manual UI testing:
  - status and test result cells now display Chinese labels;
  - table columns use interactive resizing so users can drag column boundaries;
  - users can copy the current cell or the whole row from buttons or the context menu.
  - table selection now highlights a single cell instead of the whole row, making copied cells clear.
- Refined the password library table after manual UI testing:
  - table columns use interactive resizing;
  - users can copy the current cell or the whole row from buttons or the context menu;
  - the active row and active cell use different highlight colors.
- Refined the password dialog after manual UI testing:
  - success and failure counts now use normal numeric text inputs with integer validation.
  - manual test seed notes are written as UTF-8 Chinese text.

## Current Behavior

- The task queue page can show all tasks or a filtered subset.
- Failure reason grouping uses the existing `test_status` field, so no database migration is needed.
- Retried tasks run through the same queue and persistence path as newly enqueued tasks.
- Copying a row uses tab-separated text so it can be pasted into spreadsheets or text files.
- Password success/failure count fields accept only integers from `0` to `1000000`.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 9/9 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 9/9 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
