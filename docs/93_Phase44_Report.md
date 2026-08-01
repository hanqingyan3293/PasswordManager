# Phase 44 Report - 7-Zip GUI Chinese Language

## 已完成

- `AppPaths` 新增 `sevenZipChineseLanguageFile()`。
- `AppPaths` 新增 `ensureSevenZipChineseLanguage()`，用于设置 7-Zip GUI 语言。
- 主程序启动后自动设置 `HKCU\Software\7-Zip\Lang = zh-cn`。
- 普通文件右键 `compress-archive` 启动 7zFM 前会再次确保语言设置。
- 保持 7-Zip 可执行文件仍来自项目内置 `tools/7zip/`。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`26e7427f95ae17adde863e9b555f9f6cae1da0224524cc30c2baefbf6279cd80`。

## 手动测试要求

更新发布包后：

- 先打开一次 PasswordManager 主程序。
- 从普通文件、文件夹或压缩包右键点击 `打包压缩包`。
- 内置 7-Zip File Manager 应显示中文界面。
