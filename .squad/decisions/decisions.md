# Decisions

## Linux x86_64 syscall macros are broken
**Author:** Dallas  
**Date:** 2025-07-24  
**Status:** Fixed

### Issue
The `my_syscall0` through `my_syscall6` macros in `l_os.h` (line 633+) for x86_64 Linux were missing GCC statement-expression wrappers.

### Resolution (2026-03-11)
- Added `({` at the start and `_ret; })` at the end of each `my_syscallN` macro
- Fixed AArch64 startup assembly alignment issues
- Removed dead ARM block code
- All x86_64 and AArch64 builds now pass

---

## Windows CI uses clang on GHA windows-latest
**Author:** Dallas  
**Date:** 2026-03-11  
**Status:** Fixed
**Issue:** #14

### Context
The Windows build (`build.bat`) already uses `clang` with `-ffreestanding -lkernel32`. GitHub Actions `windows-latest` runners ship with LLVM/clang pre-installed.

### Decision
Windows CI workflow (`windows-ci.yml`) uses runner's built-in clang with `shell: cmd` calling existing batch scripts.

### Implications
- Fixed verify.bat findstr exit code leak that was silently ignoring failures

---

## PR Review Verdicts — Ripley (2026-03-11)

### PR #27 — "ci: Add Windows CI workflow" (Dallas)
**Verdict: Changes Requested**

The workflow structure is correct — `windows-latest`, clang verification, `build.bat`, `verify.bat` — all good. Inlining the test loop instead of calling `test_all.bat` was the right call; `test_all.bat` doesn't check error codes either.

**Blocking issue (RESOLVED 2026-03-12):** The `if errorlevel 1` check in the test step was failing to catch `exit(-1)` failures. Fixed with `setlocal enabledelayedexpansion` and `if !ERRORLEVEL! neq 0` pattern. Also fixed `verify.bat` findstr exit code leak.

**Resolution:** Dallas fixed both Windows CI and verify.bat error handling. Changes verified.

### PR #28 — "ci: Add ARM/AArch64 CI workflow" (Lambert)
**Verdict: Approved**

Matrix strategy is clean. Compiler flags match the Taskfile exactly. QEMU user-mode with `-static` linking and correct sysroots is the right approach. `fail-fast: false` is correct — we want to see ALL platform failures, not just the first.

Bash `set -e` in GHA properly catches `exit(-1)` failures (truncated to exit code 255 on Linux via `status & 255`). The `for f in test/*` glob matches the Taskfile's own pattern.

**Resolution (2026-03-12):** Dallas fixed AArch64 startup asm alignment issues. Taskfile gap resolved.

### PR #29 — "test: Expand test coverage for l_* functions" (Lambert)
**Verdict: Approved**

82 edge-case tests across 15 function groups. Follows existing `test.c` pattern exactly — same include order, same macros, same `exit(-1)` on failure. Edge cases are meaningful: empty strings, embedded nulls, overlapping memmove, boundary chars, all radices for itoa, roundtrip verification.

The `l_strstr("","")` divergence from libc is documented as known behavior. This is tracked in decisions.md — Dallas should decide whether to fix or formalize the deviation. Not a blocker for this PR.

**Minor follow-up:** Test macros duplicated between `test.c` and `test_extended.c`. Consider extracting a shared `test/test_macros.h` in a future PR.
