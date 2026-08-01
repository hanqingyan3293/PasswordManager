# Phase 28 Plan - Page Auto Refresh

## Goal

Reduce manual refresh operations after records change.

## Scope

- Main window page navigation.
- Home, password library, history, and task queue page reload entry points.

## Rules

- Do not change database schema.
- Do not add background polling.
- Do not change table layout or selection behavior.
- Keep refresh on page entry lightweight and explicit.

## Acceptance

- Opening the application refreshes the visible page after startup.
- Clicking into home, password library, history, or task queue refreshes that page automatically.
- Manual refresh buttons remain available.
- Existing task queue live refresh through task manager signals remains available.
