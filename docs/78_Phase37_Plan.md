# Phase 37 Plan - Same Path Different Archive History Isolation

## 目标

修复同一路径同名压缩包被重新创建后，旧密码历史仍显示在新压缩包上的问题。

## 原因

`archives.path` 是唯一键。同一路径重新扫描时会更新同一条 archive 记录，包括 `quick_hash`，但旧的 `archive_passwords` 仍挂在同一个 `archive_id` 下。

结果是：

- 旧压缩包密码仍显示在新压缩包上。
- 新压缩包解压成功后，新旧密码会同时显示。

## 方案

- `ArchiveRepository::upsert()` 在更新前读取旧记录。
- 如果同一路径存在记录，且旧 `quick_hash` 与新 `quick_hash` 不同：
  - 删除该 `archive_id` 下旧的 `archive_passwords`。
  - 再更新 `archives` 当前记录。
- 不改变 `archives.path` 唯一键，保持首页只显示当前路径的一份文件状态。

## 验收

- 同路径同名但内容不同的压缩包重新扫描后，旧密码历史不再显示。
- 同路径同名但内容相同的压缩包重新扫描后，历史不被清理。
- 右键 `自动查找密码` 和 `查看结果` 不再混入旧文件密码。
