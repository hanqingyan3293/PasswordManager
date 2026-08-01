# Phase 6 Report

状态：已完成

## 完成内容

- 新增 `PasswordMatcher`。
- 从密码库记录生成候选密码。
- 过滤空密码。
- 对候选密码去重。
- 按优先级排序：
  1. 收藏密码优先。
  2. 成功次数高优先。
  3. 失败次数低优先。
  4. 更新时间新优先。
  5. ID 大优先。
- 首页新增“智能匹配测试”按钮。
- 对选中压缩包批量创建测试任务。
- 新增 `PasswordMatcherTests`。

## 当前匹配范围

已实现：
- 收藏密码。
- 成功次数。
- 失败次数。
- 更新时间。
- 去重。
- 候选数量限制。

未实现：
- 精确历史成功密码。
- 文件指纹历史密码。
- 同目录/同资源组密码。
- 分类与压缩包关联。
- 说明文件提取密码。
- 人工输入自动学习。

这些需要在后续“历史关联保存”和“高级匹配”中继续补齐。

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
- `PasswordMatcherTests` 通过。
- `PasswordTestTaskManagerTests` 通过。
- 程序启动和关闭成功。

## 本阶段未做

- 未做历史关联保存。
- 未做成功/失败次数自动回写。
- 未做压缩包与分类自动关联。
- 未做说明文件提取密码。
- 未做解压。
- 未做右键菜单。
- 未做 GPU 加速。

## 下一阶段建议

Phase 7：历史关联保存与基础解压前置。

建议先做：
- `archive_passwords` 表。
- 任务成功后保存压缩包与密码关联。
- 密码成功/失败次数自动更新。
- 历史记录页面显示关联结果。

然后再进入基础解压。

