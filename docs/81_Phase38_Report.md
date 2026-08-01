# Phase 38 Report - Right Click Coverage and Cleanup

## 已完成

- 普通文件右键新增 `压缩包密码管理器 > 打包压缩包`。
- 文件夹空白处右键新增 `压缩包密码管理器 > 打包压缩包 / 打开主程序`。
- 右键菜单继续使用方案 A：根菜单写入 `ExtendedSubCommandsKey`，二级动作写入独立菜单类。
- `打包压缩包` 继续调用本项目发布目录内的 `tools/7zip/7zFM.exe`，不调用系统其他位置的 7-Zip。
- 补入 `tools/7zip/Lang/zh-cn.txt`，并加入构建复制、发布复制和 SHA256 清单。
- 卸载右键菜单时额外清理历史残留根项，包括 `*`、`Folder`、`Directory\Background`、`CompressedFolder`、`.zip/.rar/.7z` 和 `zipfile/rarfile/7zfile` 下的旧 `PasswordManager`。
- `ShellIntegrationTests` 增加普通文件和文件夹空白处注册表路径、菜单类、压缩命令覆盖。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过，`4df42dd336d681ebb8338164081f1d7019b11fe2004bb334f8842c0e88c3895a`。

## 手动测试要求

更新发布包后，需要打开主程序进入 `设置`，点击 `重新安装/修复` 重新写入右键菜单。

重点检查：

- 普通文件右键显示 `压缩包密码管理器 > 打包压缩包`。
- 压缩包右键仍显示自动查找密码、使用密码库测试、查看结果、解压、打包压缩包、打开主程序。
- 文件夹右键显示扫描文件夹、打包压缩包、打开主程序。
- 文件夹空白处右键显示打包压缩包、打开主程序。
- 设置页点击 `卸载右键菜单` 后，不再残留失效的 `PasswordManager` 入口。
- 7-Zip 图形界面优先显示中文；如果本机 7-Zip GUI 仍保持英文，下一步再补 `7zFM.ini` 语言默认值。
