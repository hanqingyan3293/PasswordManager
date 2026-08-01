# Phase 17 Report - Task Queue Cleanup

## Completed

- Added `PasswordTestTaskRepository::removeFinishedById(int id)`.
- Added `PasswordTestTaskRepository::removeFinished()`.
- Added `PasswordTestTaskManager::removeFinishedTask(int id)`.
- Added `PasswordTestTaskManager::clearFinishedTasks()`.
- Task cleanup is limited to finished statuses:
  - `COMPLETED`
  - `FAILED`
  - `CANCELLED`
  - `TIMEOUT`
- `WAITING` and `RUNNING` task records are protected.
- Task queue page now has:
  - `删除记录`
  - `清理已结束`
  - context-menu `删除记录`
- Deletion confirmation dialogs explain that password library records and success history are not deleted.
- Added repository coverage for preserving waiting/running tasks.
- Added manager coverage for removing finished tasks and refreshing the in-memory list.

## Current Behavior

- Deleting a selected record only works after the task has ended.
- Bulk cleanup removes all ended task records and keeps active or waiting work intact.
- Cleanup affects only `password_test_tasks`.
- Password plaintext storage remains unchanged.
- Successful password history in `archive_passwords` remains unchanged.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 9/9 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 9/9 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
