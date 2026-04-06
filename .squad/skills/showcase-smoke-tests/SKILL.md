---
name: "showcase-smoke-tests"
description: "Patterns for deterministic showcase utility smoke tests"
domain: "testing"
confidence: "high"
source: "manual"
---

## Context
The `test/` directory mixes a real unit test binary (`test.c`), non-interactive showcase utilities, and interactive demos. For CI smoke coverage, the safest pattern is to treat the showcase utilities like golden-output tools: tiny fixtures in `test/fixtures`, exact expected outputs in sibling `*.out` files, and a strict whitelist of binaries to invoke.

## Patterns

### Run path-reporting tools from the fixture directory
Some utilities echo the filename they were given (`checksum`, `wc`). To keep output identical across Windows and Unix, set the working directory to `test/fixtures` and invoke them with basename arguments like `lines.txt` instead of repository-relative paths.

### Keep fixtures normalized in Git
Store `test/fixtures/*.txt` and `test/fixtures/*.out` with LF endings and `test/fixtures/*.bin` as binary. This keeps byte counts, base64 inputs, and golden-output snapshots stable on Windows checkouts and Linux/WSL runners.

### Use byte-exact stdout snapshots
Prefer exact `*.out` files over ad-hoc string matching for showcase programs. `countlines` emits no trailing newline, and `hexdump` preserves a trailing space on its final offset line, so line-oriented helpers can accidentally mask regressions.

### Keep exit-code semantics small and deliberate
If a showcase tool returns a semantic value in its exit code, the fixture must stay tiny. `countlines` returns the line count, so use a three-line file and assert both stdout and exit code rather than depending on shell-specific exit-code handling for larger values.

### Smoke-test only the non-interactive utility tier
The safe always-on matrix is `base64`, `checksum`, `countlines`, `grep`, `hexdump`, `ls`, `printenv` (named variable only), `sort`, `upper`, and `wc`. Keep `sh`, `led`, and `snake` build-only until there is a PTY-aware harness with deterministic input and timeouts.

## Anti-Patterns

- **Glob-running every binary in `bin/`** — That will hang on `sh`, `led`, or `snake`.
- **Using `printenv` with no arguments in CI** — Environment enumeration is noisy and runner-dependent.
- **Comparing showcase output with trimmed strings** — You will miss real regressions in spacing or newline behavior.
