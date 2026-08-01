# Phase 25 Report - Performance Baseline Preparation

## Completed

- Added `PerformanceBenchmarkService`.
- Added settings-page `运行性能基准`.
- Added command-line `--benchmark [folder]`.
- Benchmark report includes:
  - database read timings
  - password candidate generation timing
  - optional archive scan timing
  - notes that this is a single-thread baseline
- Added `PerformanceBenchmarkServiceTests`.

## Current Behavior

- Benchmark reports are written under `logs/benchmark-*.txt`.
- Benchmark does not mutate the database.
- Benchmark does not test GPU acceleration or 7-Zip password throughput.

## Verification

- `powershell -ExecutionPolicy Bypass -File scripts/build_msvc.ps1`: passed.
- `ctest --test-dir build --output-on-failure`: 13/13 passed.
- `powershell -ExecutionPolicy Bypass -File scripts/package_release.ps1 -QtPrefixPath 'C:\Qt\6.8.3\msvc2022_64'`: passed.
- `ctest --test-dir build-release --output-on-failure`: 13/13 passed.
- `out/PasswordManager-portable/PasswordManager.exe --smoke-test`: exit code 0.
- `out/PasswordManager-portable/PasswordManager.exe --benchmark`: exit code 0.
- `out/PasswordManager-portable/tools/7zip/7z.exe t out/PasswordManager-0.1.0-win-x64-portable.zip`: passed.
- Release ZIP SHA256 matches `out/PasswordManager-0.1.0-win-x64-portable.zip.sha256`.
