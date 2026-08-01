# Phase 35 Report - Explorer Context Menu Plan A And Controlled Extraction

## 已完成

- 将 Explorer 右键菜单改为方案A：根菜单使用 `ExtendedSubCommandsKey` 引用独立菜单类。
- 压缩包菜单类写入：
  - `HKCU\Software\Classes\PasswordManager.ArchiveMenu\Shell`
- 文件夹菜单类写入：
  - `HKCU\Software\Classes\PasswordManager.DirectoryMenu\Shell`
- 移除右键菜单里的 `添加到测试队列`。
- 新增压缩包右键 `解压`。
- 新增压缩包和文件夹右键 `打包压缩包`。
- 补入内置 `tools/7zip/7zG.exe`，并加入 Debug 构建复制、Release 打包复制和 SHA256 记录。
- `查看结果` 和 `自动查找密码` 改为只弹窗反馈，不打开主界面。
- `解压` 采用方案2：
  - 默认输出到压缩包同目录的同名文件夹。
  - 先尝试无密码解压。
  - 需要密码时先尝试已知成功密码。
  - 没有可用密码或已知密码失败时弹窗要求输入。
  - 非空密码解压成功后自动写入密码库和成功历史。
  - 解压结果写入解压日志。
- 主窗口最小尺寸下调到 `560 x 360`。
- 修复右键相关源码中的乱码字符串和损坏引号。
- 更新 `ShellIntegrationTests`，覆盖菜单类 Key、原生命令路径和 `7zG.exe` 命令。

## 当前右键命令

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

## 需要手动验证

更新发布包后进入 `设置`，点击 `重新安装/修复`，让程序删除旧注册表树并写入新的方案A结构。

然后在资源管理器测试：

1. 右键 `.zip/.rar/.7z`，确认 `压缩包密码管理器` 后面有 `>`。
2. 点击 `查看结果`，确认只弹窗，不打开主界面。
3. 点击 `自动查找密码`，确认有弹窗反馈。
4. 点击 `使用密码库测试`，确认打开主程序并进入测试队列。
5. 点击 `解压`，分别测试无密码压缩包、已知密码压缩包、未知密码压缩包。
6. 点击 `打包压缩包`，确认 7-Zip GUI 启动。
7. 右键文件夹，确认 `扫描文件夹`、`打包压缩包`、`打开主程序` 可用。

## 已验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。

- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过。

真实右键菜单仍需要在资源管理器里手动验证。
