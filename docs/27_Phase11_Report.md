# Phase 11 Report - Release Archive Polish

## Completed

- `scripts/package_release.ps1` now creates a versioned archive:
  - `out/PasswordManager-0.1.0-win-x64-portable.zip`
- The script now creates a ZIP checksum file:
  - `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`
- The portable directory now includes:
  - `README_RELEASE.txt`
  - `ACCEPTANCE_CHECKLIST.txt`
  - `RELEASE_MANIFEST.txt`
- `RELEASE_MANIFEST.txt` contains SHA256 hashes for files inside the portable directory.
- The script keeps all generated release artifacts under `out/`.

## Validation Results

| Check | Result |
| --- | --- |
| Release script | Passed |
| ZIP file exists | Passed |
| SHA256 file exists | Passed |
| SHA256 comparison | Passed |
| ZIP integrity test with bundled 7-Zip | Passed |
| Release smoke test | Passed, exit code 0 |
| Release CTest in `build-release/` | Passed, 7/7 |
| Debug CTest in `build/` | Passed, 7/7 |

## Release Artifacts

```text
out/PasswordManager-portable/
out/PasswordManager-0.1.0-win-x64-portable.zip
out/PasswordManager-0.1.0-win-x64-portable.zip.sha256
```

## Verification Commands

```powershell
powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath "C:\Qt\6.8.3\msvc2022_64"
.\out\PasswordManager-portable\PasswordManager.exe --smoke-test
.\out\PasswordManager-portable\tools\7zip\7z.exe t .\out\PasswordManager-0.1.0-win-x64-portable.zip
& "C:\Program Files\CMake\bin\ctest.exe" --test-dir build-release --output-on-failure
```

## Remaining Work

- Phase 12 should focus on the Windows shell action behavior:
  - `--shell-action add-test-queue <archive>` should scan/register the archive and enqueue password tests.
  - `--shell-action view-results <archive>` should focus or filter related history.
- GPU and multi-thread acceleration remains a later dedicated topic.
