# Phase 10 Report - Release Package and Regression Test

## Completed

- Added repeatable release script: `scripts/package_release.ps1`.
- Added `-BuildType` support to `scripts/configure_msvc.ps1`.
- Release packaging now uses a separate `build-release/` directory with `CMAKE_BUILD_TYPE=Release`.
- Release output is generated under `out/PasswordManager-portable/`.
- Qt runtime files are deployed with `windeployqt`.
- Bundled 7-Zip files are copied from project `tools/7zip/` into release `tools/7zip/`.
- Added `--smoke-test` startup mode for automated release validation.
- Fixed `SevenZipProbe` to use a small `7z -version` probe with timeout cleanup.

## Release Directory

```text
out/PasswordManager-portable/
  PasswordManager.exe
  Qt6Core.dll
  Qt6Gui.dll
  Qt6Sql.dll
  Qt6Widgets.dll
  vc_redist.x64.exe
  platforms/
  sqldrivers/
  tools/7zip/
  data/
  config/
  logs/
  backup/
  README_RELEASE.txt
```

## Validation Results

| Check | Result |
| --- | --- |
| Debug CTest in `build/` | Passed, 7/7 |
| Release package build | Passed |
| Release smoke test | Passed, exit code 0 |
| Bundled 7-Zip validation | Passed with `7z i` |
| Release CTest in `build-release/` | Passed, 7/7 |

## Important Note

Do not directly double-click Debug test executables such as `build/PasswordManagerTests.exe`.
They depend on Debug Qt DLLs such as `Qt6Cored.dll`.

Use CTest instead:

```powershell
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build --output-on-failure
```

For portable runtime validation, use the release package:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
.\out\PasswordManager-portable\PasswordManager.exe --smoke-test
.\out\PasswordManager-portable\tools\7zip\7z.exe i
```

## Remaining Work

- Phase 11 can focus on packaging polish: ZIP archive creation, versioned release names, checksum file, and a manual acceptance checklist.
- GPU and multi-thread acceleration remains a later dedicated topic.
