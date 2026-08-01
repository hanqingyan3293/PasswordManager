# Phase 39 Report - Stable File Compress Menu and Registry Delete Fallback

## 已完成

- 普通文件右键 `打包压缩包` 改为走 `PasswordManager.exe --shell-action compress-archive "%1"`。
- `compress-archive` 动作会判断传入路径类型：
  - 文件：打开父目录。
  - 文件夹：打开自身目录。
- 压缩包和文件夹菜单仍保持直接调用内置 `7zFM.exe` 的现有行为。
- 注册表删除增加 `RegDeleteTreeW()` 兜底，覆盖当前项和历史残留项。
- 更新 `ShellIntegrationTests`，覆盖普通文件打包命令。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`ada9a71c8437cc87119f2ac78fdbf763514356d8afca65a51afe946629cc187e`。

## 手动测试要求

更新发布包后，进入 `设置` 执行 `重新安装/修复`。

重点检查：

- txt、py、json 右键 `打包压缩包` 应打开 7-Zip File Manager 到所在目录，不应再报 `Cannot open file ... as archive`。
- zip、exe、blend、文件夹右键仍能打开 7-Zip File Manager。
- 设置页执行 `卸载右键菜单` 后，资源管理器右键不应残留 `PasswordManager`。
- 7-Zip GUI 中文化本阶段未继续处理，如仍显示英文，后续单独补默认语言配置。
