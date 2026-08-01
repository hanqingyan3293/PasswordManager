# Phase 45 Report - Local Description Password Candidates

## 已完成

- `PasswordMatcher` 新增说明文本密码提取。
- `PasswordMatcher` 新增压缩包同目录说明文件扫描。
- `PasswordMatcher::buildCandidates()` 支持密码库候选后追加额外候选，并保持去重和数量限制。
- 首页 `智能匹配测试` 改为按每个压缩包分别合并密码库候选和同目录说明文件候选。
- 右键 `使用密码库测试` 同样会读取该压缩包同目录说明文件候选。
- 说明文件候选不会直接写入密码库；只有实际测试成功后，现有成功流程才会记录。
- `PasswordMatcherTests` 新增说明文本解析、同目录说明文件读取、候选合并优先级测试。

## 解析范围

- 文件类型：`.txt`、`.md`、`.nfo`、`.url`。
- 文件位置：压缩包同目录。
- 文件大小：单文件不超过 256 KB。
- 文件名：压缩包同名说明文件优先，其次读取常见说明文件名。
- 规则：仅解析 `密码:`、`解压密码:`、`压缩包密码:`、`password:`、`pass:`、`pwd:` 等明确格式。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256：`e1dda6d42e70339d7dfc9195517603dce1a0d9eebf4a5c6cd957ebff89ea1897`。

## 手动测试要求

更新发布包后：

- 在压缩包同目录新建 `压缩包同名.txt`，内容写 `解压密码：实际密码`。
- 确认密码库没有该密码。
- 在首页扫描并选择该压缩包，点击 `智能匹配测试`，任务队列应包含说明文件候选。
- 右键压缩包点击 `使用密码库测试`，也应能加入说明文件候选测试。
