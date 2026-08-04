# Phase56 体积/占用基线报告

日期：2026-08-04

## 已完成

- 新增 `scripts/measure_release.ps1`。
- 生成 `out/release-size-baseline.md`。
- 记录发布包摘要、分类体积、最大文件列表和轻量路线检查。
- 确认当前发布包没有 QML/Quick 运行时残留。

## 当前基线

| 项目 | 数值 |
| --- | ---: |
| 便携目录 | 101.32 MiB |
| zip 包 | 56.31 MiB |
| PasswordManager.exe | 532.00 KiB |
| 文件数 | 111 |
| smoke-test | 44 ms |
| SHA256 | `cd6036d706096b86c22d3aa9f6758c53d797aded023064f221bb0e917fee004e` |

## 主要体积来源

| 分类 | 大小 | 占比 |
| --- | ---: | ---: |
| Graphics Runtime | 39.30 MiB | 38.8% |
| MSVC Redistributable | 24.45 MiB | 24.1% |
| Qt DLL | 23.40 MiB | 23.1% |
| Qt Translations | 4.96 MiB | 4.9% |
| Qt Plugins | 4.51 MiB | 4.5% |
| 7-Zip | 4.02 MiB | 4.0% |

## 最大文件

| 文件 | 大小 |
| --- | ---: |
| `vc_redist.x64.exe` | 24.45 MiB |
| `opengl32sw.dll` | 19.68 MiB |
| `dxcompiler.dll` | 13.65 MiB |
| `Qt6Gui.dll` | 8.86 MiB |
| `Qt6Widgets.dll` | 6.20 MiB |
| `Qt6Core.dll` | 5.87 MiB |
| `d3dcompiler_47.dll` | 4.52 MiB |

## 验证

- 测量脚本运行：通过。
- `out/release-size-baseline.md` 生成：通过。
- QML/Quick DLL 残留：0。
- `qml` / `qmltooling` 目录残留：0。

## 下一步建议

- Phase57 可先评估 `vc_redist.x64.exe` 是否改为安装说明或独立可选包。
- 评估 Qt translations 是否按语言白名单保留。
- 评估 DirectX/OpenGL fallback 文件是否能按目标机器策略拆分为可选组件。
- 任何删除都必须在干净 Windows 机器或虚拟机验证。
