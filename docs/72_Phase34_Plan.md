# Phase 34 Plan - Explorer Cascading Menu Registry Fix

## Goal

Fix the Windows Explorer right-click menu so submenu items actually launch `PasswordManager.exe --shell-action ...`.

## Root Cause

The previous registry layout used an empty `SubCommands` value with child commands under `PasswordManager\shell\...\command`.
That layout can display a cascading menu on some Windows versions, but it is not the documented structure for executable submenu items.

## Scope

- Replace the `SubCommands="" + shell` layout with `ExtendedSubCommandsKey\Shell`.
- Keep the current-user HKCU install route.
- Keep the two-level menu:
  - `压缩包密码管理器 > 自动查找密码`
  - `压缩包密码管理器 > 使用密码库测试`
  - `压缩包密码管理器 > 添加到测试队列`
  - `压缩包密码管理器 > 查看结果`
  - `压缩包密码管理器 > 打开主程序`
- Keep folder actions:
  - `压缩包密码管理器 > 扫描文件夹`
  - `压缩包密码管理器 > 打开主程序`
- Write executable paths with native Windows separators.
- Keep shell-action logging and foreground handling.

## Acceptance

- Registry entries are written under `ExtendedSubCommandsKey\Shell`.
- Commands point to the current executable path.
- Automated tests pass.
- User can repair/reinstall the menu from settings and manually test Explorer actions.
