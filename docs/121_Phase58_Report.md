# Phase58 轻量包发布可靠性报告

日期：2026-08-08

## 结果

Phase58 已完成确认范围内的三项工作：发布数据隔离与版本一致性、VC++ Runtime 依赖扫描、应用本地 VC++ Runtime 部署。

## 数据隔离修复

Phase57 轻量脚本直接复制 `out/PasswordManager-portable`。完整包脚本在生成干净 ZIP 后会恢复该目录中的本机运行数据，因此轻量包可能复制以下敏感文件：

- `data/passwordmanager.sqlite3`
- `config/settings.ini`
- `logs/*`
- `backup/*`

Phase58 将轻量包来源改为本次生成的完整包干净 ZIP。完整包和轻量包在压缩前都会检查四个运行目录，发现任何文件即停止打包。本机旧轻量 ZIP 已重新生成并覆盖。

新增 `scripts/verify_release_archive.ps1`。ZIP 生成后会自动解包，再次检查运行数据、EXE 哈希、VC++ Runtime 清单和完整发布清单，消除压缩前检查与实际归档之间的差异。

完整包和轻量包的本地运行数据恢复已移入 `finally` 失败路径。打包成功或失败都会尝试恢复测试数据库、配置和日志；只有恢复成功后才删除暂存副本。

验收结果：

| ZIP | 运行数据文件数 |
| --- | ---: |
| 完整包 | 0 |
| 轻量包 | 0 |

## 版本一致性

轻量脚本默认执行最新 Release 构建和完整打包，再解开完整 ZIP 生成轻量包。以下三处主程序必须具有相同 SHA256：

- `build-release/PasswordManager.exe`
- 完整 ZIP 内的 `PasswordManager.exe`
- 轻量 ZIP 内的 `PasswordManager.exe`

本次一致哈希：`921f8cb3385a05da918871e61fc7db7e5c7ab1e0d3ce455a585620e30b568546`。

## VC++ Runtime 依赖

新增 `scripts/deploy_vc_runtime.ps1`，使用 Visual Studio x64 `dumpbin` 扫描发布目录内全部 EXE/DLL，并递归完成运行库依赖闭包。文件只从 Visual Studio 官方 x64 Redistributable 目录复制。

本次依赖闭包：

| 文件 | 大小 |
| --- | ---: |
| `MSVCP140.dll` | 544.7 KiB |
| `MSVCP140_1.dll` | 35.1 KiB |
| `MSVCP140_2.dll` | 273.6 KiB |
| `VCRUNTIME140.dll` | 121.6 KiB |
| `VCRUNTIME140_1.dll` | 48.6 KiB |

合计约 1.00 MiB。两种发布包都包含这些应用本地 DLL 和 `VC_RUNTIME_MANIFEST.txt`。完整包额外保留 `vc_redist.x64.exe`，轻量包移除该安装器。

## 验证

- Debug 自动测试：14/14 通过。
- Release 构建：通过。
- 完整包与轻量包生成：通过。
- 完整包与轻量包 ZIP 完整性：通过。
- 完整包与轻量包 smoke-test：通过。
- ZIP 运行数据隔离：通过。
- VC++ Runtime 五文件完整性与 x64 架构：通过。
- 三处 EXE 哈希一致性：通过。
- QML/Quick 残留：0。

## 新体积

| 项目 | 完整包 | 轻量包 | 节省 |
| --- | ---: | ---: | ---: |
| ZIP | 56.69 MiB | 30.93 MiB | 25.76 MiB |
| 本地便携目录 | 102.32 MiB | 72.74 MiB | 29.58 MiB |

完整包 SHA256：`73fc915225d3507a9023d9114482a31d25e9eb39fc9651bfbee8601acd7faa04`。

轻量包 SHA256：`9e97a891b7c8c8e4f77f3699c58b68eca1f38b820e8ddb435a772e491e9205bd`。

本地便携目录体积包含为手动测试而保留的数据库、配置和日志；正式 ZIP 已确认不包含这些文件。

## 剩余边界

- 当前目标系统为 Windows 10/11 x64。
- 尚未执行干净 Windows 虚拟机验收，因此轻量包暂不取代完整包作为唯一下载。
- 图形 fallback 文件未调整。
