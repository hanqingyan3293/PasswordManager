# Phase 5 Report

状态：已完成

## 完成内容

- 新增内存任务模型。
- 新增 `PasswordTestTaskManager`。
- 支持状态：
  - WAITING
  - RUNNING
  - COMPLETED
  - FAILED
  - CANCELLED
  - TIMEOUT
- 使用后台 `QProcess` 执行 7-Zip 密码测试。
- 首页“测试密码”改为加入任务队列，不再同步阻塞 UI。
- 解压队列页面替换为任务队列页面。
- 支持取消 WAITING 和 RUNNING 任务。
- 新增 `PasswordTestTaskManagerTests`。
- 测试压缩包扩充为 20 个。

## 测试资源

测试压缩包目录：

```text
testdata/archives/
```

生成和临时文件目录：

```text
testdata/tmp/
```

当前测试压缩包数量：20。

命名规则：

```text
fixture_01_password_pm-fixture-01.zip
fixture_02_password_pm-fixture-02.7z
...
fixture_20_password_pm-fixture-20.7z
```

密码记录在：

```text
testdata/archives/README.md
```

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
- `PasswordTestTaskManagerTests` 通过。
- 程序启动和关闭成功。
- `testdata/archives/` 中确认有 20 个测试压缩包。

## 本阶段未做

- 未做多线程并发。
- 未做批量密码自动测试。
- 未做智能匹配。
- 未做历史关联保存。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 6：智能匹配。

建议范围：
- 从密码库读取候选密码。
- 按优先级生成候选列表。
- 对选中压缩包创建一组 WAITING 测试任务。
- 成功后暂只显示结果，历史关联保存可以放到 Phase 6 后半段或 Phase 7 前。

