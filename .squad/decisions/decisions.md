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

---

## Rename build.ps1 → ci.ps1 + multi-compiler matrix
**Author:** Dallas  
**Date:** 2026-03-15  
**Status:** Implemented

### Context
`build.ps1` performs build + test + verify, not just build. Script naming needed updating to reflect scope.

### Decision
1. Renamed `build.ps1` → `ci.ps1`
2. `build.ps1` kept as backward-compatible one-line wrapper
3. `ci.ps1` defaults to `-Compiler all`, running gcc + clang for Linux and ARM targets
4. Build matrix: Windows (clang), Linux (gcc + clang), ARM (gcc + clang)

### Implications
- All existing references to `build.ps1` still work
- CI now catches compiler-specific bugs
- `verify` and `verify_arm` pass through compiler args
- README.md updated to reference `ci.ps1`

---

## l_getopt: No I/O from the parser
**Author:** Dallas  
**Date:** 2026-07-25  
**Status:** Implemented

### Decision

`l_getopt` does no I/O — it returns `'?'` for unknown options and missing arguments, but never prints error messages itself. The `l_opterr` variable is kept for POSIX API compatibility but has no effect.

### Rationale

`l_getopt` lives in the `#ifndef L_OSH` section of l_os.h, while `l_puts` (and all I/O functions) are declared inside `#ifdef L_WITHDEFS`. Due to the multi-include pattern used by test.c (`#include "l_os.h"` without `L_MAINFILE` first, then with), `l_puts` isn't visible when `l_getopt` is compiled. Rather than adding a forward declaration or restructuring the file, keeping the function pure is cleaner for a freestanding library — callers already handle their own error output.

### Impact

Callers (sort.c, ls.c, etc.) must handle the `'?'` return themselves if they want error messages. Both refactored examples already do this via their `default:` switch case.

---

## Error Reporting Layer
**Author:** Dallas  
**Date:** 2026-07-25  
**Status:** Implemented

Added cross-platform error reporting: `l_errno()`, `l_strerror()`, error constants (`L_ENOENT`, `L_EACCES`, etc.), and Win32 error translation.

### Design Notes

- **Error constants** use POSIX values on all platforms (e.g., `L_ENOENT=2`). On Windows, `l_win_error_to_errno()` maps Win32 `GetLastError()` codes to our `L_E*` constants.
- **l_last_errno** is a module-level `static int`. Accessed through `static inline` helpers (`l_errno()`, `l_set_errno()`, `l_set_errno_from_ret()`) to avoid `-Wstatic-in-inline` when called from external-linkage `inline` functions.
- **Linux:** `l_set_errno_from_ret(ret)` extracts errno from negative syscall return values. Return values are unchanged (still return -errno, not -1).
- **Windows:** Each I/O function explicitly calls `l_set_errno()` on failure with the translated Win32 error, and `l_set_errno(0)` on success.
- **Override macros:** `errno`, `strerror`, and all `E*` constants are `#undef`'d before redefining to avoid conflicts with system headers.
- **Thread safety:** Not thread-safe (single static variable). Acceptable for a freestanding single-threaded runtime without TLS.
- **Important usage pattern:** `l_errno()` must be captured immediately after a failing call — any subsequent `l_write`/`puts` call will overwrite it.

### Impact

- `l_os.h`: ~80 new lines (constants, helpers, strerror, win32 mapper, errno-setting in 4 core I/O functions)
- `test/test.c`: New `test_errno_strerror` function (~50 lines, 30 assertions)
- No breaking changes to existing code
- All 22 CI targets pass (Windows, Linux gcc/clang, ARM gcc/clang, AArch64 gcc/clang — build+test+verify)
