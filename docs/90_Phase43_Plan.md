# Phase 43 Plan - Registry Status Read Stabilization

## 目标

- 修复设置页安装状态仍显示未安装或不完整的问题。
- 保持状态刷新只读，不重新创建注册表 key。
- 提高 Windows 注册表状态读取的确定性。

## 调整

- 将状态读取从 `RegGetValueW()` 改为：
  - `RegOpenKeyExW()` 打开 key。
  - `RegQueryValueExW()` 查询值长度和类型。
  - `RegQueryValueExW()` 读取字符串值。
- 默认值仍用 `nullptr` 读取。
- 不存在的 key 或值返回空字符串，不创建任何注册表项。

## 验收

- 设置页点击安装或重新安装/修复后，状态应显示正常。
- 设置页点击卸载后，状态应显示未安装。
- 卸载后刷新状态不应创建空壳 key。

