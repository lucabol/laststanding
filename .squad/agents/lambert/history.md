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

## Work Session — 2026-03-31

Wrote L_Str test suite with comprehensive coverage.

**Test Suite (test/test.c):**
- 7 test functions with 76 total assertions
- Coverage: constructors, comparison, slicing/search, arena ops, split/join, case conversion, buf helpers
- Test functions:
  1. `test_l_str_constructors()` — `l_str`, `l_str_from`, `l_str_null` with edge cases (empty, NULL, arena exhaustion)
  2. `test_l_str_comparison()` — `l_str_eq`, `l_str_cmp`, `l_str_startswith`, `l_str_endswith`, `l_str_contains`
  3. `test_l_str_slicing()` — `l_str_sub`, `l_str_trim`, `l_str_ltrim`, `l_str_rtrim` with boundary conditions
  4. `test_l_str_search()` — `l_str_chr`, `l_str_rchr`, `l_str_find` with not-found cases
  5. `test_l_str_arena()` — `l_str_dup`, `l_str_cat`, `l_str_cstr`, `l_str_from_cstr` with arena tracking
  6. `test_l_str_split_join()` — `l_str_split`, `l_str_join` with various delimiters and empty cases
  7. `test_l_str_case_and_buf()` — `l_str_upper`, `l_str_lower`, `l_buf_push_str`, `l_buf_push_cstr`, `l_buf_push_int`, `l_buf_as_str`

