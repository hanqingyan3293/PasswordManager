# Phase 39 Plan - Stable File Compress Menu and Registry Delete Fallback

## 目标

- 普通文件右键 `打包压缩包` 采用方案 C：打开文件所在目录的 7-Zip File Manager。
- 避免 txt、py、json 等普通文件被 `7zFM.exe "%1"` 当作压缩包打开并报错。
- 卸载右键菜单时增加 Windows 原生注册表删除兜底，解决 `PasswordManager` 入口残留。

## 实现方案

- 普通文件菜单命令改为：

```text
PasswordManager.exe --shell-action compress-archive "%1"
```

- `compress-archive` 动作中：
  - 如果传入路径是文件，打开该文件的父目录。
  - 如果传入路径是文件夹，打开该文件夹。
  - 实际仍调用内置 `tools/7zip/7zFM.exe`。

- 注册表删除：
  - 保留原 `QSettings::remove()`。
  - 在 Windows 下额外调用 `RegDeleteTreeW()` 删除同一棵注册表树。
  - 不存在的键视为删除成功。

## 验收

- 普通文件右键不会再出现 `Cannot open file ... as archive`。
- 普通文件右键会打开 7-Zip File Manager 到所在目录。
- 文件夹和压缩包右键原有行为不变。
- 设置页卸载右键菜单后不再残留 `PasswordManager` 入口。

