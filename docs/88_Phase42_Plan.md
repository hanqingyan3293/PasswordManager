# Phase 42 Plan - Registry Value Length Probe Fix

## 目标

- 修复安装/重新安装右键菜单后，设置页仍显示未安装或不完整的问题。
- 保持 Phase 41 的只读状态查询，不重新引入 `QSettings` 状态读取。

## 根因

Windows `RegGetValueW()` 在第一次仅查询值长度时，可能返回 `ERROR_MORE_DATA` 并给出所需缓冲区大小。原实现只接受 `ERROR_SUCCESS`，因此会把已经存在的注册表值误判为空。

## 实现方案

- `nativeRegistryStringValue()` 第一次调用 `RegGetValueW()` 时同时接受：
  - `ERROR_SUCCESS`
  - `ERROR_MORE_DATA`
- 后续仍用返回的 size 分配缓冲区，再读取真实字符串。

## 验收

- 安装/重新安装右键菜单后，设置页状态应显示正常。
- 卸载后状态刷新仍不创建空壳 key。
- 自动测试继续通过。

