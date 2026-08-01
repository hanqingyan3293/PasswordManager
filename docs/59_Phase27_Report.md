# Phase 27 Report - Manual Test Learning and Empty Table Regression

## Completed

- Restored visible empty tables on home, password library, history, and task queue pages.
- Kept empty-state guidance text below the table instead of replacing the table.
- Added `PasswordRepository::findByPassword()`.
- Updated successful password-test handling so manual non-empty passwords are learned into the plaintext password library.
- Existing password-library records are reused and their success count is incremented.
- Empty successful passwords are only saved to archive success history, not to the password library.
- Release packaging now preserves local portable `data`, `config`, `logs`, and `backup` directories after producing the clean ZIP package.
- Added repository coverage for plaintext password lookup.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed after the old portable database file handle was released.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 verification: passed.
