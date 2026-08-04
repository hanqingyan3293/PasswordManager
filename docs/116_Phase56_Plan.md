# Phase56 体积/占用基线计划

日期：2026-08-04

## 目标

建立可复跑的发布包体积和基础启动耗时基线，为后续轻量优化提供数据，不靠主观判断。

## 范围

- 新增发布包测量脚本。
- 输出便携目录大小、zip 大小、exe 大小、文件数量、smoke-test 耗时。
- 按类别汇总 Application、Qt DLL、Graphics Runtime、MSVC Redistributable、7-Zip、Qt Plugins、Qt Translations 等体积。
- 列出最大的发布文件。
- 检查 QML/Quick 依赖残留。

## 不做

- 不删除 DLL。
- 不修改打包策略。
- 不做运行时内存监控。
- 不做多线程/GPU 优化。

## 验收

- 脚本可在当前发布包上运行。
- 生成 `out/release-size-baseline.md`。
- 报告能明确指出当前主要体积来源和 QML 残留状态。
