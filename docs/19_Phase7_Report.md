# Phase 7 Report: History Association and Stats

状态：已完成

## 完成内容

- 新增 `archive_passwords` 表。
- 数据库 schema 升级到 3。
- 新增 `ArchivePasswordRepository`。
- 新增 `ArchivePasswordRecord`。
- 任务完成后发出 `taskFinished` 信号。
- 智能匹配任务携带 `archive_id` 和 `password_id`。
- 正确密码任务完成后保存压缩包与密码关联。
- 密码库记录成功/失败次数自动回写。
- 历史记录页替换为真实表格。
- 新增 `ArchivePasswordRepositoryTests`。

## 数据表

### `archive_passwords`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `id` | INTEGER | 自增主键 |
| `archive_id` | INTEGER | 压缩包 ID |
| `password_id` | INTEGER | 密码库 ID，可为空 |
| `password` | TEXT | 成功密码明文 |
| `success_count` | INTEGER | 该压缩包使用该密码成功次数 |
| `last_success_at` | TEXT | 最近成功时间 |
| `created_at` | TEXT | 创建时间 |
| `updated_at` | TEXT | 更新时间 |

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
- 5 组自动化测试全部通过。
- 程序启动和关闭成功。
- 数据库 `schema_version = 3`。
- 数据库包含 `archive_passwords` 表。

## 本阶段未做

- 未做解压。
- 未做解压日志。
- 未做说明文件提取密码。
- 未做同目录/同资源组匹配。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 8：基础解压。

建议范围：
- 用户选中已有关联密码的压缩包。
- 用户确认输出目录。
- 使用内置 7-Zip 解压。
- 不自动批量解压。
- 记录解压结果。

