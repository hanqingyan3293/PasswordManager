# Phase 46 Report - Candidate Priority Formalization

## 已完成

- `PasswordMatcher` 新增 `buildLayeredCandidates()`。
- 候选顺序固化为：当前压缩包历史、同目录历史、密码库排序、说明文件候选。
- 历史候选按成功次数、最近成功时间、记录 ID 排序。
- 密码库候选继续按收藏、成功次数、失败次数、更新时间、ID 排序。
- `ArchivePasswordRepository` 新增 `listForArchive()`，用于按 `archive_id` 精确读取当前压缩包成功历史。
- 首页 `智能匹配测试` 接入历史关联仓库，按每个压缩包生成独立候选。
- 右键 `使用密码库测试` 接入同一套分层候选逻辑。
- 新增单元测试覆盖分层顺序和按 archiveId 精确读取。

## 当前限制

- “同分类候选”暂未实现，因为压缩包记录当前没有分类字段。
- “文件 fingerprint history”当前仍依赖同路径 quickHash 清理和当前 archiveId 历史；跨路径同 quickHash 查询需要后续新增仓库查询接口。

## 验证

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`：通过。
- `ctest --test-dir build --output-on-failure`：13/13 通过。
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`：通过。
- `ctest --test-dir build-release --output-on-failure`：13/13 通过。
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`：退出码 0。
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`：退出码 0。
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`：通过。
- 发布 ZIP SHA256：`8c089a5df46251d6267e621199c875b971959504148b2194fb85ce6058b04cf8`。
