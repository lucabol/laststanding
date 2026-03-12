# Unified CI: Single workflow using ci.ps1

**Author:** Dallas (Systems Dev)
**Date:** 2026-03-17
**Status:** Implemented

## Decision

Replaced three separate GitHub Actions workflows (`windows-ci.yml`, `linux-ci.yml`, `arm-ci.yml`) with a single `ci.yml` that uses `ci.ps1` as the sole entry point.

## Changes

1. **ci.ps1 now detects its host OS** using `$IsLinux`/`$IsWindows` (PowerShell Core automatic variables). On native Linux (e.g., GitHub Actions ubuntu runner with pwsh), it calls `./Taskfile` directly — no WSL wrapper, no CRLF stripping. On Windows, the existing WSL-based behavior is preserved.

2. **Single workflow `.github/workflows/ci.yml`** with three jobs:
   - `windows` — `runs-on: windows-latest`, calls `./ci.ps1 -Target windows`
   - `linux` — `runs-on: ubuntu-latest`, calls `./ci.ps1 -Target linux`
   - `arm` — `runs-on: ubuntu-latest`, installs cross-compilers + qemu, calls `./ci.ps1 -Target arm`

3. **Deleted:** `windows-ci.yml`, `linux-ci.yml`, `arm-ci.yml`.

## Rationale

- Single source of truth for CI logic — ci.ps1 handles compiler detection, failure reporting, summary output, and binary size tables regardless of platform.
- Adding a new test file or build step requires changing only ci.ps1, not three separate YAML files.
- All GitHub Actions runners have `pwsh` pre-installed.
- ci.ps1 still works identically on a developer's Windows machine (using WSL for Linux/ARM).

## Impact

- No change for local dev workflow (`.\ci.ps1` on Windows still uses WSL).
- GitHub Actions CI now runs through ci.ps1 on all platforms.
- Old workflow files removed — any references to them in branch protection rules need updating to the new `CI` workflow name.
