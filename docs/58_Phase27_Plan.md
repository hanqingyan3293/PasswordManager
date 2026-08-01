# Phase 27 Plan - Manual Test Learning and Empty Table Regression

## Goal

Fix two manual-test regressions without changing the broader UI structure:

- Empty tables must still show their column headers.
- A successful manually entered non-empty password must be added to the plaintext password library.

## Scope

- Home, password library, history, and task queue table empty states.
- Password repository lookup by plaintext password.
- Password test completion handling in the application entry point.
- Release packaging behavior for local portable runtime data.

## Rules

- Keep passwords plaintext as previously locked.
- Do not add empty passwords to the password library.
- Do not auto-extract after a successful test.
- Do not change archive scanning behavior.
- Keep the release ZIP clean; preserve local portable runtime data only after ZIP creation.

## Acceptance

- Empty table pages keep visible headers and resizable columns.
- Empty table hint text may appear below the table, but the table itself remains visible.
- Manual password test success with a new non-empty password adds one password-library record.
- Manual password test success with an existing password updates that existing record's success count instead of adding a duplicate.
- No-password archive tests still work and do not create empty password-library rows.
- Repackaging does not delete local `out/PasswordManager-portable/data`, `config`, `logs`, or `backup` contents used for manual testing.
