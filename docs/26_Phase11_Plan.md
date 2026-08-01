# Phase 11 Plan - Release Archive Polish

## Goal

Turn the portable release directory into a versioned distributable archive with checksum evidence.

## Scope

- Keep `out/PasswordManager-portable/` as the unpacked portable release directory.
- Create a versioned ZIP archive under `out/`.
- Create SHA256 checksum for the ZIP archive.
- Create `RELEASE_MANIFEST.txt` inside the portable directory with per-file SHA256 hashes.
- Keep release packaging repeatable through `scripts/package_release.ps1`.

## Locked Decisions

| Item | Decision | Reason |
| --- | --- | --- |
| Archive format | ZIP | Built into Windows and PowerShell, no extra packaging dependency. |
| Archive name | `PasswordManager-<version>-win-x64-portable.zip` | Makes version, platform, architecture, and package type clear. |
| Checksum | SHA256 | Standard integrity check for release files. |
| Output location | `out/` only | Generated artifacts stay outside source and build folders. |

## Validation

1. Release script completes without errors.
2. `out/PasswordManager-portable/RELEASE_MANIFEST.txt` exists.
3. `out/PasswordManager-0.1.0-win-x64-portable.zip` exists.
4. `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256` exists.
5. ZIP can be listed with bundled 7-Zip.
6. Release smoke test exits with code 0.
7. Debug and Release CTest suites pass.

## Rollback

Delete generated files under `out/`. Source, build, and test data are not affected.
