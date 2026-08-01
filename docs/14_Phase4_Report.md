# Phase 4 Report

状态：已完成

## 完成内容

- 新增 `SevenZipRunner`。
- 固定调用 exe 相对目录下的 `tools/7zip/7z.exe`。
- 使用 `7z t -y -pPASSWORD ARCHIVE` 做密码测试。
- 区分成功、密码错误、压缩包错误、7-Zip 缺失、超时、进程错误。
- 首页支持对选中压缩包输入密码并测试。
- 新增真实压缩包测试资源。
- 新增 `SevenZipRunnerTests`。

## 测试资源

测试压缩包目录：

```text
testdata/archives/
```

生成过程和临时文件目录：

```text
testdata/tmp/
```

测试压缩包：

- 当前已扩充为 20 个 fixture。
- 命名规则：`fixture_XX_password_pm-fixture-XX.zip` 或 `.7z`。
- 密码规则：`pm-fixture-XX`。
- 详细清单见 `testdata/archives/README.md`。

## 验证结果

已执行：

```powershell
powershell -ExecutionPolicy Bypass -File scripts/configure_msvc.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1
ctest --test-dir build --output-on-failure
```

结果：

- CMake 配置成功。
- Ninja 编译成功。
- `PasswordManagerTests` 通过。
- `SevenZipRunnerTests` 通过。
- ZIP fixture 正确密码返回成功。
- ZIP fixture 错误密码返回密码错误。
- 7Z fixture 正确密码返回成功。
- 缺失 7-Zip 返回缺失状态。
- 程序启动和关闭成功。

## 本阶段未做

- 未做多线程任务队列。
- 未做批量密码测试。
- 未做智能密码匹配。
- 未做历史关联保存。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 5：任务系统。

建议范围：
- 建立 WAITING、RUNNING、PAUSED、COMPLETED、FAILED 状态。
- 单线程后台任务先行，避免 UI 卡死。
- 支持取消和超时。
- 暂缓复杂多线程并发策略，等任务模型稳定后再加。
