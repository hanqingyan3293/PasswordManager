# Phase 40 Report - Verified Shell Menu Uninstall

## 已完成

- 新增 `ShellIntegration::uninstallRegistryKeys()`，集中列出当前和历史所有 `PasswordManager` 右键菜单注册表路径。
- 卸载流程在删除后使用原生 `RegOpenKeyExW()` 逐项验证残留。
- 如果仍有残留，`uninstall()` 返回失败，并把残留 key 列表传给设置页错误弹窗。
- `ShellIntegrationTests` 增加卸载注册表清单覆盖。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`9af6f2ce8a5fed2c102cb2ee79ce16c0966da6565c3f09d54a15f0ac4b45d721`。

## 手动测试要求

更新发布包后进入 `设置`，点击 `卸载右键菜单`：

- 如果卸载成功，应提示已卸载，资源管理器右键不应再有 `PasswordManager`。
- 如果仍残留，应弹出“右键菜单卸载后仍有残留注册表项”，并列出具体 key。
- 如果弹出残留列表，把列表发回来，下一步按具体 key 继续处理。
