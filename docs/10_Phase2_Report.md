# Phase 2 Report

状态：已完成

## 已锁定决策

密码存储策略：明文 SQLite，不加密。

安全影响：
- `data/passwordmanager.sqlite3` 中的密码可被直接读取。
- 备份、复制、同步该数据库文件等同于复制密码库。
- 后续如果改为 DPAPI 或主密码加密，需要做迁移。

## 完成内容

- 新增 SQLite 数据库服务。
- 首次启动自动创建 `data/passwordmanager.sqlite3`。
- 新增 `database_info` 表，当前 `schema_version = 1`。
- 新增 `passwords` 表。
- 新增密码仓库，支持列表、搜索、新增、编辑、删除。
- 新增密码库页面，支持搜索框、表格、新增、编辑、删除、刷新。
- 主窗口接入真实密码库页面。

## 数据表

### `database_info`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `key` | TEXT | 主键 |
| `value` | TEXT | 配置值 |

### `passwords`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `id` | INTEGER | 自增主键 |
| `password` | TEXT | 明文密码 |
| `category` | TEXT | 分类 |
| `note` | TEXT | 备注 |
| `favorite` | INTEGER | 是否收藏 |
| `success_count` | INTEGER | 成功次数 |
| `failure_count` | INTEGER | 失败次数 |
| `created_at` | TEXT | 创建时间 |
| `updated_at` | TEXT | 更新时间 |

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- 程序启动和关闭成功。
- 首次启动创建 `build/data/passwordmanager.sqlite3`。
- 数据库包含 `database_info`、`passwords`、`sqlite_sequence`。
- `schema_version = 1`。
- 临时写入的密码记录在程序重启后仍存在。
- 临时测试记录已清理。

## 本阶段未做

- 未做密码加密。
- 未做 JSON 导入导出。
- 未做压缩包扫描。
- 未做 7-Zip 密码测试。
- 未做历史关联保存。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 3：文件扫描。

建议范围：
- 单文件选择。
- 多文件选择。
- 文件夹递归扫描。
- ZIP/RAR/7Z 后缀识别。
- 文件名、大小、修改时间、快速 Hash。
- 暂不做完整 Hash，作为可选项保留。

