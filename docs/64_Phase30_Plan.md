# Phase 30 Plan - Table Pagination and Numeric ID Sorting

## Goal

Improve large-table usability and fix ID column sorting.

## Scope

- Home archive table.
- Password library table.
- History table.
- Task queue table.

## Rules

- Keep existing table columns and actions.
- Keep manual refresh and automatic page-entry refresh.
- Do not change database schema.
- Pagination is UI-only.

## Acceptance

- Each main table can choose page size: 10, 20, 50, 100, 200, 500, or all.
- Each main table supports previous page, next page, and page-number input.
- Search or filter changes reset to page 1.
- Empty tables still show headers.
- ID columns sort as numbers, not as text.
