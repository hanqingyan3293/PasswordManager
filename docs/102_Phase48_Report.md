# Phase 48 Report - Full Hash File Fingerprint

## 已完成

- `ArchiveRecord` 新增 `fullHash`。
- `ArchiveScanner` 新增完整 SHA256 计算。
- `archives` 表新增 `full_hash` 字段，新库直接创建，旧库启动时自动 `ALTER TABLE` 迁移。
- `ArchiveRepository` 读写 `full_hash`。
- 同路径变化判断优先使用 `full_hash`，旧记录缺失时回退 `quick_hash`。
- `ArchivePasswordRepository` 新增 `listForFullHash()`。
- 智能匹配候选新增同 fullHash 历史层，位于当前压缩包历史之后、同目录历史之前。
- 首页表格 `快速 Hash` 改为 `文件指纹`，显示 fullHash 前 16 位。
- 测试覆盖 fullHash 扫描、跨路径 fullHash 历史查询、候选优先级顺序。

## 当前限制

- 已存在的旧记录只有重新扫描后才会写入 `full_hash`。
- 全哈希会读取完整文件，大文件扫描会比 quickHash 更慢。
- 多线程/GPU 加速仍留到后续单独讨论。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256：`f469a96b45425bff58dfaba6b8d20812a9ce0d221b9ad26cbe262446b4e61798`。
