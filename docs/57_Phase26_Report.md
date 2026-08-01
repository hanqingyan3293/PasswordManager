# Phase 26 Report - Manual Password Test Guardrails

## Completed

- Updated the home-page password input placeholder to document no-password testing.
- Empty manual password test now asks for confirmation and queues an empty password when confirmed.
- Smart matching now asks for confirmation before queuing automatic candidate tasks.
- Smart matching empty-library message now points users to empty-password manual testing.
- Added `fixture_21_no_password.zip`.
- Added no-password coverage to `SevenZipRunnerTests`.
- Added no-password coverage to `PasswordTestTaskManagerTests`.

## Current Behavior

- `测试密码` is the explicit path for one password or no-password archive testing.
- `智能匹配测试` remains available, but it no longer starts silently.
- No-password testing uses an empty password string passed to bundled 7-Zip.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 verification: passed.
