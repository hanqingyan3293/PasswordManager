# Phase 1 Report

状态：已完成

## 完成内容

- 主窗口升级为稳定的 Qt Widgets 基础框架。
- 保留首页、密码库、历史记录、解压队列、设置五个导航页。
- 新增设置页，显示程序目录、数据目录、配置目录、日志目录、备份目录、工具目录。
- 新增内置 7-Zip 状态检查，固定检查 exe 相对路径下的 `tools/7zip/7z.exe`。
- 新增 7-Zip 版本读取。
- 新增构建后复制规则，把运行必需的 7-Zip 文件复制到 `build/tools/7zip/`。
- 增加基础 Qt Widgets 样式和状态栏。

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- `build/PasswordManager.exe` 启动和关闭成功。
- `build/logs/app.log` 写入启动和关闭日志。
- `build/tools/7zip/7z.exe` 可运行，版本为 7-Zip 26.02 x64。
- 临时移除 `build/tools/7zip/7z.exe` 后，程序仍可启动和关闭，不崩溃。

## 本阶段未做

- 未做密码库 CRUD。
- 未做 SQLite 表结构。
- 未做压缩包扫描。
- 未做密码测试。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。
- 未做 QML 页面。

## 下一阶段建议

Phase 2：SQLite 与密码库。

开始前需要先确认密码库安全策略：

1. 明文 SQLite：开发最快，但不适合真实保存敏感密码。
2. Windows DPAPI 加密：适合本机离线工具，能绑定当前 Windows 用户。
3. 主密码加密：跨机器迁移更好，但需要设计主密码、恢复和遗忘处理。

