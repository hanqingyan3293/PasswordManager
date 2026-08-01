# Phase 51 Report：日志系统完善与分类候选

## 已完成

- `AppLogger` 新增分层日志方法：
  - `archive()` 写入 `archive.log`。
  - `extract()` 写入 `extract.log`。
  - `database()` 写入 `database.log`。
- 程序启动、退出、数据库打开、右键动作、扫描、任务状态和解压结果增加日志。
- 密码测试任务日志只记录任务 ID、压缩包 ID、密码 ID、状态和路径，不记录明文密码。
- 诊断包继续导出日志目录下的 `*.log`，测试覆盖 5 类日志文件。
- `archives` 表新增 `category` 字段，schema_version 升级到 7，旧库启动时自动迁移。
- `ArchiveRecord` 和 `ArchiveRepository` 支持压缩包分类读写。
- 首页压缩包表格新增 `分类` 列。
- 首页新增 `设置分类`，支持对一个或多个选中压缩包批量设置分类；输入空值可清空分类。
- 设置页新增 `同分类密码候选` 开关。
- `PasswordRepository` 新增 `listByCategory()`。
- 智能匹配和右键 `使用密码库测试` 新增同分类密码候选层。

## 候选顺序

1. 当前压缩包历史。
2. 同 fullHash 历史。
3. 同目录历史。
4. 同分类密码。
5. 密码库全局排序。
6. 同目录说明文件。

## 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

```text
SHA256: bada9586ca220c0656e15eeb012e0f2036b32e38d85ac1f593546bba11638afd
```
