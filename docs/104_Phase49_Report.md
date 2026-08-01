# Phase 49 Report - Full Hash Backfill Tool

## 已完成

- `ArchiveRepository` 新增 `listMissingFullHash()`。
- `ArchiveRepository` 新增 `updateFullHash()`。
- 新增 `ArchiveFingerprintService`，负责补全旧记录完整文件指纹。
- 设置页 `数据备份与恢复` 区域新增 `补全文件指纹` 按钮。
- 补全过程只更新 `archives.full_hash` 和 `scanned_at`，不改密码库、不改历史记录、不删除记录。
- 新增 `ArchiveFingerprintServiceTests`，覆盖文件存在补全和文件不存在计数。
- 修复空 `fullHash` 写库时被 Qt/SQLite 视为 NULL 的问题。

## 显示结果

补全完成后显示：

- 待补全记录数。
- 已补全数量。
- 文件不存在数量。
- 失败数量。
- 最后错误。

## 当前限制

- 当前实现为同步执行，大文件较多时设置页会等待完成。
- 后续可增加进度条、取消按钮、后台线程和并发控制。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：14/14 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：14/14 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256：`858ba93a493d10ee7aad93c74c5c94b451005a74ff3af4499102f267c49edf19`。
