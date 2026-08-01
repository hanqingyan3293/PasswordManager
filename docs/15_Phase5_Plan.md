# Phase 5 Plan: Task System

状态：执行中

## 1. 目标

把同步密码测试升级为后台任务系统，避免 UI 被单次 7-Zip 测试阻塞。

## 2. 本阶段要做

- 建立测试任务模型。
- 支持状态：WAITING、RUNNING、COMPLETED、FAILED、CANCELLED、TIMEOUT。
- 使用后台 `QProcess` 执行单任务。
- 支持取消当前任务。
- 支持超时。
- 首页测试密码时加入任务队列。
- 解压队列页面显示任务列表。

## 3. 本阶段不做

- 不做多线程并发。
- 不做批量密码自动测试。
- 不做智能匹配。
- 不做历史关联保存。
- 不做解压。
- 不做右键菜单。
- 不做 GPU 加速。

## 4. 测试资源

测试压缩包固定放在：

```text
testdata/archives/
```

生成和临时文件固定放在：

```text
testdata/tmp/
```

当前保留 20 个测试压缩包，密码记录在 `testdata/archives/README.md`。

## 5. 验收标准

- CMake 配置成功。
- Ninja 编译成功。
- 自动化测试通过。
- 程序能启动和关闭。
- 首页测试密码会创建任务。
- 任务页能显示 WAITING/RUNNING/COMPLETED/FAILED 等状态。
- 正确密码任务完成为 COMPLETED。
- 错误密码任务完成为 FAILED。
- 任务可取消。

