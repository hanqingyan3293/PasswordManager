# Phase 23 Plan - User Experience Polish

## Goal

Improve real-environment testing ergonomics without changing core data behavior.

## Scope

- Add clear empty-state text for major tables.
- Enable table column sorting.
- Keep selected actions correct after sorting by resolving records through table IDs.
- Add settings-page buttons to open data, backup, and log directories.

## Out Of Scope

- New business logic.
- Database schema changes.
- Password encryption.
- GPU or multi-threading.

## Acceptance

- Empty tables show Chinese guidance.
- Users can sort visible tables by clicking headers.
- Editing, deleting, extracting, and testing still target the correct selected record after sorting.
- Settings page can open data, backup, and log folders.
