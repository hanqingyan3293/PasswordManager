# Phase58 轻量包发布可靠性计划

日期：2026-08-08

## 目标

修复轻量包可能携带本机运行数据或旧版程序的问题，并通过应用本地部署 Microsoft Visual C++ Runtime，使轻量包不再要求目标系统预先安装 VC++ 运行库。

## 范围

- 轻量包必须从本次生成的干净完整 ZIP 构建，不再复制正在使用的便携目录。
- 完整包和轻量包在压缩前必须检查 `data`、`config`、`logs`、`backup`，这些目录不得包含文件。
- ZIP 生成后必须自动解包复验运行数据隔离、发布清单和文件哈希。
- 构建产物、完整包和轻量包中的 `PasswordManager.exe` 必须具有相同 SHA256。
- 扫描发布目录内全部 EXE/DLL 的 MSVC Runtime 依赖。
- 从 Visual Studio 官方 x64 Redistributable 目录复制实际需要的运行库 DLL。
- 在发布包内生成 VC++ Runtime 清单。

## 明确不做

- 不删除图形兼容组件。
- 不调整 Qt 模块或业务功能。
- 不执行干净虚拟机验收。
- 不扩大 Windows 7 兼容范围；当前目标为 Windows 10/11 x64。

## 安全要求

- 正式 ZIP 不得包含 SQLite 数据库、配置、日志或备份文件。
- 本地便携目录中的运行数据必须在打包成功或失败后恢复；恢复失败时必须保留暂存副本。
- VC++ DLL 不得从 Windows 系统目录复制。
- 缺少依赖、架构不是 x64、版本或哈希不一致时必须停止打包。

## 验收

- Debug 测试通过。
- 完整包和轻量包均可生成。
- 两种包的 ZIP 完整性检查通过。
- 两种包的 smoke-test 退出码为 0。
- 两种正式 ZIP 的运行数据文件数均为 0。
- 三处 `PasswordManager.exe` SHA256 一致。
- 轻量包包含扫描得到的全部 VC++ Runtime DLL，且不包含 `vc_redist.x64.exe`。
