# Phase 9 Report: Windows Shell Integration

状态：已完成

## 完成内容

- 新增 `ShellIntegration`。
- 使用 HKCU 当前用户注册表，不要求管理员权限。
- 支持 `.zip`、`.rar`、`.7z`。
- 设置页新增 Windows 右键菜单状态。
- 设置页新增安装、卸载、刷新按钮。
- 安装/卸载前必须二次确认。
- 右键菜单包含：
  - 添加测试队列。
  - 查看结果。
- 新增命令行入口 `--shell-action`。
- 新增 `ShellIntegrationTests`。

## 注册表范围

使用当前用户路径：

```text
HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.zip\shell\PasswordManager
HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.rar\shell\PasswordManager
HKEY_CURRENT_USER\Software\Classes\SystemFileAssociations\.7z\shell\PasswordManager
```

## 安全边界

- 不写 HKLM。
- 不要求管理员权限。
- 不自动批量解压。
- 自动化测试不真实写注册表。
- 只有用户在设置页确认后才安装/卸载右键菜单。

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
ctest --test-dir build --output-on-failure
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- 7 组自动化测试全部通过。
- 程序正常启动和关闭。
- 使用 `--shell-action view-results` 模拟右键菜单启动成功。

## 本阶段未做

- 未做管理员级右键菜单。
- 未做完整多功能二级菜单。
- 未做 ExplorerCommand COM 扩展。
- 未做右键菜单直接解压。
- 未做图标资源注册。

## 下一阶段建议

Phase 10：发布测试。

建议范围：
- 使用 `windeployqt` 收集 Qt 运行库。
- 创建绿色便携目录。
- 包含 `tools/7zip/`。
- 在发布目录内启动验证。
- 记录发布包结构。

