# Phase 10 Plan - Release Package and Regression Test

## Goal

Create a repeatable Windows portable release package and verify that it can run from the release directory without relying on tools outside the project.

## Scope

- Add a release packaging script.
- Package `PasswordManager.exe` with Qt runtime files through `windeployqt`.
- Copy bundled 7-Zip from `tools/7zip/` into the release directory.
- Create runtime directories in the release directory:
  - `data`
  - `config`
  - `logs`
  - `backup`
- Run full automated tests before release validation.
- Smoke-test the release executable and bundled 7-Zip.

## Locked Decisions

| Item | Decision | Reason |
| --- | --- | --- |
| Package type | Portable directory | Matches the current green portable route. |
| 7-Zip source | Copy only from project `tools/7zip/` | Prevents accidental use of system 7-Zip. |
| Qt deployment | Use Qt `windeployqt` | Official deployment tool for Qt runtime DLLs/plugins. |
| Output directory | `out/PasswordManager-portable/` | Keeps generated release files outside source and build directories. |
| Data storage | Plaintext SQLite | User explicitly confirmed no password encryption for this stage. |

## Release Layout

```text
out/PasswordManager-portable/
  PasswordManager.exe
  Qt runtime files
  platforms/
  sqldrivers/
  styles/
  tools/
    7zip/
      7z.exe
      7z.dll
      License.txt
      VERSION.txt
      SHA256SUMS.txt
  data/
  config/
  logs/
  backup/
  README_RELEASE.txt
```

## Verification

1. Build succeeds with `scripts/build_msvc.ps1`.
2. All CTest cases pass.
3. Packaging script creates `out/PasswordManager-portable/`.
4. Release directory contains `PasswordManager.exe`, Qt runtime, and `tools/7zip/7z.exe`.
5. `out/PasswordManager-portable/tools/7zip/7z.exe` starts and reports a version.
6. `out/PasswordManager-portable/PasswordManager.exe` starts from the release directory.

## Risks

| Risk | Mitigation |
| --- | --- |
| Missing Qt runtime file | Use `windeployqt` and smoke-test release executable. |
| Wrong 7-Zip source | Script copies explicit file list from project `tools/7zip/`. |
| Accidental delete outside project | Script refuses to clean directories outside project `out/`. |
| Release data mixed with source data | Runtime directories are created inside the release directory. |

## Rollback

Delete `out/PasswordManager-portable/`. Source code and test data are not modified by packaging.
