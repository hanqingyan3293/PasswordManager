# Phase 50 Report：扫描性能与文件指纹模式

## 已完成

- `ArchiveScanner` 支持构造时选择是否计算 fullHash。
- `ScanResult` 新增扫描耗时和 fullHash 计算模式标记。
- 设置页新增 `扫描时计算完整文件指纹`，保存到 `config/settings.ini`。
- 首页扫描文件、扫描文件夹按该设置选择精确模式或快速模式。
- 右键扫描文件夹、右键压缩包动作按同一设置选择扫描模式。
- 扫描完成弹窗显示保存数量、跳过数量、耗时和模式。
- `ArchiveRepository` 在快速重复扫描时保留已有 fullHash，避免旧完整指纹被空值覆盖。
- 新增测试覆盖快速扫描跳过 fullHash、快速重复扫描保留已有 fullHash。

## 用户影响

- 默认仍是精确模式，行为和 Phase 49 一致。
- 如果关闭 `扫描时计算完整文件指纹`，大文件扫描会更快，但新记录的 `文件指纹` 可能为空。
- 快速模式下同 fullHash 的跨路径历史匹配可能无法命中，直到重新开启精确扫描或执行 `补全文件指纹`。

## 验证

- Debug 构建：通过。
- Debug CTest：14/14 通过。
- Release 打包：通过。
- Release CTest：14/14 通过。
- smoke-test：退出码 0。
- benchmark：退出码 0。
- ZIP 完整性和 SHA256 校验：通过。

```text
SHA256: a9cce789d57ab33770f5b18606ab0e8c73bba84c9817e9ea80506eed43ccd4be
```
