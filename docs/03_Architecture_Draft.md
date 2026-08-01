# PasswordManager Architecture Draft

版本：v0.1
状态：架构草案

## 1. 分层结构

```text
UI Layer
  Qt Widgets pages, dialogs, tables, settings

Application Layer
  app startup, navigation, command routing, user actions

Domain Layer
  password ranking, archive identity, task state, extract policy

Service Layer
  database service, archive scanner, 7-Zip runner, task scheduler, logger

Infrastructure Layer
  SQLite, filesystem, process execution, config files, bundled tools
```

## 2. 核心模块

| 模块 | 职责 | 第一阶段状态 |
| --- | --- | --- |
| AppBootstrap | 初始化路径、日志、数据库、主窗口 | Phase 1 |
| PathService | 管理 data/config/logs/tools 路径 | Phase 1 |
| Logger | 写 app/error/archive/extract/database 日志 | Phase 1 |
| SettingsService | 读写配置 | Phase 1 |
| DatabaseService | SQLite 连接、迁移、备份入口 | Phase 2 |
| PasswordRepository | 密码 CRUD、分类、收藏、统计 | Phase 2 |
| ArchiveScanner | 扫描和识别 ZIP/RAR/7Z | Phase 3 |
| SevenZipRunner | 调用内置 7z.exe 并解析输出 | Phase 4 |
| TaskScheduler | 测试和解压任务队列 | Phase 5 |
| PasswordMatcher | 密码候选排序 | Phase 6 |
| ExtractService | 用户确认后的解压执行 | Phase 7 |
| ShellIntegration | Windows 右键菜单 | Phase 8 |

## 3. 依赖规则

允许：
- UI 调用 Application Layer。
- Application Layer 调用 Service 和 Domain。
- Service Layer 调用 Infrastructure。
- Domain Layer 不依赖 UI。

禁止：
- 数据库层直接操作 UI。
- 7-Zip 进程输出直接驱动界面。
- 任务线程直接修改 Widgets。
- 任意模块直接调用系统 PATH 中的 `7z.exe`。
- 未确认的功能绕过任务系统直接解压。

## 4. 密码测试数据流

```text
用户选择压缩包
  -> ArchiveScanner 识别文件
  -> DatabaseService 查询历史记录
  -> PasswordMatcher 生成候选密码
  -> TaskScheduler 创建测试任务
  -> SevenZipRunner 调用 tools/7zip/7z.exe
  -> 解析测试结果
  -> 成功则保存 archive_passwords
  -> UI 显示结果，等待用户确认是否解压
```

## 5. 错误流

```text
错误发生
  -> 返回结构化错误码
  -> 写入对应日志
  -> UI 显示可理解提示
  -> 任务进入 FAILED 或可恢复状态
```

错误码示例：
- `ERR_7ZIP_MISSING`
- `ERR_7ZIP_TIMEOUT`
- `ERR_ARCHIVE_UNSUPPORTED`
- `ERR_ARCHIVE_CORRUPTED`
- `ERR_PASSWORD_FAILED`
- `ERR_DATABASE_OPEN_FAILED`
- `ERR_USER_CANCELLED`

## 6. 后期扩展点

| 扩展点 | 用途 | 阶段 |
| --- | --- | --- |
| 密码提取规则 | 从本地说明文件提取密码 | Phase 6+ |
| 插件系统 | 后续高级扩展 | MVP 之后 |
| QML 页面 | 现代视觉升级 | Phase 9 |
| GPU / 多线程加速 | 性能专题 | Phase 11 |
| 右键菜单命令 | Windows 资源管理器集成 | Phase 8 |

