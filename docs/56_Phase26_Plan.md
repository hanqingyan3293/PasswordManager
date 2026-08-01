# Phase 26 Plan - Manual Password Test Guardrails

## Goal

Fix the home-page password test flow found during real-environment testing.

## Scope

- Allow explicit empty-password testing for archives without passwords.
- Require confirmation before empty-password testing.
- Require confirmation before smart matching queues many automatic candidate tasks.
- Update tests for no-password archives.

## Out Of Scope

- Removing smart matching.
- Changing queued task persistence.
- Changing password plaintext storage.
- GPU or multi-threading.

## Acceptance

- A selected no-password archive can be tested with an empty password.
- Empty password testing is intentional through a confirmation dialog.
- Smart matching does not queue tasks until the user confirms.
- Automated tests cover no-password archive testing.
