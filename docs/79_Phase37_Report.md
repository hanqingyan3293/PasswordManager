# Phase 37 Report - Same Path Different Archive History Isolation

## 已完成

- `ArchiveRepository::upsert()` 增加 quickHash 变化检测。
- 同一路径已有记录但 `quick_hash` 变化时，会先删除该 archive_id 下旧的 `archive_passwords` 历史关联。
- 保留 `archives.path` 唯一键，首页仍显示当前路径对应的当前文件状态。
- 修复右键结果弹窗最近成功时间格式，使用 `yyyy-MM-dd HH:mm:ss`。
- `ArchivePasswordRepositoryTests` 增加覆盖：
  - 同路径旧 hash 记录旧密码。
  - upsert 新 hash。
  - archive_id 不变。
  - quickHash 更新。
  - 旧密码历史被清理。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256 校验：通过。
