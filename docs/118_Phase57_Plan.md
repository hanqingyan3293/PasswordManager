# Phase57 轻量包方案计划

日期：2026-08-08

## 目标

在不影响默认完整发布包稳定性的前提下，提供一个可验证的轻量便携包方案，用数据评估体积优化收益。

## 范围

- 新增独立轻量包脚本。
- 从完整发布目录复制生成 `out/PasswordManager-lite-portable`。
- 生成独立 zip：`out/PasswordManager-0.1.0-win-x64-lite.zip`。
- 默认完整包不变。
- 轻量包只移除低风险可选项：
  - `vc_redist.x64.exe`。
  - 非 `qt_zh_CN.qm` 的 Qt 翻译。
  - 非 `qsqlite.dll` 的 SQL 驱动。

## 不做

- 不删除默认完整包中的文件。
- 不移除 `opengl32sw.dll`、`dxcompiler.dll`、`d3dcompiler_47.dll`、`dxil.dll`。
- 不移除 7-Zip GUI。
- 不改应用代码和业务功能。

## 验收

- 完整包仍可保留。
- 轻量包可生成。
- 轻量包 smoke-test 通过。
- 轻量包 zip 完整性通过。
- 生成轻量包体积基线，用于和完整包对比。
