# Phase 41 Plan - Non-Mutating Shell Status Query

## 目标

- 修复设置页卸载后仍看到 `PasswordManager.*Menu` 空壳 key 的问题。
- 右键菜单状态刷新必须是只读操作，不能因为查询状态而重新创建注册表 key。
- 保留现有安装、修复、卸载流程。

## 根因

`ShellIntegration::status()` 使用 `QSettings` 读取 Windows NativeFormat 注册表路径。对不存在的 key 构造 `QSettings` 后，Qt 可能创建空 key，导致卸载成功后，设置页刷新状态又生成：

- `PasswordManager.ArchiveMenu`
- `PasswordManager.FileMenu`
- `PasswordManager.DirectoryMenu`
- `PasswordManager.DirectoryBackgroundMenu`

这些空壳 key 没有真实命令，但会造成“卸载后注册表仍有残留”的现象。

## 实现方案

- Windows 下新增 `RegGetValueW()` 只读字符串读取。
- `ShellIntegration::status()` 和 `ShellIntegration::isInstalled()` 改用原生只读 API。
- 写入和删除仍保留 `QSettings` + Windows 原生删除兜底。

## 验收

- 设置页刷新状态不应创建 `PasswordManager.*Menu` 空壳 key。
- 卸载后再刷新状态，不应重新出现空壳 key。
- 自动测试继续通过。

