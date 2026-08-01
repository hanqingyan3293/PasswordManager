# Phase 32 Report - Explorer Menu Structure and Shell Actions

## Completed

- Reworked archive right-click registration into one top-level `压缩包密码管理器` menu with grouped submenus.
- Added archive menu group `密码测试`:
  - `自动查找密码`
  - `使用密码库测试`
- Added archive menu group `任务`:
  - `添加到测试队列`
  - `查看结果`
  - `打开主程序`
- Added folder right-click registration:
  - `扫描文件夹`
  - `打开主程序`
- Added `lookup-password`, `scan-folder`, and `open-main` shell actions.
- `自动查找密码` now scans/registers the archive, looks up known successful password history, and shows a popup result.
- `使用密码库测试` still only queues password tests and does not extract.
- Fixed `ShellActionService` so an empty password library is reported as failure instead of a misleading successful queue operation.
- Changed `DatabaseService` to use a unique SQLite connection name per instance, improving test isolation.
- Extended `ShellActionServiceTests` for known-password lookup and folder scan behavior.

## Deferred

- `智能匹配测试并解压` needs a separate phase because it must define confirmation, output folder, overwrite, partial failure, and logging behavior.
- True multi-select Explorer batch handling still needs a separate implementation and manual Windows Explorer test.
- Settings page still has a global install/uninstall/repair control. Per-action switches are not implemented yet.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 verification: passed.
