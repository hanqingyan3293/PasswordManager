# Phase 38 Plan - Right Click Coverage and Cleanup

## 目标

- 让 `打包压缩包` 不只出现在压缩包和文件夹右键中，也出现在普通文件右键中。
- 文件夹空白处右键增加 `压缩包密码管理器 >`，用于从当前目录打包或打开主程序。
- 内置 7-Zip GUI 补中文语言文件，发布包内不依赖系统 7-Zip。
- 卸载右键菜单时清理历史残留的 `PasswordManager` 注册表项，避免右键留下失效入口。

## 注册表结构

- 压缩包：`HKCU\Software\Classes\SystemFileAssociations\.zip/.rar/.7z\shell\PasswordManager`
- 普通文件：`HKCU\Software\Classes\*\shell\PasswordManager`
- 文件夹：`HKCU\Software\Classes\Directory\shell\PasswordManager`
- 文件夹空白处：`HKCU\Software\Classes\Directory\Background\shell\PasswordManager`
- 二级菜单类：`PasswordManager.ArchiveMenu`、`PasswordManager.FileMenu`、`PasswordManager.DirectoryMenu`、`PasswordManager.DirectoryBackgroundMenu`

## 菜单行为

普通文件右键：

```text
压缩包密码管理器 >
└── 打包压缩包
```

文件夹空白处右键：

```text
压缩包密码管理器 >
├── 打包压缩包
└── 打开主程序
```

`打包压缩包` 继续直接打开内置 `tools/7zip/7zFM.exe`，由用户在 7-Zip 图形界面中选择压缩参数。

## 验收

- Debug 构建通过。
- Debug CTest 通过。
- Release 发布包生成通过。
- Release CTest、smoke-test、benchmark、ZIP 完整性和 SHA256 校验通过。
- 手动测试时，在设置页点击 `重新安装/修复` 后验证普通文件、压缩包、文件夹、文件夹空白处右键菜单。

