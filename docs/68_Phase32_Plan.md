# Phase 32 Plan - Explorer Menu Structure and Shell Actions

## Goal

Bring the Windows Explorer right-click integration closer to the original single-entry menu design, while only exposing actions that are already safe and implemented.

## Scope

- Keep one top-level menu entry: `压缩包密码管理器`.
- Add grouped submenus for archive files:
  - `密码测试`
  - `任务`
- Add implemented archive actions:
  - `自动查找密码`
  - `使用密码库测试`
  - `添加到测试队列`
  - `查看结果`
  - `打开主程序`
- Add folder right-click support:
  - `扫描文件夹`
  - `打开主程序`
- Add shell-action service coverage for known-password lookup and folder scanning.

## Out Of Scope

- `智能匹配测试并解压` is not exposed yet because automatic extraction flow still needs separate confirmation and output-directory rules.
- True Explorer multi-select batch command handling is not complete in this phase.
- Per-action right-click menu switches in settings are not complete in this phase.
- GPU and multithread acceleration remain a later topic.

## Acceptance

- Registry installation writes grouped menu entries under the current user only.
- Uninstall removes all registry entries written by this app.
- `自动查找密码` only looks up existing successful password history and shows a popup result.
- `使用密码库测试` queues password-library candidates but does not extract.
- Folder right-click scan records supported ZIP, RAR, and 7Z files.
- Automated tests cover lookup, folder scanning, and command generation without writing real registry entries.
