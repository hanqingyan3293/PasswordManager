# PasswordManager v0.1.0 MVP Release Notes

## 下载

- `PasswordManager-0.1.0-win-x64-portable.zip`
- `PasswordManager-0.1.0-win-x64-portable.zip.sha256`
- `PasswordManager-0.1.0-win-x64-lite.zip`
- `PasswordManager-0.1.0-win-x64-lite.zip.sha256`

## 运行环境

- Windows x64。
- 便携版解压后运行 `PasswordManager.exe`。
- 发布包已包含 Qt 运行库、应用本地 VC++ Runtime 和项目内置 7-Zip。

## 已实现

- 本地 SQLite 密码库，密码按确认方案明文保存。
- ZIP/RAR/7Z 扫描。
- 内置 7-Zip 密码测试。
- 当前压缩包历史、同目录历史、密码库、说明文件的分层智能匹配。
- 历史记录、任务队列、分页表格和复制。
- 基础解压。
- Windows 资源管理器右键菜单。
- 内置 7-Zip 图形界面中文化。
- 压缩包分类候选和分层日志。
- Widgets 基础现代化样式。
- 纯 Widgets 首页概览。
- 可选轻量包。

## 注意事项

- 程序本地离线运行，不提供账号系统和云同步。
- 密码、导出文件、诊断包和数据库备份可能包含明文密码。
- 右键菜单需要在程序 `设置` 页安装或修复。
- GPU / 多线程加速未包含在 v0.1.0。
- 当前支持目标为 Windows 10/11 x64；轻量包不要求预先安装 Microsoft Visual C++ Runtime。
- 正式 ZIP 不包含开发机的数据库、配置、日志或备份文件。

## 校验

```text
SHA256: 73fc915225d3507a9023d9114482a31d25e9eb39fc9651bfbee8601acd7faa04
Lite SHA256: 9e97a891b7c8c8e4f77f3699c58b68eca1f38b820e8ddb435a772e491e9205bd
```
