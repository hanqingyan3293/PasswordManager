# Phase 22 Plan - Import Deduplication And Validation

## Goal

Make plaintext CSV import safer by reporting skipped rows and avoiding duplicate password records.

## Scope

- Skip rows with empty passwords.
- Skip passwords that already exist in the local password library.
- Skip duplicate passwords within the same imported CSV.
- Report imported, skipped, duplicate, and invalid row counts.
- Keep import behavior append-only for valid non-duplicate rows.

## Out Of Scope

- Interactive import preview.
- Merging duplicate metadata.
- Encrypted import.
- Excel `.xlsx` import.
- Archive/history import.

## Acceptance

- Existing passwords are not imported again.
- Duplicate rows inside the same file are skipped.
- Empty-password rows are skipped.
- UI reports row counts after import.
- Automated tests cover the behavior.
