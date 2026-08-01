# Phase 40 Plan - Verified Shell Menu Uninstall

## 目标

- 右键菜单卸载后不再只依赖删除调用返回值。
- 卸载完成后逐项反查注册表，明确告诉用户是否仍有残留。
- 如果残留仍存在，设置页弹窗显示具体注册表路径，便于继续定位。

## 实现方案

- 新增 `ShellIntegration::uninstallRegistryKeys()`，集中维护所有当前和历史右键菜单注册表路径。
- Windows 下使用 `RegOpenKeyExW()` 原生反查 key 是否存在。
- `ShellIntegration::uninstall()` 在删除所有 key 后执行残留验证。
- 如发现残留，返回失败并通过 `errorMessage` 输出完整残留列表。

## 验收

- 自动测试覆盖卸载注册表清单。
- 设置页卸载后如果仍有残留，应显示残留路径，而不是提示已卸载。
- 如果注册表清理成功，右键菜单应不再出现 `PasswordManager` 入口。

