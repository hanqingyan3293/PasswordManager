# Phase 35 Plan - Explorer Context Menu Plan A And Controlled Extraction

## 目标

按确认后的方案A完成 Windows 资源管理器右键菜单：

- 使用 `ExtendedSubCommandsKey` 引用独立菜单类，避免根菜单点击无动作。
- 压缩包右键菜单只保留二级菜单，不再做三级菜单。
- 移除 `添加到测试队列`。
- 新增 `解压`，采用方案2：由 PasswordManager 控制解压流程。
- 新增 `打包压缩包`，直接调用内置 `7zG.exe`。

## 菜单结构

压缩包：

```text
压缩包密码管理器 >
├── 自动查找密码
├── 使用密码库测试
├── 查看结果
├── 解压
├── 打包压缩包
└── 打开主程序
```

文件夹：

```text
压缩包密码管理器 >
├── 扫描文件夹
├── 打包压缩包
└── 打开主程序
```

## 每项行为

- 自动查找密码：扫描并登记该压缩包，只弹窗显示是否存在已知成功密码。
- 使用密码库测试：打开主程序，将密码库候选加入测试队列，不解压。
- 查看结果：不打开主界面，只弹窗显示该压缩包的已知密码记录；没有记录时提示无已知密码。
- 解压：默认输出到压缩包同目录的同名文件夹。先尝试无密码；需要密码时先尝试已知密码；仍失败时要求用户输入。成功后记录解压日志，非空密码成功时写入密码库和历史关联。
- 打包压缩包：直接调用内置 `tools/7zip/7zG.exe`。
- 打开主程序：打开主窗口。
- 扫描文件夹：打开主程序并扫描该文件夹内支持的压缩包。

## 注册表方案

根菜单只引用菜单类：

```text
HKCU\Software\Classes\SystemFileAssociations\.zip\shell\PasswordManager
  MUIVerb = 压缩包密码管理器
  ExtendedSubCommandsKey = PasswordManager.ArchiveMenu

HKCU\Software\Classes\PasswordManager.ArchiveMenu\Shell\...
```

文件夹菜单：

```text
HKCU\Software\Classes\Directory\shell\PasswordManager
  MUIVerb = 压缩包密码管理器
  ExtendedSubCommandsKey = PasswordManager.DirectoryMenu

HKCU\Software\Classes\PasswordManager.DirectoryMenu\Shell\...
```

## 验收

- 右键根菜单必须显示 `>`。
- 点击二级菜单必须有可见动作或弹窗反馈。
- `查看结果` 不打开主界面。
- `打包压缩包` 使用本项目内置 `7zG.exe`。
- `解压` 能处理无密码、已知密码、手动输入密码三种路径。
- Debug 和 Release 测试通过。
