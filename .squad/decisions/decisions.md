# Decisions

## Linux x86_64 syscall macros are broken
**Author:** Dallas  
**Date:** 2025-07-24  
**Status:** Needs Fix

### Issue
The `my_syscall0` through `my_syscall6` macros in `l_os.h` (line 633+) for x86_64 Linux are missing GCC statement-expression wrappers. They expand to bare statements:

```c
#define my_syscall1(num, arg1) \
         long _ret; \
         register long _num asm("rax") = (num); \
         ...
```

When used as `return my_syscall1(...)`, this expands to `return long _ret;` — invalid C.

The correct pattern (from Linux nolibc) wraps in `({...})`:

```c
#define my_syscall1(num, arg1) ({ \
         long _ret; \
         register long _num asm("rax") = (num); \
         ... \
         _ret; \
})
```

### Impact
- Linux x86_64 builds via WSL and native Linux are broken (`Taskfile build` fails)
- ARM CI works because `arm-ci.yml` uses inline build commands that compile directly, bypassing the Taskfile
- Windows builds are unaffected (different code path using Win32 API)

### Recommendation
Fix the macros to use GCC statement expressions. Each `my_syscallN` needs `({` at the start and `_ret; })` at the end.

---

## Windows CI uses clang on GHA windows-latest
**Author:** Dallas  
**Date:** 2026-03-11  
**Issue:** #14

### Context
The Windows build (`build.bat`) already uses `clang` with `-ffreestanding -lkernel32`. GitHub Actions `windows-latest` runners ship with LLVM/clang pre-installed, so no additional toolchain install step is needed.

### Decision
The Windows CI workflow (`windows-ci.yml`) uses the runner's built-in clang rather than installing MSVC or a separate LLVM package. Steps use `shell: cmd` with `call` to run the existing batch scripts directly.

### Implications
- If the runner image drops clang in the future, we'll need to add an install step.
- MSVC-specific tools like `dumpbin` may or may not be on PATH; `verify.bat` already handles that gracefully with fallbacks.

---

## PR Review Verdicts — Ripley (2026-03-11)

### PR #27 — "ci: Add Windows CI workflow" (Dallas)
**Verdict: Changes Requested**

The workflow structure is correct — `windows-latest`, clang verification, `build.bat`, `verify.bat` — all good. Inlining the test loop instead of calling `test_all.bat` was the right call; `test_all.bat` doesn't check error codes either.

**Blocking issue:** The `if errorlevel 1` check in the test step won't catch `exit(-1)` failures. On Windows, `l_exit(-1)` calls `ExitProcess(-1)`, setting ERRORLEVEL to -1 (signed). Batch `if errorlevel 1` tests ERRORLEVEL >= 1 (signed comparison). Since -1 < 1, test failures are silently ignored. CI gives false confidence.

**Required fix:** Use `setlocal enabledelayedexpansion` and `if !ERRORLEVEL! neq 0` instead.

**Impact:** Dallas should fix and force-push. Also consider fixing `test_all.bat` to match (separate issue).

### PR #28 — "ci: Add ARM/AArch64 CI workflow" (Lambert)
**Verdict: Approved**

Matrix strategy is clean. Compiler flags match the Taskfile exactly. QEMU user-mode with `-static` linking and correct sysroots is the right approach. `fail-fast: false` is correct — we want to see ALL platform failures, not just the first.

Bash `set -e` in GHA properly catches `exit(-1)` failures (truncated to exit code 255 on Linux via `status & 255`). The `for f in test/*` glob matches the Taskfile's own pattern.

AArch64 Taskfile gap is already tracked in decisions.md.

### PR #29 — "test: Expand test coverage for l_* functions" (Lambert)
**Verdict: Approved**

82 edge-case tests across 15 function groups. Follows existing `test.c` pattern exactly — same include order, same macros, same `exit(-1)` on failure. Edge cases are meaningful: empty strings, embedded nulls, overlapping memmove, boundary chars, all radices for itoa, roundtrip verification.

The `l_strstr("","")` divergence from libc is documented as known behavior. This is tracked in decisions.md — Dallas should decide whether to fix or formalize the deviation. Not a blocker for this PR.

**Minor follow-up:** Test macros duplicated between `test.c` and `test_extended.c`. Consider extracting a shared `test/test_macros.h` in a future PR.
