# Phase 13 Report - Shell View Results Filtering

## Completed

- Added `HistoryPage::focusArchivePath()`.
- `--shell-action view-results <archive>` now:
  - opens the history page
  - sets the history search box to the selected archive's absolute path
  - reloads filtered history records
  - selects the first matching row when a match exists
- Added repository test coverage for filtering history by archive path.
- Rebuilt and repackaged the portable release.

## Behavior

```powershell
PasswordManager.exe --shell-action view-results "C:\path\archive.zip"
```

Expected result:

1. The app opens the history page.
2. History is filtered to the selected archive path.
3. If a successful password record exists for that archive, the first matching row is selected.

## Validation Results

| Check | Result |
| --- | --- |
| Debug CTest in `build/` | Passed, 8/8 |
| Release package build | Passed |
| Release CTest in `build-release/` | Passed, 8/8 |
| Release smoke test | Passed, exit code 0 |
| ZIP integrity test | Passed |

## Notes

- Registry entries did not change.
- Task queue persistence is still not implemented.
- GPU and multi-thread acceleration remain future topics.
