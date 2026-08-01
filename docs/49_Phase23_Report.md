# Phase 23 Report - User Experience Polish

## Completed

- Added empty-state labels for:
  - Home archive table
  - Password library table
  - History table
  - Task queue table
- Enabled table sorting for the same tables.
- Updated selected-record lookup in sorted tables to resolve by table ID.
- Added settings-page buttons:
  - `打开数据目录`
  - `打开备份目录`
  - `打开日志目录`

## Current Behavior

- Empty states are shown only when the current table/filter has no records.
- Sorting is UI-only and does not change database order.
- Directory buttons create the target directory if needed, then open it through Windows.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
