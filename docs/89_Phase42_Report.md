# Phase 42 Report - Registry Value Length Probe Fix

## 已完成

- 修复 `nativeRegistryStringValue()` 的长度探测判断。
- 首次 `RegGetValueW()` 返回 `ERROR_MORE_DATA` 时不再误判失败。
- 保留 Phase 41 的 Windows 原生只读状态查询。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`bbba0cc846f765ab3b74a03d7fa5beb141623ea84a59d7b74678d6e7e4fde01f`。

## 手动测试要求

更新发布包后：

- 在设置页点击 `重新安装/修复`，状态应显示已安装/正常。
- 再点击 `卸载右键菜单`，状态应显示未安装，且不应残留空壳 key。
