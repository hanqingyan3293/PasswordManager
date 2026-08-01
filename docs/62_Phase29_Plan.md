# Phase 29 Plan - Batch Matching, History Links, Compatibility, Help

## Goal

Continue after real-environment acceptance by improving batch work and daily-use navigation.

## Scope

- Home archive table multi-selection and batch smart matching.
- History page copy, password-library jump, and history deletion.
- Settings page shortcuts for manual/help/test-data locations.
- Release package user manual copy.
- Fixture compatibility baseline using bundled 7-Zip.

## Rules

- Manual single-password testing still uses the current selected archive only.
- Batch smart matching uses selected archive rows and existing password-library candidates.
- Deleting history must not delete password-library records.
- Compatibility testing uses bundled `tools/7zip/7z.exe`.
- No GPU or multithreaded acceleration in this phase.

## Acceptance

- Multiple archive rows can be selected on the home page.
- Batch smart matching asks for confirmation with archive count, candidate count, and total task count.
- History rows support copying password, copying row, jumping to password library, and deleting history.
- Settings page can open application directory, user manual, and test-data directory.
- Portable release includes `USER_MANUAL.md`.
- Existing automated tests and release validation pass.
- All local ZIP/7Z fixtures pass bundled 7-Zip compatibility testing.
