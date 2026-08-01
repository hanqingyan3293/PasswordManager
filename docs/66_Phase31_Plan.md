# Phase 31 Plan - Shell Registry Diagnostics and Repair

## Goal

Improve Windows Explorer context-menu reliability after portable builds are moved, replaced, or repaired.

## Scope

- Current-user HKCU shell integration only.
- Registry status diagnostics per supported extension.
- Repair/reinstall action from settings.
- Automated tests for command matching.

## Rules

- Do not write HKLM.
- Do not require administrator rights.
- Do not automatically install or repair registry entries on startup.
- Only write registry after explicit user confirmation in settings.
- Keep existing actions: add to test queue and view results.

## Acceptance

- Settings page shows whether `.zip`, `.rar`, and `.7z` entries are normal, incomplete, path-mismatched, or missing.
- Settings page can reinstall/repair entries so commands point to the current executable path.
- Existing install, uninstall, and refresh controls remain.
- Automated tests do not write real registry entries.
