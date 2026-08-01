# Phase 29 Report - Batch Matching, History Links, Compatibility, Help

## Completed

- Home archive table now supports extended row selection.
- `智能匹配测试` now supports one or more selected archives and queues candidates for each selected archive.
- Confirmation message now shows archive count, candidate count, and total task count.
- History page now supports copying the password and copying the whole row.
- History page now supports jumping to the matching password-library record.
- History page now supports deleting one success-history association without deleting the password-library record.
- Settings page now includes shortcuts for application directory, user manual, and test data.
- Release packaging now copies `docs/46_User_Manual.md` to portable `USER_MANUAL.md`.
- Compatibility baseline: 20 encrypted ZIP/7Z fixtures and 1 no-password ZIP fixture passed with bundled 7-Zip.
- Added repository coverage for deleting history while preserving the password-library record.

## Compatibility Baseline

- ZIP encrypted fixtures: passed.
- 7Z encrypted fixtures: passed.
- ZIP no-password fixture: passed.
- RAR fixtures: not included in local test fixtures yet.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- Fixture compatibility test with bundled `tools/7zip/7z.exe`: 21/21 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release portable `USER_MANUAL.md`: present.
- Release ZIP SHA256 verification: passed.
