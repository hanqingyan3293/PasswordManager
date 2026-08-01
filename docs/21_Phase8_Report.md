# Phase 8 Report: Basic Extraction

状态：已完成

## 完成内容

- 新增 `ExtractService`。
- 固定调用 exe 相对目录下的 `tools/7zip/7z.exe`。
- 支持使用已保存成功密码解压单个压缩包。
- 历史记录页新增“解压”按钮。
- 解压前必须由用户选择输出目录。
- 解压前必须二次确认。
- 新增 `extract_logs` 表。
- 数据库 schema 升级到 4。
- 新增 `ExtractLogRepository`。
- 新增 `ExtractServiceTests`。

## 安全边界

- 不自动批量解压。
- 不修改原始压缩包。
- 不调用系统 PATH 中的 7-Zip。
- 只使用历史记录中已成功验证过的密码。

## 数据表

### `extract_logs`

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `id` | INTEGER | 自增主键 |
| `archive_id` | INTEGER | 压缩包 ID |
| `archive_path` | TEXT | 压缩包路径 |
| `output_directory` | TEXT | 输出目录 |
| `status` | TEXT | 解压状态 |
| `message` | TEXT | 解压消息 |
| `created_at` | TEXT | 创建时间 |

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
- 6 组自动化测试全部通过。
- 正确密码可解压测试压缩包。
- 错误密码返回失败。
- 程序启动和关闭成功。
- 数据库 `schema_version = 4`。
- 数据库包含 `extract_logs` 表。

## 本阶段未做

- 未做解压任务队列。
- 未做暂停/继续。
- 未做批量解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 9：Windows 右键菜单。

建议范围：
- 设置页提供右键菜单总开关。
- 支持安装/卸载注册表项。
- 先实现“添加测试队列”和“查看结果”入口。
- 注册表操作必须二次确认。

