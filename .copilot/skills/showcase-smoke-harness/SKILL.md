---
name: "showcase-smoke-harness"
description: "Cross-platform byte-exact smoke tests for standalone showcase binaries"
domain: "testing"
confidence: "high"
source: "manual"
---

## Context

Use this pattern when a repo ships standalone demo or utility binaries that must be validated on Windows and Linux/QEMU without adding a full integration-test framework. It fits this codebase especially well because the binaries are freestanding, write raw bytes, and often care about exact trailing newlines or spaces.

## Patterns

### LF-locked fixture directory

Put tiny inputs and exact expected outputs in a dedicated directory under `test/`, then pin that directory to `eol=lf` in `.gitattributes`. This keeps byte-for-byte comparisons stable on Windows checkouts and under WSL.

### Explicit smoke whitelist

Do not glob `bin/*` for showcase checks. Invoke an explicit list of binaries so interactive demos stay build-only unless they have a deterministic no-PTY path.

### Native harness per host OS

- **Linux/WSL/QEMU:** use a bash harness that runs the target binary directly (or through `qemu-*`) and compares stdout with `cmp -s`.
- **Windows:** use a PowerShell harness, but capture stdout with `System.Diagnostics.Process`. PowerShell `>` rewrites line endings and breaks byte-exact comparisons.

### Keep helper files out of compilation

If helper scripts or fixture directories live under `test/`, make sure build loops compile `test/*.c`, not `test/*`. Otherwise cross-platform build scripts will try to compile shell scripts and fixture directories.

### WSL CRLF normalization must include harness files

If CI already strips CRLF before invoking WSL, extend that step to cover any shell harnesses and LF-sensitive fixture files added for smoke tests. Otherwise the harness can fail before the actual smoke command runs.

## Examples

- `test/showcase_smoke/` — LF-locked fixtures and expected outputs
- `test/showcase_smoke.sh` — bash/QEMU smoke harness
- `test/showcase_smoke.ps1` — Windows-native raw-stdout harness
- `Taskfile` + `test_all.bat` — integrate smoke checks into existing `test` flows instead of adding a separate CI action

## Anti-Patterns

- **PowerShell `>` for exact output** — It normalizes text and line endings.
- **Interactive demos in the smoke whitelist** — Avoid anything that needs raw terminal input unless there is a real deterministic harness.
- **Fixture paths in stdout expectations** — `cd` into the fixture directory first so programs print stable bare filenames instead of host-specific paths.
