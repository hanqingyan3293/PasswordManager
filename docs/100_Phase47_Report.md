# Phase 47 Report - Feature-Level Settings

## 已完成

- 新增 `AppConfig`，配置保存到 `config/settings.ini`。
- 设置页新增 `智能匹配` 区域。
- 设置页新增 `右键菜单功能项` 区域。
- 首页 `智能匹配测试` 按设置启用或禁用候选来源。
- 右键 `使用密码库测试` 按同一份智能匹配设置生成候选。
- 说明文件读取支持配置候选数量和单文件读取大小。
- 右键菜单安装/修复按设置写入菜单项。
- 右键菜单状态检测按当前功能项设置判断，避免禁用项被误判为安装不完整。
- 发布脚本会把 `LICENSE` 放入便携包。

## 默认值

- 当前压缩包历史优先：开启。
- 同目录历史候选：开启。
- 密码库候选：开启。
- 同目录说明文件候选：开启。
- 最大候选数：100。
- 说明文件最大候选数：20。
- 单个说明文件读取上限：256 KB。
- 右键菜单功能项：默认全部开启。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布包 `LICENSE`：存在。
- 发布 ZIP SHA256：`fd5c86cce8b78d46f9e7f027b781715289bd34a50ed4591951e2e50901c9a359`。

## 手动测试要求

- 打开设置页，修改智能匹配开关和数量，点击 `保存功能设置`。
- 重启程序后确认设置仍保留。
- 关闭 `同目录说明文件候选` 后，说明文件中的密码不应进入智能匹配队列。
- 关闭某个右键菜单功能项后，点击 `保存功能设置`，再执行 `重新安装/修复`，右键菜单中该项应消失。
