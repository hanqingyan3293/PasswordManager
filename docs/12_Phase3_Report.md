# Phase 3 Report

状态：已完成

## 完成内容

- 新增压缩包扫描器。
- 支持 ZIP/RAR/7Z 后缀识别。
- 支持单文件、多文件扫描。
- 支持文件夹递归扫描。
- 非支持格式会跳过。
- 新增快速 Hash。
- 新增 `archives` 表。
- 数据库 schema 从 1 升级到 2。
- 新增首页扫描页面，显示已记录压缩包。
- 新增扫描器 Qt Test。

## 快速 Hash

当前快速 Hash 不是完整文件 Hash：

- 文件小于等于 2 MiB 时读取全文件。
- 文件大于 2 MiB 时读取头部 1 MiB、尾部 1 MiB，并混入文件大小。

用途是快速识别候选压缩包，不用于安全校验。

## 数据表

### `archives`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `id` | INTEGER | 自增主键 |
| `path` | TEXT | 压缩包绝对路径，唯一 |
| `file_name` | TEXT | 文件名 |
| `extension` | TEXT | 扩展名 |
| `size_bytes` | INTEGER | 文件大小 |
| `modified_at` | TEXT | 文件修改时间 |
| `quick_hash` | TEXT | 快速 Hash |
| `scanned_at` | TEXT | 扫描时间 |

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
ctest --test-dir build --output-on-failure
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- `PasswordManagerTests` 通过。
- 程序启动和关闭成功。
- 数据库 `schema_version = 2`。
- `archives` 表已创建。

## 本阶段未做

- 未做完整 Hash。
- 未做 7-Zip 密码测试。
- 未做密码匹配。
- 未做历史关联保存。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 4：内置 7-Zip 集成。

建议范围：
- 固定调用 exe 相对路径 `tools/7zip/7z.exe`。
- 对选中的压缩包执行密码测试命令。
- 区分成功、密码错误、压缩包损坏、超时、7-Zip 缺失。
- 暂不做多线程任务队列，留给 Phase 5。

