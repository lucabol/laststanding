# Project Context

- **Owner:** Luca Bolognese
- **Project:** laststanding — A freestanding C runtime. Minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Statically linked, stripped, stdlib-free.
- **Stack:** C, inline assembly, cross-platform (Linux x86_64, ARM, AArch64, Windows)
- **Key file:** `l_os.h` — single header containing everything (string/memory functions, number conversion, syscall wrappers, file openers, platform startup code)
- **Build:** `./Taskfile test` (Linux), `build.bat` / `test_all.bat` (Windows)
- **Compiler flags:** `-Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin`
- **Conventions:** `l_` prefix for all functions, `L_FD` type for file descriptors, `L_MAINFILE` compile guard, UTF-8 internally
- **Created:** 2026-03-11

## Work Session — 2026-03-11T11:30:00Z

Completed PR review sweep (PRs #27, #28, #29). Identified critical ERRORLEVEL bug in Windows test validation. Verdicts:
- #27: Changes Requested (ERRORLEVEL signed comparison bug; use `!ERRORLEVEL! neq 0` with delayed expansion)
- #28: Approved (ARM/AArch64 CI matrix clean; QEMU + static linking correct)
- #29: Approved (82 edge-case tests; follows test.c pattern exactly)

Detailed verdicts written to .squad/decisions/inbox/ripley-pr-reviews.md and merged into decisions.md.

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->
- **Windows ERRORLEVEL gotcha:** `l_exit(-1)` calls `ExitProcess(-1)` on Windows, setting ERRORLEVEL to -1 (signed). Batch `if errorlevel 1` tests >= 1 (signed), so it misses -1. Linux `l_exit` masks with `& 255`, so exit(-1) → 255 which bash catches. Any Windows batch test runner must use `!ERRORLEVEL! neq 0` with `enabledelayedexpansion`.
- **Taskfile `test/*` glob pattern:** The Taskfile uses `for f in test/*` (not `test/*.c`). CI workflows matching this pattern are consistent with the project. Currently test/ only contains .c files.
- **`test_all.bat` doesn't check error codes:** It runs all exes but doesn't detect failures. CI workflows should NOT rely on it for pass/fail gating.
- **PR review on own repos:** GitHub API rejects `--approve` and `--request-changes` on your own PRs. Use `gh pr comment` for detailed reviews instead.
- **2026-03-12 — Dallas follow-up fixes.** All PR #27 blocking issues resolved: (1) `build.ps1` CRLF now pure PowerShell (ReadAllBytes/WriteAllBytes); (2) l_os.h x86_64/AArch64 syscall statement expressions + startup asm; (3) `verify.bat` exit code. Tests passing across all platforms.
