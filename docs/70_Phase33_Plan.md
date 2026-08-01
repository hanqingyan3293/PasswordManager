# Phase 33 Plan - Context Menu Feedback and Window Size Fix

## Goal

Fix the right-click menu behavior reported in manual testing and make the main window easier to resize smaller.

## Scope

- Remove third-level Explorer menu structure.
- Keep one top-level entry: `压缩包密码管理器`.
- Put all implemented archive actions directly in the second-level menu.
- Put all implemented folder actions directly in the second-level menu.
- Show visible popup feedback for shell actions.
- Run shell actions after the main window is shown.
- Reduce default window size and minimum window size.

## Acceptance

- Archive right-click menu has no third-level submenu.
- Folder right-click menu has no third-level submenu.
- Clicking a shell action opens the app and shows a visible result popup.
- The main window can be dragged smaller than the previous 1120 x 720 starting size.
- Automated tests pass.
