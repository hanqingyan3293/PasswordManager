# Phase 13 Plan - Shell View Results Filtering

## Goal

Make the Windows context-menu action `view-results` open the history page filtered to the selected archive path.

## Scope

- Implement application-side handling for:
  - `--shell-action view-results <archive>`
- Reuse the existing history page search/filter behavior.
- Add focused repository test coverage for archive-path filtering.

## Out of Scope

- Registry changes.
- Task queue persistence.
- GPU acceleration.
- Multi-threaded password testing.
- Password encryption.

## Acceptance Criteria

1. `view-results` opens the history page.
2. The history page search box is set to the absolute archive path.
3. History records are filtered to that archive path.
4. The first matching row is selected when matches exist.
5. Existing automated tests still pass.

## Rollback

Revert the Phase 13 changes in `HistoryPage`, `MainWindow`, and repository tests, then delete the Phase 13 docs.
