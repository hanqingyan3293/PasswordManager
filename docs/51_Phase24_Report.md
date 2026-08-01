# Phase 24 Report - Logs And Diagnostics

## Completed

- Added `DiagnosticService`.
- Added settings-page `导出诊断包`.
- Diagnostic export writes `diagnostic.txt`.
- Diagnostic export copies existing `.log` files.
- Diagnostic text includes:
  - app version
  - Qt version
  - OS
  - runtime paths
  - bundled 7-Zip status
  - database table counts
- Added `DiagnosticServiceTests`.

## Current Behavior

- Diagnostic export creates a directory under `logs/diagnostic-*`.
- The full SQLite database is not included in the diagnostic directory.
- The diagnostic directory opens after successful export.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
