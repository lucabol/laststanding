# Project Context

- **Owner:** Luca Bolognese
- **Project:** laststanding — A freestanding C runtime. Minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Statically linked, stripped, stdlib-free.
- **Stack:** C, inline assembly, cross-platform (Linux x86_64, ARM, AArch64, Windows)
- **Key file:** `l_os.h` — single header containing everything (string/memory functions, number conversion, syscall wrappers, file openers, platform startup code)
- **Build:** `./Taskfile test` (Linux), `build.bat` / `test_all.bat` (Windows)
- **Compiler flags:** `-Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin`
- **Conventions:** `l_` prefix for all functions, `L_FD` type for file descriptors, `L_MAINFILE` compile guard, UTF-8 internally
- **Tests:** Each `.c` file in `test/` compiles to one binary in `bin/`. Uses `TEST_ASSERT(condition, "description")` and `TEST_FUNCTION("name")` macros from `test/test.c`.
- **Created:** 2026-03-11

## Work Session — 2026-03-11T11:30:00Z

Fixed ERRORLEVEL bug in PR #27 Windows CI workflow (per Ripley's review). Applied delayed expansion fix (`enabledelayedexpansion` + `!ERRORLEVEL! neq 0`) to test validation logic. Tested locally, pushed to PR branch, commented with explanation. Also approved for PRs #28 and #29 contributions (ARM CI + extended tests).

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->

- **2026-03-11 — Issue #12: Expanded test coverage.** Added `test/test_extended.c` with 82 edge-case tests for 15 l_* function groups (strlen, strstr, strchr, strrchr, strcpy, strncmp, memcmp, memcpy, memset, memmove, reverse, isdigit, atoi/atol, itoa, plus itoa↔atoi roundtrip). Updated `build.bat`. PR #29.
- **Gitignore note:** The `.gitignore` pattern `test_*` catches files in `test/` that start with `test_`. New test files must be force-added with `git add -f`, or named without a `test_` prefix (e.g. `extended.c`).
- **2026-03-11 — Issue #12: l_strstr empty behavior.** `l_strstr("", "")` returns NULL, unlike libc which returns the haystack pointer. Documented in tests. Decision pending from Dallas (fix implementation vs. document as intentional).
- **main() signature:** On Windows, `l_os.h` declares `int main(int argc, char* argv[])` — all test files must match this signature exactly (not `int main(void)`).
- **Build on Windows without dev prompt:** clang is at `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\Llvm\x64\bin\clang.exe` — the x64 Native Tools Command Prompt sets this in PATH automatically.
- **2026-03-11 — PR #27 ERRORLEVEL fix.** Windows batch `if errorlevel 1` does a signed `>= 1` comparison, so `exit(-1)` (ERRORLEVEL = -1) passes silently. Fix: use `setlocal enabledelayedexpansion` and `if !ERRORLEVEL! neq 0`. Also `%VAR%` inside `for` loop parenthesized blocks must use `!VAR!` for delayed expansion. This applies to `test_all.bat` as well (same gap exists there).
