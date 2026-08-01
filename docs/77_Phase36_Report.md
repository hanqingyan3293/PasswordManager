# Phase 36 Report - Context Menu Corrections

## 已完成

- `打包压缩包` 改为调用内置 `tools/7zip/7zFM.exe` 打开 7-Zip 图形界面，不再直接生成 `.7z`。
- 补入 `tools/7zip/7zFM.exe`，并加入构建复制、发布复制和 SHA256 记录。
- `SevenZipRunner` 增加无密码预检：
  - 对无密码压缩包测试任意非空密码时返回 `NO_PASSWORD_REQUIRED`。
  - 空密码测试仍可正常验证无密码压缩包。
- `PasswordTestTaskManager` 增加 `NoPasswordRequired` 任务结果，避免无密码压缩包把候选密码写成正确密码。
- 首页 `智能匹配测试` 会跳过无需密码的压缩包。
- 右键 `使用密码库测试` 会跳过无需密码的压缩包。
- 密码库和历史记录回写只在 `Completed + Success + 非空密码` 时执行。
- `自动查找密码` / `查看结果` 对无密码压缩包提示“该压缩包无需密码”。
- 密码记录弹窗最多显示前 5 条，并提示完整记录去主程序查看。
- 右键 `解压` 改为先测试密码，测试通过后再解压；解压超时提高到 10 分钟。
- 增加单实例转发：
  - 已有主窗口时，`打开主程序`、`使用密码库测试`、`扫描文件夹` 会转发给旧窗口，不再保留新窗口。
- 自动化测试增加：
  - 无密码压缩包传入任意密码时返回 `NO_PASSWORD_REQUIRED`。
  - 无密码压缩包不会通过右键密码库测试加入候选任务。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过。

## 需要手动验证

进入 `设置` 点击 `重新安装/修复`，刷新右键注册表后测试：

1. 无密码压缩包：右键 `使用密码库测试` 不应写入任何密码成功记录。
2. 无密码压缩包：右键 `自动查找密码` / `查看结果` 应提示无需密码。
3. 已有主窗口时：右键 `使用密码库测试` 应调出旧窗口，不新开窗口。
4. 已有主窗口时：右键 `打开主程序` 应调出旧窗口。
5. 右键 `打包压缩包` 应打开 7-Zip 图形界面。
6. 带密码压缩包：右键 `解压` 输入正确密码后应正常完成，且不再出现“已解出但提示失败”的情况。