**Key test patterns:**
- Always use `l_str_eq()` for L_Str comparisons (not null-terminated, can't use l_strcmp on .data)
- Exception: `l_str_cstr()` results are null-terminated, so `l_strcmp()` is safe there
- Arena lifecycle isolated per test group with 4096-byte arenas
- All 76 assertions passing on Windows build ✓

**CI result:** All tests passing on Windows and cross-platform targets ✓

## Learnings

- L_Str is not null-terminated; must never use l_strcmp on raw `.data` field
- Arena-based string construction requires careful lifecycle management; isolate per test function
- Delimiters in `l_str_split` can appear at boundaries; test both "delim at start" and "delim at end"-03-11T11:30:00Z

Fixed ERRORLEVEL bug in PR #27 Windows CI workflow (per Ripley's review). Applied delayed expansion fix (`enabledelayedexpansion` + `!ERRORLEVEL! neq 0`) to test validation logic. Tested locally, pushed to PR branch, commented with explanation. Also approved for PRs #28 and #29 contributions (ARM CI + extended tests).

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->

- **2026-03-11 — Issue #12: Expanded test coverage.** Added `test/test_extended.c` with 82 edge-case tests for 15 l_* function groups (strlen, strstr, strchr, strrchr, strcpy, strncmp, memcmp, memcpy, memset, memmove, reverse, isdigit, atoi/atol, itoa, plus itoa↔atoi roundtrip). Updated `build.bat`. PR #29.
- **Gitignore note:** The `.gitignore` pattern `test_*` catches files in `test/` that start with `test_`. New test files must be force-added with `git add -f`, or named without a `test_` prefix (e.g. `extended.c`).
- **2026-03-11 — Issue #12: l_strstr empty behavior.** `l_strstr("", "")` returns NULL, unlike libc which returns the haystack pointer. Documented in tests. Decision pending from Dallas (fix implementation vs. document as intentional).
- **main() signature:** On Windows, `l_os.h` declares `int main(int argc, char* argv[])` — all test files must match this signature exactly (not `int main(void)`).
- **Build on Windows without dev prompt:** clang is at `C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\Llvm\x64\bin\clang.exe` — the x64 Native Tools Command Prompt sets this in PATH automatically.
- **2026-03-11 — PR #27 ERRORLEVEL fix.** Windows batch `if errorlevel 1` does a signed `>= 1` comparison, so `exit(-1)` (ERRORLEVEL = -1) passes silently. Fix: use `setlocal enabledelayedexpansion` and `if !ERRORLEVEL! neq 0`. Also `%VAR%` inside `for` loop parenthesized blocks must use `!VAR!` for delayed expansion. This applies to `test_all.bat` as well (same gap exists there).
- **2026-03-12 — Dallas fixed build blockers.** (1) `build.ps1` CRLF conversion now pure PowerShell byte-level (`ReadAllBytes`/`WriteAllBytes`) instead of WSL `sed -i`; (2) l_os.h x86_64/AArch64 syscall macros wrapped with GCC statement expressions; (3) `verify.bat` exit code leak fixed with `exit /b 0`. All cross-platform targets pass.
- **Platform availability of l_* functions.** `l_lseek`, `l_dup`, `l_mkdir`, `l_chdir` are Linux-only (syscall wrappers, no Windows implementations). `l_wcslen` is cross-platform (defined in common section of l_os.h). Tests for Linux-only functions must be guarded with `#ifndef _WIN32`.
- **l_wcslen works fine on Windows.** The old comment "causes issues" in test.c was stale. `l_wcslen` with `L"..."` wide string literals compiles and runs correctly on Windows with clang/freestanding. Added 5 passing tests.
- **Expanded test coverage for l_lseek, l_dup, l_mkdir, l_wcslen.** Added to `test/test_extended.c`: l_wcslen (5 tests, cross-platform), l_lseek (11 tests, Linux-only guard), l_dup (7 tests, Linux-only guard), l_mkdir (4 tests, Linux-only guard). All pass on Windows build. Linux-guarded tests compile-skip cleanly.
- **2026-03-25 — Showcase smoke fixtures.** All 10 non-interactive showcase utilities can be smoke-tested deterministically if CI uses `test/fixtures` and the matching `*.out` snapshots. Run path-reporting tools (`checksum`, `wc`) from the fixture directory so filenames stay basename-only across Windows and Unix.
- **Showcase smoke gotchas.** `countlines` returns the line count as its exit code, so keep the fixture tiny (<255 lines) and assert stdout too. Compare showcase stdout byte-for-byte: `countlines` has no trailing newline, and `hexdump` keeps a trailing space on its final offset line.
- **2026-03-26 — Error reporting test plan.** Wrote comprehensive test plan for `l_errno()`, `l_strerror()`, and 12 error constants (`L_ENOENT` through `L_ENOTEMPTY`). 16 test cases covering correctness, edge cases, unknown codes, cross-platform constant validation, consecutive error updates, pointer stability, and integration with `l_open`/`l_read`/`l_write`/`l_close`. Key platform gotchas: `L_EISDIR` and `L_EACCES` tests are Linux-only (Windows ACL model doesn't map cleanly); write-to-readonly-fd may yield EBADF on Linux but EACCES on Windows. Plan delivered to `.squad/decisions/inbox/lambert-error-test-plan.md`. Awaiting Dallas's implementation before writing actual test code.
- **2026-04-01 — Hostname regression coverage.** For deterministic socket-resolution tests, keep `l_inet_addr("localhost") == 0` to pin the raw dotted-quad parser, then drive the public `l_resolve(const char *hostname, char *ip_out)` API through literal passthrough, `localhost`, NULL/empty/malformed inputs, and a guarded-buffer overrun check. Integration coverage in `test/test.c` should feed `l_resolve("localhost", ip)` into loopback TCP/UDP tests and compare that path with direct `"127.0.0.1"` loopback usage.
- **Approved behavior note — use `l_resolve` for localhost integration.** On the current Windows build, `l_resolve("localhost", ip)` succeeds but `l_socket_connect(sock, "localhost", port)` is not a reliable contract to test directly. Keep the public resolver as the assertion point and pass its IPv4 string into socket helpers for deterministic cross-platform coverage.
- **Validation gap — Windows test freshness and poll behavior.** `test_all.bat` skips rebuilding when binaries merely exist, so tester workflows must rebuild first after editing sources. After a fresh Windows rebuild, the current failure is still `test_poll()` (`ready > 0` on a pipe), while the hostname/socket assertions pass.

## Note — 2026-03-24

Dallas delivered two new showcase programs: base64 encoder/decoder (RFC 4648, buffered I/O, streaming) and sort utility (shell sort with Knuth gaps, supports -r/-f/-n/-u flags). Also fixed test_all.bat to use `test*.exe` pattern to avoid hanging on stdin-reading tools. All 15 CI tests pass.

- **2026 — L_Str test suite.** Added 7 test functions with 76 assertions covering all ~25 L_Str functions: constructors (`l_str`, `l_str_from`, `l_str_null`), comparison (`l_str_eq`, `l_str_cmp`, `l_str_startswith`, `l_str_endswith`, `l_str_contains`), slicing (`l_str_sub`, `l_str_trim`, `l_str_ltrim`, `l_str_rtrim`, `l_str_chr`, `l_str_rchr`, `l_str_find`), arena ops (`l_str_dup`, `l_str_cat`, `l_str_cstr`, `l_str_from_cstr`), split/join (`l_str_split`, `l_str_join`), case (`l_str_upper`, `l_str_lower`), and buf helpers (`l_buf_push_str`, `l_buf_push_cstr`, `l_buf_push_int`, `l_buf_as_str`). Key pattern: always use `l_str_eq` for L_Str comparisons, never `l_strcmp` on `.data` (not null-terminated). For `l_str_cstr` results, `l_strcmp` is safe. All pass on Windows build.

- **2026 — Terminal pixel backend tests.** Wrote `tests/test_term_gfx.c` with 9 test functions and 30 assertions covering `l_ansi_color_rgb` (foreground, background, extremes, buffer safety, NUL termination), half-block UTF-8 encoding verification, terminal pixel dimension math, ARGB color extraction, and `l_ansi_color` regression. Also implemented `l_ansi_color_rgb` in l_os.h following the existing `l_ansi_color` pattern. Added test to `test_all.bat` and all Taskfile test runner lists. Windows CI passes clean.
