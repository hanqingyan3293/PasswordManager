# Phase 12 Plan - Shell Action Queue Flow

## Goal

Make the Windows context-menu action `add-test-queue` perform real work.
When a user right-clicks a supported archive and chooses the action, the app should scan/register that archive and enqueue password tests from the local plaintext password library.

## Scope

- Keep shell integration under current-user registry only.
- Keep supported extensions unchanged: `.zip`, `.rar`, `.7z`.
- Implement application-side handling for:
  - `--shell-action add-test-queue <archive>`
- Reuse existing archive scanning, password matching, and task queue modules.
- Add automated tests for the shell action service.

## Out of Scope

- GPU acceleration.
- Multi-threaded password testing.
- Registry schema changes.
- Installer changes.
- Password encryption.

## Acceptance Criteria

1. Unsupported or missing file paths do not enqueue tasks.
2. Supported archive paths are saved to the archive database.
3. Password candidates are generated from the existing password library.
4. One queue task is created for each unique candidate password.
5. Queued tasks include archive ID and password ID when available.
6. Existing automated tests still pass.

## Rollback

Revert Phase 12 source changes and delete `docs/28_Phase12_Plan.md` / `docs/29_Phase12_Report.md`.
