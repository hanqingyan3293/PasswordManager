# Phase 19 Plan - Password Library CSV Import And Export

## Goal

Add plaintext password library import and export so users can move password records between local files and PasswordManager.

## Scope

- Add CSV export for password library records.
- Add CSV import for password library records.
- Use UTF-8 CSV.
- Support commas and quotes through standard CSV escaping.
- Keep password storage plaintext.
- Add UI actions on the password library page.
- Add automated tests for import and export.

## Format

CSV columns:

```text
password,category,note,favorite,success_count,failure_count
```

## Out Of Scope

- Encrypted export.
- Cloud sync.
- Excel `.xlsx` export.
- Deduplication rules.
- Importing task queue or archive history.
- GPU or multi-threaded acceleration.

## Acceptance

- Export creates a UTF-8 CSV file with header.
- Import accepts CSV files with the documented header.
- Empty password rows are skipped.
- Imported records appear in the password library.
- Debug and Release tests pass.
