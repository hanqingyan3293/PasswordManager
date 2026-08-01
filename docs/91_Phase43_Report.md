# Phase 43 Report - Registry Status Read Stabilization

## 已完成

- `nativeRegistryStringValue()` 从 `RegGetValueW()` 改为 `RegOpenKeyExW()` + `RegQueryValueExW()`。
- 状态读取仍保持只读，不使用 `QSettings` 读取 Windows 注册表。
- 不存在的 key 或值返回空字符串，不创建空壳注册表项。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`4519cfa259adbe404d469b05e0ba097ef809fd59c267d1cbb37c8460394ff282`。

## 手动测试要求

更新发布包后：

- 设置页点击 `重新安装/修复`，状态应显示已安装/正常。
- 设置页点击 `卸载右键菜单`，状态应显示未安装。
- 右键菜单卸载后不应再残留空壳 `PasswordManager.*Menu` key。
