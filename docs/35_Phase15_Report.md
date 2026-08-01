# Phase 15 Report - Unfinished Task Recovery

## Completed

- Replaced startup cancellation with recovery preparation:
  - `WAITING` tasks stay queued.
  - stale `RUNNING` tasks become `FAILED`.
  - stale running tasks store `PROCESS_ERROR` and `Interrupted before application restart.`.
- `PasswordTestTaskManager` now loads persisted tasks after process/timer signals are connected, then starts the next waiting task automatically.
- Persisted tasks are loaded by `id ASC`, preserving queue order on recovery.
- Added and updated tests for repository and manager startup recovery behavior.

## Current Behavior

- New tasks still run sequentially.
- After application restart:
  - tasks that had not started continue automatically;
  - tasks that were running in the previous process are marked failed;
  - historical task rows remain visible from the task queue page.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 9/9 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 9/9 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
