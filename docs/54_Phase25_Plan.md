# Phase 25 Plan - Performance Baseline Preparation

## Goal

Create a single-thread performance baseline before discussing multi-threading or GPU acceleration.

## Scope

- Add a local benchmark service.
- Measure database read time for core tables.
- Measure password candidate generation time.
- Optionally measure archive scanning time for a selected folder.
- Add settings-page `运行性能基准`.
- Add command-line `--benchmark [folder]`.
- Add automated tests.

## Out Of Scope

- GPU acceleration.
- Multi-threaded execution.
- 7-Zip password throughput benchmark.
- Automatic optimization.

## Acceptance

- Benchmark writes a timestamped report under `logs/`.
- Benchmark can run without a scan folder.
- Optional scan folder benchmark is supported.
- The report clearly states that this is a single-thread baseline.
