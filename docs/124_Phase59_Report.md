# Phase59 干净 Windows 环境验收阶段报告

日期：2026-08-08

## 当前状态

验收工具、Windows Sandbox 配置和人工清单已经完成。当前开发机工具自测通过，但 Windows Sandbox 功能处于 `Disabled`，所以干净环境人工验收尚未执行，Phase59 暂不标记为最终验收完成。

## 新增文件

- `scripts/prepare_clean_environment_test.ps1`
- `scripts/run_clean_environment_acceptance.ps1`
- `docs/122_Phase59_Plan.md`
- `docs/123_Phase59_Manual_Checklist.md`
- `docs/124_Phase59_Report.md`

生成输出：

- `out/clean-environment-test/PasswordManager-Clean-Test.wsb`
- `out/clean-environment-test/input`
- `out/clean-environment-test/results`

每次运行会在 `results` 下生成独立的时间戳会话目录。准备脚本只重建 `input`，保留所有历史结果和已经填写的人工清单。

## 自动检查

自动验收脚本检查：

1. 64 位 Windows。
2. 轻量 ZIP 与 SHA256 sidecar 一致。
3. ZIP 根目录正确且无预置运行数据。
4. 5 个应用本地 VC++ Runtime DLL 齐全。
5. VC++ Runtime 清单和完整发布清单哈希正确。
6. 内置 7-Zip 可以运行。
7. 中文 Unicode 路径可以创建和使用。
8. 无密码压缩包、正确密码和错误密码场景符合预期。
9. `PasswordManager.exe --smoke-test` 通过。
10. GUI 可以保持运行。
11. GUI 进程实际加载的 5 个 VC++ Runtime DLL 全部来自应用目录。

## 本机工具自测

| 项目 | 结果 |
| --- | --- |
| PowerShell 5.1 语法 | 通过 |
| 脚本 ASCII 兼容 | 通过 |
| `.wsb` XML | 通过 |
| 验收套件生成 | 通过 |
| 自动检查 | 14/14 通过 |
| VC++ Runtime 实际加载路径 | 5/5 来自应用目录 |
| 临时工作目录清理 | 通过 |

本机系统为 Windows 10 x64，系统目录已经存在 5/5 VC++ Runtime DLL，并检测到开发工具，因此该结果只证明验收工具和应用本地加载路径有效，不能代替干净机器结论。

## Sandbox 隔离

- 输入映射目录：只读。
- 结果映射目录：可写。
- 网络：关闭。
- 虚拟 GPU：关闭。
- 剪贴板重定向：关闭。
- 程序先复制到 Sandbox 桌面临时目录，再启动。

## 下一验收门

1. 用户决定是否启用 Windows Sandbox，或提供另一台干净 Windows 10/11 x64 机器。
2. 运行 `out/clean-environment-test/PasswordManager-Clean-Test.wsb`。
3. 确认自动结果全部通过。
4. 按 `MANUAL_CHECKLIST.md` 完成界面、密码工作流、7-Zip 中文和右键菜单测试。
5. 将结果反馈后更新本报告，决定轻量包是否可标记为稳定推荐。

## 官方参考

- [Install Windows Sandbox](https://learn.microsoft.com/windows/security/application-security/application-isolation/windows-sandbox/windows-sandbox-install)
- [Use and configure Windows Sandbox](https://learn.microsoft.com/windows/security/application-security/application-isolation/windows-sandbox/windows-sandbox-configure-using-wsb-file)
