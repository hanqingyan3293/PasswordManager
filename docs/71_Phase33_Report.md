# Phase 33 Report - Context Menu Feedback and Window Size Fix

## Completed

- Changed archive registry layout from grouped third-level submenus to direct second-level actions:
  - `自动查找密码`
  - `使用密码库测试`
  - `添加到测试队列`
  - `查看结果`
  - `打开主程序`
- Changed folder registry layout to direct second-level actions:
  - `扫描文件夹`
  - `打开主程序`
- Registry install now clears the app's old `PasswordManager` menu tree before writing the new layout.
- Shell actions now run after the main window is shown, so page changes and message boxes are visible.
- Added popup feedback for:
  - `查看结果`
  - `使用密码库测试`
  - `添加到测试队列`
  - `扫描文件夹`
  - `打开主程序`
  - unknown shell actions
- Added stronger shell-action foreground handling on Windows:
  - `showNormal()`
  - `raise()`
  - `activateWindow()`
  - native `SetForegroundWindow()`
- Shell-action message boxes now stay on top of the app window.
- Added shell-action log lines for requested, executing, and completed actions.
- Reduced the main window default size from 1120 x 720 to 960 x 640.
- Set the main window minimum size to 680 x 420.
- Changed the left navigation from fixed 180 px width to a 120-180 px range.
- Fixed settings-page layout at smaller window sizes:
  - Added a vertical scroll area.
  - Changed the directory action buttons from one long row to a two-row grid.
  - Moved right-click menu and backup action buttons into multiple rows.
  - Allowed long path and status labels to shrink and wrap instead of stretching the page width.

## Manual Step Required

Open `设置` and click `重新安装/修复` after installing this build. This clears the old third-level registry layout and writes the new second-level layout.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 verification: passed.
