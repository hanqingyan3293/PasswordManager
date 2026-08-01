# Phase 41 Report - Non-Mutating Shell Status Query

## 已完成

- 新增 Windows 原生只读注册表字符串读取，使用 `RegGetValueW()`。
- `ShellIntegration::status()` 不再用 `QSettings` 读取注册表状态。
- `ShellIntegration::isInstalled()` 不再用 `QSettings` 读取注册表状态。
- 保留安装和卸载写操作的现有路径。
- 明确修复卸载后设置页刷新重新创建 `PasswordManager.*Menu` 空壳 key 的问题。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`e5b29f198ceebeb20148e0a70a3081f881108fd1cf3082db3d17aec946cf521b`。

## 手动测试要求

更新发布包后：

1. 打开主程序进入 `设置`。
2. 点击 `卸载右键菜单`。
3. 关闭并重新打开设置页，或点击刷新。
4. 检查注册表中不应重新出现 `PasswordManager.*Menu` 空壳 key。
5. 资源管理器右键不应出现 `PasswordManager`。
