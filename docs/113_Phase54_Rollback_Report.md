# Phase54 QML 试点回退报告

日期：2026-08-02

## 决策

QML 试点显示正常，但为了在保留所有功能的前提下尽可能降低发布包体积和运行时占用，当前阶段回退 QML 试点，继续采用纯 Qt Widgets 路线。

## 回退内容

- 移除首页 QML 概览面板。
- 移除 `HomeOverviewViewModel`。
- 移除 QML resource 和 `qml/HomeOverview.qml`。
- 移除 `Qt6::Qml`、`Qt6::Quick`、`Qt6::QuickWidgets` 链接依赖。
- 发布脚本恢复普通 `windeployqt --release --compiler-runtime`。

## 后续原则

- 数据密集页面继续保留 Widgets。
- 如果需要首页概览面板，优先用 Widgets 实现，避免为单个展示面板引入 Qt Quick 运行时。
- QML 只作为后续专题重新评估，不进入当前轻量发布路线。

## 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- ZIP 完整性测试：通过。
- 便携版 exe：`out/PasswordManager-portable/PasswordManager.exe`，更新时间 `2026-08-02 16:54:34`。
- ZIP SHA256：`9702db22aa77025ebbeab4f1d9193e98f541afa39de309c485665f1362a2e73e`。
- 便携版 zip：约 57 MiB。
- 发布目录没有 `Qt6Qml`、`Qt6Quick`、`Qt6QuickWidgets`、`Qt6OpenGL` 相关 DLL。
- 发布目录没有 `qml` 和 `qmltooling` 目录。
