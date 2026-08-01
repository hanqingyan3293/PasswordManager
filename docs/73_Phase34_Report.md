# Phase 34 Report - Explorer Cascading Menu Registry Fix

## Completed

- Replaced the previous `SubCommands=""` cascading-menu layout with `ExtendedSubCommandsKey\Shell`.
- Archive actions are now written under:
  - `SystemFileAssociations\.zip\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
  - `SystemFileAssociations\.rar\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
  - `SystemFileAssociations\.7z\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
- Folder actions are now written under:
  - `Directory\shell\PasswordManager\ExtendedSubCommandsKey\Shell`
- Removed the empty `SubCommands` value from new installations.
- Command paths now use native Windows backslashes.
- `ShellIntegrationTests` now verifies native command formatting.

## Manual Step Required

Open `设置` and click `重新安装/修复` after installing this build. This removes the old `PasswordManager` registry tree and writes the new `ExtendedSubCommandsKey` layout.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 verification: passed.
