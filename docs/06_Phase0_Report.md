# Phase 0 Report

状态：已完成

## 目标

- 初始化 Git 仓库。
- 建立 CMake + Qt Widgets 项目骨架。
- 建立基础运行目录。
- 准备内置 7-Zip 目录。

## 验收标准

- 项目包含 `CMakeLists.txt`。已完成。
- 项目包含最小 Qt Widgets 主窗口代码。已完成。
- 项目目录包含 `tools/7zip/`。已完成。
- 7-Zip 运行文件从官方来源放入 `tools/7zip/`。已完成。
- 当前机器未安装 CMake、Qt、MSVC 时，文档必须明确说明暂不能本机构建。环境已补齐并完成验证。

## 本次完成内容

- 初始化 Git 仓库。
- 创建 CMake + Qt Widgets 最小项目骨架。
- 创建主窗口、导航占位页、路径服务和日志服务。
- 创建内置 7-Zip 目录。
- 从 7-Zip 官方下载 Windows x64 MSI，并抽取 `7z.exe` 与 `7z.dll` 到 `tools/7zip/`。
- 记录 7-Zip 版本、来源和 SHA256。
- 安装并验证 CMake、Ninja、Qt 6.8.3。
- 完成 CMake 配置、编译和最小启动验证。

## 7-Zip 信息

- Version: 26.02
- Release Date: 2026-06-25
- Source Page: https://www.7-zip.org/download.html
- Download URL: https://www.7-zip.org/a/7z2602-x64.msi
- Runtime Path: `tools/7zip/7z.exe`

## 当前环境

- Visual Studio 2022 Community 17.14.34
- MSVC v143
- CMake 4.4.0
- Ninja 1.13.2
- Qt 6.8.3 `msvc2022_64`
- Bundled 7-Zip 26.02 x64

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/check_environment.ps1
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- 生成 `build/PasswordManager.exe`。
- 启动验证成功，日志写入 `build/logs/app.log`。

## 历史环境限制

初次检查时，本机普通 PowerShell 环境中未找到：

- `cmake`
- `ninja`
- `cl`
- `qmake`
- `qt-cmake`

该限制已通过安装 CMake、Ninja、Qt 6.8.3，并使用 Visual Studio Developer 环境脚本解决。后续构建命令：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```
