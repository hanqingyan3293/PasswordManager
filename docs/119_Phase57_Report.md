# Phase57 轻量包方案报告

日期：2026-08-08

> 注意：本报告记录 Phase57 当时的实现。该版本轻量包后来发现会从正在使用的便携目录复制本机数据库、配置和日志，且依赖目标系统预装 VC++ Runtime。旧产物已作废，当前实现和验收结果以 Phase58 报告为准。

## 已完成

- 新增 `scripts/package_lite_release.ps1`。
- 从完整发布包生成独立轻量包，不改变默认完整包。
- 轻量包移除 `vc_redist.x64.exe`、非简体中文 Qt 翻译、非 SQLite SQL 驱动。
- 轻量包保留图形 fallback 文件，避免过早牺牲兼容性。
- 轻量包内新增 `LITE_PACKAGE_NOTES.txt`，说明兼容性要求和回退到完整包的建议。

## 输出

| 项目 | 路径 |
| --- | --- |
| 轻量目录 | `out/PasswordManager-lite-portable` |
| 轻量 zip | `out/PasswordManager-0.1.0-win-x64-lite.zip` |
| SHA256 | `out/PasswordManager-0.1.0-win-x64-lite.zip.sha256` |
| 轻量体积报告 | `out/release-size-lite-baseline.md` |

## 体积对比

| 项目 | 完整包 | 轻量包 | 节省 |
| --- | ---: | ---: | ---: |
| zip | 56.31 MiB | 30.58 MiB | 25.73 MiB |
| 便携目录 | 101.32 MiB | 71.74 MiB | 29.58 MiB |
| 文件数 | 111 | 78 | 33 |

完整包 SHA256：`3df40afd82f74b08d5586873e806172df9965e709da419881e48127443d33a24`。

轻量包 SHA256：`ae347a97432aeb61202844a3e179075ab31086a5f7a969f6bbdd241b370b8816`。

## 移除内容

- `vc_redist.x64.exe`。
- Qt 翻译只保留 `translations/qt_zh_CN.qm`。
- SQL 驱动只保留 `sqldrivers/qsqlite.dll`。

## 保留内容

- 保留 `opengl32sw.dll`。
- 保留 `dxcompiler.dll`、`d3dcompiler_47.dll`、`dxil.dll`。
- 保留完整内置 7-Zip，包括 `7zG.exe` 和 `7zFM.exe`。

## 验证

- 轻量包生成：通过。
- 轻量包 smoke-test：退出码 0。
- 轻量包 ZIP 完整性：通过。
- 轻量包体积基线生成：通过。
- QML/Quick 残留：0。

## 风险

- 轻量包依赖目标系统已有 Microsoft Visual C++ Runtime。
- 轻量包需要干净 Windows 机器或虚拟机验收后，才能作为默认推荐下载。
- 如果用户环境缺少运行库或图形兼容性异常，应使用完整包。
