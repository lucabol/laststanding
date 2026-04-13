# Squad Decisions

## Active Decisions

### User Directive: Run CI First Before Documentation

**By:** Luca Bolognese (via Copilot)  
**Date:** 2026-03-25T10:24:11.701Z  
**Status:** Implemented

Always run CI first; if it succeeds, document new features in `README.md`, then commit and push.

---

### User Directive: Treat Warnings as Errors

**By:** Luca Bolognese (via Copilot)  
**Date:** 2026-03-25T18:14  
**Status:** Implemented

Always consider warnings as errors in local CI. If warnings appear, treat the build as broken and fix them.

---

### User Directive: Always Run CI After Any Main Change (Reinforced)

**By:** Luca Bolognese (via Copilot)  
**Date:** 2026-03-25T07:21:00Z  
**Status:** Implemented

Always run ci.ps1 (full 15/15) after ANY code change lands on main — whether from our own commits, merged PRs, or pulled external changes. Never declare something green without a local CI run. Never move on to the next task after a merge without running CI first. This directive has been reinforced multiple times and is non-negotiable.

---

### l_getcwd added, l_chdir made cross-platform

**Author:** Dallas  
**Date:** 2026-07-25  
**Status:** Implemented

Added `l_getcwd` as a new cross-platform function and promoted `l_chdir` from Unix-only to cross-platform.

**Design Notes:**
- **l_getcwd Linux:** Uses raw syscall numbers per architecture rather than `__NR_getcwd` because `<asm/unistd.h>` doesn't define `__NR_getcwd` on all targets. Follows the same pattern as `l_mmap`/`l_munmap`.
- **l_getcwd Windows:** Uses `GetCurrentDirectoryW` + new `l_wide_to_utf8` helper for UTF-16→UTF-8 conversion.
- **l_chdir Windows:** Uses `SetCurrentDirectoryW` + existing `l_utf8_to_wide` helper.
- **l_wide_to_utf8:** New helper added alongside existing `l_utf8_to_wide`. Wraps `WideCharToMultiByte`. Available for future Windows functions that need wide→UTF-8 conversion.
- **Override macros:** `getcwd` added. `chdir` was already present.

**Impact:**
- Declarations moved to cross-platform section (before `#ifdef __unix__`)
- No breaking changes to existing code
- Tests added and passing on Windows

---

### build.bat auto-detects compilers

**Author:** Dallas  
**Date:** 2025-07-24  
**Status:** Implemented

`build.bat` no longer requires launching from the x64 Native Tools Command Prompt. It auto-detects a C compiler in this order:

1. `clang` or `cl` already on PATH (covers GHA runners and dev prompts)
2. VS-bundled clang found via `vswhere.exe` (no vcvarsall needed)
3. `vcvarsall.bat x64` called to put MSVC tools on PATH
4. Error if nothing found

Users (and CI) can now run `build.bat` from any plain `cmd.exe` window without manual environment setup.

The build loop now uses `for %%f in (test\*.c)` instead of a hard-coded file list. Adding a new test `.c` file no longer requires editing `build.bat`.

---

### Showcase Programs: base64 and sort

**Author:** Dallas  
**Date:** 2026-03-24  
**Status:** Implemented

Delivered two reference implementations demonstrating freestanding I/O patterns:

- **base64.c** — RFC 4648 encoder/decoder with buffered I/O (4KB read/write buffers). Encodes with 76-char line wrapping. Decodes while skipping whitespace. Round-trip verified on binary data.
- **sort.c** — Shell sort with Knuth gap sequence. Supports `-r` (reverse), `-f` (fold case), `-n` (numeric), `-u` (unique). 64KB input buffer, 4096 line max.

**Design notes:**
- Shell sort avoids O(n²) worst case; Knuth gap sequence gives ~O(n^1.25) average
- Buffered I/O in base64 keeps memory constant regardless of file size
- Numeric sort delegates to `l_atoi` for leading whitespace/sign handling
- Test suite pattern fixed: `test_all.bat` now uses `test*.exe` glob to exclude stdin-reading tools (base64, sort, grep, hexdump, wc)

---

### Self-Hosted Spawn Test Helpers

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Eliminated platform-specific OS commands (`cmd.exe`, `/bin/sh`, `/bin/true`, `/bin/false`) from spawn tests by making the test binary itself serve as the child process via command-line helper modes.

**Decision:**
- `--exit N` — Exit with code N (replaces `cmd.exe /c exit N`, `/bin/false`, `/bin/true`)
- `--echo-stderr MSG` — Write MSG to stderr (replaces shell `>&2` redirection)
- `--hold-stdin` — Consume stdin and hold until EOF (for pipe/redirection testing)

**Rationale:**
- Removes 4 `#ifdef _WIN32` blocks from test/test.c
- Output is byte-identical across all platforms (no CRLF from cmd.exe)
- No dependency on external binaries that may not exist under QEMU
- Pattern consistent with existing `--spawn-stdio-close-stdout-helper` mode

**Impact:**
- All spawn tests pass on 21 CI targets
- Improved reliability under QEMU and CI environments

---

### Environment Iteration API: l_env_start / l_env_next / l_env_end

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Designed iterator-based API for cross-platform environment variable iteration instead of exposing `l_envp` directly or using a callback pattern.

**API:**
```c
void       *l_env_start(void);
const char *l_env_next(void **iter, char *buf, size_t bufsz);
void        l_env_end(void *handle);
```

**Rationale:**
- **No allocation:** Fits freestanding constraint; caller provides stack buffer
- **Cross-platform:** Eliminates `#ifdef _WIN32` from consumers (printenv.c was motivating case)
- **Simple:** Three functions, no opaque struct, no callbacks; callers use familiar while-loop pattern
- **Consistent:** Follows existing `l_getenv` model of using `l_envp` on Unix and Win32 API on Windows

**Alternatives Rejected:**
- Callback style: Less flexible (can't break early, harder to compose)
- Global `environ()` function: Would require allocation on Windows (violates no-malloc rule)
- Single `l_env_next()` without buffer: Can't work on Windows without static buffer (thread-unsafe) or allocation

**Impact:**
- All 21 CI targets pass
- Reduced platform-specific boilerplate in consumers
- printenv.c refactored from ~30 lines platform glue to ~15 lines clean portable code

---

### l_find_executable API and sh.c Portability Cleanup

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Added cross-platform executable discovery function and used it to eliminate `sh_envp` global and environment-passing infrastructure from sh.c.

**API:**
```c
int l_find_executable(const char *cmd, char *out, size_t outsz);
```

- Returns 1 (found) or 0 (not found)
- Searches PATH with platform-appropriate separators (`;` on Windows, `:` on Unix)
- **Windows:** Tries `.exe`, `.bat`, `.com` extensions **before** bare name (critical for cross-compilation)
- **Unix:** Bare name only; no extension magic

**Design Rationale:**
- Extension search order on Windows (try extensions first, not last) is critical for cross-compilation scenarios where a directory contains both extensionless Linux binaries and `.exe` Windows binaries from the same project
- `l_spawn(..., NULL)` inherits environment on both platforms; explicit envp passing no longer needed

**sh.c Cleanup:**
- Removed `sh_envp` global
- Removed `spawn_envp()` helper
- Removed 5 `#ifdef __unix__` blocks for environment passing (only HOME/USERPROFILE remains — genuine OS difference)
- Net reduction: ~65 lines of platform-specific code

**Impact:**
- All 21 CI targets pass
- sh.c more maintainable with fewer platform-specific blocks
- Cross-compilation reliability improved

---

### l_strstr empty-string behavior

**From:** Lambert (Tester)  
**Date:** 2026-03-11  
**Status:** Resolved  
**Issue:** #12

`l_strstr("", "")` initially returned `NULL`, while standard libc `strstr("", "")` returns a pointer to the empty haystack.

**Resolution (2026-03-25):**

Dallas investigated and found the implementation **already contains the fix**. Line 894 in `l_os.h` explicitly handles the empty-needle case:

```c
if (len == 0) return (char *)s1;  // Returns haystack when needle is empty
```

This is correct POSIX behavior per the C standard: *"If `s2` points to a string with zero length, the function shall return `s1`."*

**Root Cause:** The issue description referenced an earlier version of the code (pre-2026-03-12) with a `while (*s1)` loop that didn't handle empty haystack. Dallas fixed this on 2026-03-12 as part of bounds-checking improvements.

**Verification:** `l_strstr("", "")` returns a pointer to the empty haystack (not `NULL`) ✓; `l_strstr("hello", "")` returns a pointer to "hello" ✓. Tests pass on Windows/clang.

**Status:** Implementation is POSIX-compliant. No further action needed.

**Author:** Dallas  
**Date:** 2026-03-24  
**Status:** Implemented

Created `test/sh.c` — a cross-platform freestanding interactive shell built entirely on `l_os.h`. This is the ultimate showcase of the laststanding library, demonstrating process control, I/O, piping, environment access, and string handling all without libc.

**Design Decisions:**

**Unix vs Windows feature split:** On Unix, the shell uses `l_fork` + `l_dup2` + `l_execve` directly, enabling full I/O redirection (`>`, `>>`, `<`) and single-pipe support (`cmd1 | cmd2`). On Windows, it uses `l_spawn` (which wraps `CreateProcessW`) for basic external command execution. I/O redirection and piping on Windows are deferred — the current `l_spawn` doesn't expose `STARTF_USESTDHANDLES`, and `l_dup2` on Windows can't force a handle to a specific fd number.

**Environment inheritance:** On Unix, `sh_envp` is captured from `argv + argc + 1` in `main()` and passed to `l_execve`. This ensures child processes inherit the full parent environment. On Windows, `CreateProcessW` inherits environment automatically (envp is unused).

**PATH search done manually:** Rather than relying on OS-level PATH search (which would give opaque "failed to execute" errors), the shell searches PATH directories itself with `l_open_read` existence checks. This enables clear "command not found" error messages on all platforms.

**Scope:** ~280 lines. Builtins: cd, pwd, exit, echo. External commands with PATH search. Quoted arguments. Unix redirection and piping. Compiles on all 5 targets.

**Open items:**
- Windows redirection/piping requires library changes (STARTF_USESTDHANDLES in l_spawn, or a new spawn variant)
- No signal handling (Ctrl+C kills the shell)
- No multi-pipe chains (single pipe only)

---

### Decision: showcase smoke coverage

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Smoke-test the non-interactive showcase binaries with byte-exact fixture comparisons on every supported target:
- `base64` (encode + decode)
- `checksum`
- `countlines` (stdout + expected exit code)
- `grep`
- `hexdump`
- `ls`
- `printenv` (named-variable path only, with injected process env)
- `sort` (`-n` and `-u`)
- `upper`
- `wc`

Keep `snake` build-only. For the interactive-but-safe cases, run only deterministic startup paths:
- `led` — no-arg usage output only
- `sh` — `--help` output only

**Rationale:** The fixture-based checks give us stable coverage across Windows, Linux, ARM32/QEMU, and AArch64/QEMU without introducing PTY harnesses or long-running interactive sessions.

**Implementation notes:**
- Fixtures live in `test/showcase_smoke/` and are locked to LF for exact byte comparison.
- Linux/WSL/QEMU uses `test/showcase_smoke.sh`; Windows-native uses `test/showcase_smoke.ps1`.
- Existing `test` actions own smoke execution; there is no extra CI action.

---

### Decision: Test Portability Primitives

**Author:** Dallas  
**Date:** 2026-03-26  
**Status:** Ready for Review  
**Scope:** Analysis only (no implementation)

The remaining `#ifdef` usage in `test/` splits into four buckets:

1. **Real library gaps** — add or widen `l_os.h` primitives.
2. **Library helper candidates** — useful, but keep them small and do not over-design.
3. **Test-only cleanup** — use controlled helper modes instead of OS-specific shell snippets.
4. **Fundamental / not-library concerns** — leave them out of `l_os.h`.

**Recommendation:** Spend portability budget on the first bucket only, plus one narrow I/O-readiness helper. That removes the highest-value `#ifdef`s without turning `l_os.h` into a shell framework.

**Concrete Plan:**

**Priority 1 — real API gaps:**
1. Make these existing APIs cross-platform on Windows:
   - `l_mkdir`
   - `l_lseek`
   - `l_dup`
   - `l_sched_yield`

**Priority 2 — one narrow new I/O primitive:**
2. Add a small readiness helper:
   - preferred shape: `int l_wait_readable(L_FD fd, unsigned int timeout_ms);`
   - semantics: `1` readable/EOF, `0` timeout, `-1` error

**Priority 3 — library helpers, but keep them small:**
3. Add:
   - **internal** env-iteration helper for `printenv`
   - **internal** command-resolution helper for `sh`

**Priority 4 — test cleanup, not library work:**
4. Refactor tests to use helper modes in `test.exe` instead of `cmd.exe` / `/bin/sh` snippets.
5. Delete `sh_envp` / `spawn_envp`; pass `NULL` envp and rely on current inherit behavior.

---

### Self-Hosted Spawn Test Helpers

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Eliminated platform-specific OS commands (`cmd.exe`, `/bin/sh`, `/bin/true`, `/bin/false`) from spawn tests by making the test binary itself serve as the child process via command-line helper modes.

**Decision:**
- `--exit N` — Exit with code N (replaces `cmd.exe /c exit N`, `/bin/false`, `/bin/true`)
- `--echo-stderr MSG` — Write MSG to stderr (replaces shell `>&2` redirection)
- `--hold-stdin` — Consume stdin and hold until EOF (for pipe/redirection testing)

**Rationale:**
- Removes 4 `#ifdef _WIN32` blocks from test/test.c
- Output is byte-identical across all platforms (no CRLF from cmd.exe)
- No dependency on external binaries that may not exist under QEMU
- Pattern consistent with existing `--spawn-stdio-close-stdout-helper` mode

**Impact:**
- All spawn tests pass on 21 CI targets
- Improved reliability under QEMU and CI environments

---

### Environment Iteration API: l_env_start / l_env_next / l_env_end

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Designed iterator-based API for cross-platform environment variable iteration instead of exposing `l_envp` directly or using a callback pattern.

**API:**
```c
void       *l_env_start(void);
const char *l_env_next(void **iter, char *buf, size_t bufsz);
void        l_env_end(void *handle);
```

**Rationale:**
- **No allocation:** Fits freestanding constraint; caller provides stack buffer
- **Cross-platform:** Eliminates `#ifdef _WIN32` from consumers (printenv.c was motivating case)
- **Simple:** Three functions, no opaque struct, no callbacks; callers use familiar while-loop pattern
- **Consistent:** Follows existing `l_getenv` model of using `l_envp` on Unix and Win32 API on Windows

**Alternatives Rejected:**
- Callback style: Less flexible (can't break early, harder to compose)
- Global `environ()` function: Would require allocation on Windows (violates no-malloc rule)
- Single `l_env_next()` without buffer: Can't work on Windows without static buffer (thread-unsafe) or allocation

**Impact:**
- All 21 CI targets pass
- Reduced platform-specific boilerplate in consumers
- printenv.c refactored from ~30 lines platform glue to ~15 lines clean portable code

---

### l_find_executable API and sh.c Portability Cleanup

**Author:** Dallas  
**Date:** 2026-03-25  
**Status:** Implemented

Added cross-platform executable discovery function and used it to eliminate `sh_envp` global and environment-passing infrastructure from sh.c.

**API:**
```c
int l_find_executable(const char *cmd, char *out, size_t outsz);
```

- Returns 1 (found) or 0 (not found)
- Searches PATH with platform-appropriate separators (`;` on Windows, `:` on Unix)
- **Windows:** Tries `.exe`, `.bat`, `.com` extensions **before** bare name (critical for cross-compilation)
- **Unix:** Bare name only; no extension magic

**Design Rationale:**
- Extension search order on Windows (try extensions first, not last) is critical for cross-compilation scenarios where a directory contains both extensionless Linux binaries and `.exe` Windows binaries from the same project
- `l_spawn(..., NULL)` inherits environment on both platforms; explicit envp passing no longer needed

**sh.c Cleanup:**
- Removed `sh_envp` global
- Removed `spawn_envp()` helper
- Removed 5 `#ifdef __unix__` blocks for environment passing (only HOME/USERPROFILE remains — genuine OS difference)
- Net reduction: ~65 lines of platform-specific code

**Impact:**
- All 21 CI targets pass
- sh.c more maintainable with fewer platform-specific blocks
- Cross-compilation reliability improved

---

### l_getenv deferred

**Author:** Dallas  
**Date:** 2026-03-16  
**Status:** Deferred

`l_getenv` was requested but deferred because:

1. **Linux:** The `_start` assembly stores `argc` and `argv` but does not capture `envp` (the third parameter on the stack). Adding envp support requires modifying startup code across all three architectures (x86_64, ARM, AArch64). The alternative — reading `/proc/self/environ` — works but requires a static buffer, has a 4KB+ size limit, and is Linux-specific (not portable to other future targets).

2. **Windows:** Would require `GetEnvironmentVariableW` with UTF-8↔UTF-16 conversion plus a static buffer for the result — doable but introduces a global mutable buffer, which is a new pattern for the library.

3. **Scope:** Both approaches are non-trivial and cross-cutting. Better handled as a standalone change with proper design discussion.

**Recommendation:** When needed, implement by extending `_start` to capture `envp` into a global, then implement `l_getenv` as a linear scan. This is the cleanest freestanding approach.

---

### Review: Ripley Approach A scope check (Windows process/stdio)

**Reviewer:** Lambert (QA)  
**Date:** 2026-03-25  
**Status:** Approved

Approved the Windows process/stdio redesign implementation. Verified:
- Windows descriptor remapping is real: `l_pipe()` allocates descriptor slots; `l_dup2(oldfd, newfd)` rebinds arbitrary slots
- Spawn contract is real: Windows `l_spawn()` uses the supplied `path` and passes `envp`
- Spawn inherits remapped stdio: `test_spawn_inherits_dup2` passes on Windows
- `test/sh.c` is off raw Win32: No `CreateProcessW`/`STARTUPINFOW` on Windows; uses library spawn helpers
- Unix child-fd closure remains correct
- Deterministic smoke coverage exists where it matters

**Recommendation:** Approve. The current branch delivers the roadmap item and clears the Approach A bar.

---

### Showcase smoke tests — Lambert

**Date:** 2026-03-25  
**Status:** Recommended

Use the non-interactive utility matrix on Windows, Linux x86_64, ARM32, and AArch64. Smoke-test:
- `base64` encode/decode
- `checksum` with fixture comparison
- `countlines` with exit code verification
- `grep`, `hexdump`, `ls`, `printenv`, `sort`, `upper`, `wc` — all with fixture comparison

Build-only for now: `sh`, `led`, `snake` (prompt-bearing shell, terminal editor, real-time game — too complex for deterministic CI).

**Must follow:**
- Use an explicit whitelist for showcase smoke tests. Do **not** glob `bin/*`.
- Compare stdout byte-for-byte against `test/fixtures/*.out` files.
- For `printenv`, only test the named-variable path with a per-process injected variable.
- For `ls`, smoke-test only the default `ls lsdir` path right now.
- Reuse existing CRLF normalization and QEMU detection from `ci.ps1`.
- Put a timeout around each showcase invocation.

---

### Windows Primitives Review — l_lseek, l_mkdir, l_sched_yield

**Reviewer:** Lambert (QA)  
**Date:** 2026-03-28  
**Verdict:** PASS — No Real Correctness Issues Found

All Windows implementations appear correct:
- `l_lseek` (Windows): Uses `l_win_fd_handle(fd)`, maps SEEK_* correctly, returns new offset on success, -1 on error.
- `l_mkdir` (Windows): UTF-8 → UTF-16 conversion via `l_utf8_to_wide`, mode intentionally ignored (Windows doesn't have Unix-style mode bits).
- `l_sched_yield` (Windows): Uses `Sleep(0)` correctly, returns 0.
- `l_dup` declaration moved to portable section; Windows implementation added.
- `l_spawn_stdio` / `L_SPAWN_INHERIT` provide correct inheritance/redirection semantics on both platforms.
- Test coverage: `test_lseek`, `test_mkdir`, `test_sched_yield` now compile and pass on Windows.

**Test Results:**
- Build: ✅ 14 executables
- All tests: ✅ ALL TESTS PASSED (including new spawn_stdio, spawn_inherits_dup2, spawn_pipeline_via_dup2)
- Verify: ✅ No stdlib dependencies, no exports, correct file sizes

**Decision:** APPROVED — No blockers. Ready to merge.

---

### Lambert review: Windows stdio/process model (pre-implementation assessment)

**Date:** 2026-03-25  
**Status:** Analysis (pre-implementation)

**Verdict:** Not ready for pipe coverage yet. Validated deterministic non-interactive redirection, but single-pipe shell scripts hang on Windows.

**Why hangs:** `l_pipe` creates inheritable handles on Windows. Both children inherit both ends of the pipe. That leaks a write end into the reader side, which prevents EOF.

**Contract gaps Dallas must resolve:**

1. **`l_dup2` contract is not real on Windows:** Windows implementation ignores `newfd` and returns an arbitrary duplicate handle instead. Needs either:
   - implement real target-descriptor semantics on Windows via a small descriptor table, or
   - narrow the contract and add an explicit spawn-with-stdio API that models Win32 safely.

2. **`l_spawn` contract is not real on Windows:** Implementation ignores both `path` and `envp`. Needs either:
   - make `path` and `envp` behave as documented, or
   - narrow the docs and replace the API with a Windows-safe process-spawn contract.

3. **`test/sh.c` is proving a workaround, not the library:** Windows `sh.c` hand-rolls Win32 redirection/pipe setup because `l_os.h` cannot express needed stdio wiring cleanly.

**Required test coverage before calling done:**
1. Redirection semantics through the supported API (child stdin from file, child stdout to file)
2. Single-pipe semantics on Windows (deterministic pipeline, process exits without hanging, exact output bytes match)
3. Documented spawn contract (explicit `path` works, custom `envp` reaches child or docs are narrowed)
4. Documented dup2 contract (`l_dup2(fd, L_STDOUT)` actually affects target if that contract remains public)

**Recommendation:** Land the already-safe redirection smoke coverage now. Do **not** enable Windows pipe smoke in CI until these gaps are fixed.

---

### L_Str Fat String Type Design

**By:** Dallas (Systems Dev)  
**Date:** 2026-03-31  
**Status:** Implemented

Added `L_Str` — a pointer+length fat string type backed by `L_Arena`. Placed in `l_os.h` alongside existing `L_Arena`/`L_Buf` types.

**Key Design Choices**
- **Zero-copy slicing:** `l_str_sub`, `l_str_trim`, `l_str_ltrim`, `l_str_rtrim` return views into original data — no allocation
- **Arena-only allocation:** All functions that produce new string data (`l_str_dup`, `l_str_cat`, `l_str_split`, `l_str_join`, `l_str_upper`, `l_str_lower`) take an `L_Arena*` — no malloc/free
- **L_Buf bridge:** `l_buf_push_str`, `l_buf_push_cstr`, `l_buf_push_int`, `l_buf_as_str` connect L_Str with the existing L_Buf growable buffer
- **Split is zero-copy for data:** `l_str_split` allocates the `L_Str[]` array in the arena but each element points into the original string
- **No NULL macro:** Uses `(void *)0` / `(char *)0` throughout for freestanding compliance

**Test Coverage (by Lambert)**
- 76 test assertions across 7 functions: constructors, comparison, slicing/search, arena ops, split/join, case conversion, buf helpers
- Always use `l_str_eq()` for L_Str comparisons (not null-terminated, can't use l_strcmp on .data)
- Exception: `l_str_cstr()` results are null-terminated, so `l_strcmp()` is safe there
- Arena lifecycle isolated per test group with 4096-byte arenas

**Impact**
- Any code using `L_Arena` can now build strings without malloc
- CI pass: 22/22 targets ✓

---

### Windows clang / l_poll Fix Decision

**Author:** Lambert (Tester)  
**Date:** 2026-04-01  
**Status:** Accepted

**Decision:** Accept Dallas's Windows `l_poll` fix.

**Root Cause:**
`l_poll()` on Windows was returning `-1` on a listening socket because the old Windows path fed a Winsock `SOCKET` directly into `WaitForMultipleObjects`, which only works on waitable kernel handles — not Winsock sockets.

**Fix Applied:**
- Socket-backed polling now correctly goes through `select()` on Windows
- Non-socket Windows polling (handles and pipes) keeps the existing `WaitForMultipleObjects` path (no regression)
- `test/test_net.c` hostname and loopback TCP/UDP regressions now pass

**Validation:**
- `build.bat` passes with Windows clang
- `test_all.bat` passes
- `verify.bat` passes
- `bin\test_net.exe` passes after fresh rebuild
- Minimal repro against old header fails (`l_poll` → `-1`)
- Minimal repro against current `l_os.h` passes

**Assessment:** Correctly scoped, no side effects, ready for integration.

---

### L_Str Demo Refactoring: Where It Helps, Where It Doesn't

**Author:** Dallas  
**Date:** 2026-03-31  
**Status:** Implemented

Refactored led.c and grep.c to use L_Buf helpers (`l_buf_push_cstr`, `l_buf_push_int`). Left sh.c unchanged.

**Rationale**
- **led.c:** Had ugly char-by-char status message construction (`msg[p++] = 'l'; msg[p++] = 'i'; ...`). Replaced with `l_buf_push_cstr`/`l_buf_push_int` — same behavior, readable code
- **grep.c:** Made 4 separate `write()` syscalls per match line. Batching into an L_Buf and writing once reduces syscall overhead
- **sh.c:** Left alone because `parse_line` does destructive in-place tokenization (inserts null terminators, handles quoted arguments). L_Str's zero-copy views don't help with mutation. The `eputs` error pattern is already more concise than L_Buf init/push/write/free

**Verify Pattern Fix**
Tightened the Taskfile verify `strings | grep` check. Old pattern matched common English words in string literals ("freestanding" → "free", "buf printf" → "printf"). New pattern only matches actual glibc linkage artifacts (`__libc`, `@GLIBC`, `ld-linux`, `libc.so`).

**Impact**
- CI pass: 22/22 targets ✓

---

### PR #59 Review: feat: add l_rename and l_access

**Reviewer:** Ripley (Lead)  
**Date:** 2025-07-25  
**Verdict:** APPROVE WITH NOTES

PR adds `l_rename` and `l_access` for Linux x86_64, AArch64, ARM32, and Windows. Includes constants, override macros, declarations, and tests. README updated.

**Correctness:** ✅
- Syscall numbers verified correct (x86_64 renameat: 264, aarch64: 38, ARM32: 329, etc.)
- Windows implementations correct: `l_rename` uses `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING`; `l_access` uses `GetFileAttributesW`.
- ARM safety: No 64-bit division.

**Convention compliance:** ✅
- `inline` (not `static inline`)
- `l_` prefix
- Override macros present
- Doc comments match pattern

**Merge conflicts:** LOW RISK — insertion points are still valid on main.

**Notes (non-blocking):**
1. Windows `l_access` limitations: `X_OK` returns success if file exists (Windows has no execute bit — acceptable platform limitation).
2. Spurious blank line in test_stat (cosmetic).
3. Test coverage adequate but could be richer (missing: rename nonexistent, rename over existing, W_OK/X_OK testing, directory rename).

**Decision:** APPROVE WITH NOTES — Code is correct, follows conventions, should merge cleanly.

---

### Review: Test Portability Analysis (Ripley)

**Author:** Ripley (Lead)  
**Date:** 2026-03-26  
**Status:** Ready for Review  
**Scope:** Analysis only (no implementation)

Analyzed platform-specific preprocessor conditionals in test suite. Found three distinct categories:

**Category 1: Platform-Specific Code That Should STAY** (real OS differences):
- Binary naming and path separators (different platforms produce differently-named binaries)
- File path separators in shell (Windows vs. Unix PATH/directory separators)
- Home directory fallback (Unix `$HOME` vs. Windows `$USERPROFILE`)
- Syscall numbers for pselect6 (Linux architecture-specific)

**Category 2: Platform-Specific Code That Should Be Hidden Behind Library Primitives** (API gaps):
- Environment variable enumeration (need `l_envp_first()` / `l_envp_next()`)
- Process spawning with platform-specific invocation (should use test helpers)
- Windows executable extension appending (reasonable to keep in sh.c for now)

**Category 3: Platform-Specific Code That Should Move to Test Helpers** (test infrastructure):
- File descriptor readiness check with syscalls (move to dedicated test helper)
- Directory creation test setup (remove once `l_mkdir()` is ported to Windows)
- Process spawning platform dispatch (extract into test helper `test_spawn_sh()`)

**Recommendations:**
- **Immediate (Phase 1):** Acknowledge most platform-specific code is acceptable.
- **Phase 2–3:** Extract test helpers for subprocess spawning; add library helper for environment iteration; port `l_mkdir()` to Windows.

**Key Design Principle:** Do NOT hide real OS differences behind a false abstraction. But if the library has an abstraction (e.g., `l_mkdir()`), all callers should use it — never raw Win32 API calls.

---

### Windows Process and Stdio Abstraction Redesign (Ripley scoping document)

**Lead:** Ripley  
**Date:** 2025-07-25  
**Status:** Scoped—Ready for Team Implementation  
**Priority:** High  
**Effort:** High  
**Risk:** High

**Problem:** Three critical functions fail to provide portable, predictable behavior:
- **`l_dup2(oldfd, newfd)`** — Cannot atomically reassign a file descriptor to a specific slot like Unix `dup2()`.
- **`l_spawn(path, argv, envp)`** — Ignores both `path` and `envp`.
- **Shell pipe/redirection logic in `test/sh.c`** — Falls back to raw Win32 instead of using library primitives.

**Two Viable Approaches:**

**Approach A: Descriptor Table + Descriptor Remapping (Unix-like Semantics Everywhere)**
- Implement a fixed descriptor table on Windows; `l_dup2()` atomically reassigns slots.
- `test/sh.c` uses same pattern on Windows and Unix.
- **Advantages:** Full Unix semantics, self-hosting, future-proof.
- **Disadvantages:** Adds table lookup overhead, changes `L_FD` semantics, more complex.

**Approach B: Spawn-with-Stdio API (Windows-Native Semantics, Explicit API)**
- New function `l_spawn_with_stdio(path, argv, envp, stdin_fd, stdout_fd, stderr_fd)`.
- `l_dup2` remains as-is; callers use new API for redirection.
- **Advantages:** No breaking changes, simpler, less overhead, clearer intent.
- **Disadvantages:** Two code paths to maintain, callers must understand platform differences.

**Recommendation:** **Approach A** — for semantic purity and self-hosting. However, Approach B also valid if team prefers minimal abstraction cost.

**Success Criteria (after implementation):**
1. `l_spawn` on Windows accepts and uses the `path` parameter
2. `l_spawn` on Windows accepts and applies the `envp` parameter
3. `l_dup2` on Windows either remaps atomically (Approach A) or is supplemented by `l_spawn_with_stdio` (Approach B)
4. `test/sh.c` no longer includes raw Win32 calls
5. New tests prove single-pipe execution, redirection, and environment passing work on Windows
6. Full `.\ci.ps1` passes on all platforms

---

### Decision: Smoke-Test Scope for Showcase Programs

**Date:** 2026-03-25  
**Status:** Decided

**Scope:**

**1. Actively Smoke-Tested (CI-gated):**
Non-interactive binaries with deterministic, quick tests: `base64`, `checksum`, `countlines`, `grep`, `hexdump`, `ls`, `printenv`, `sort`, `upper`, `wc`

**2. Minimal-Coverage (stable path only):**
- `sh` — tested via `--help` only (no full shell scripting in CI)
- `led` — tested via stable usage path only (no interactive editing in CI)

**3. Build-Only:**
- `snake` — interactive game; no automation possible without emulation

**Rationale:**
- Actively smoke-tested prove that `l_os.h` abstractions work for real file I/O, text processing, argument parsing.
- Minimal-coverage programs are valuable but inherently harder to test deterministically.
- Build-only avoids flaky tests while still proving they compile and link.

**Consensus:** Team agrees this scope is the right balance of regression detection and CI reliability.

---

### Windows Stdio Model: Approach B/Hybrid (l_spawn_stdio)

**Date:** 2026-03-26  
**Status:** Implemented

**Decision:** Instead of forcing Unix descriptor semantics onto Windows HANDLEs (Approach A — full descriptor table), we delivered **Approach B/hybrid**: an explicit spawn-with-stdio API.

**Implementation:**
- `l_spawn_stdio(const char *cmd, L_FD stdin, L_FD stdout, L_FD stderr, const char *workdir)` spawns a process with explicit stdio redirection.
- On Windows: HANDLEs passed directly to `CreateProcessW`; `stdin`, `stdout`, `stderr` are `L_FD` (can be file descriptors or HANDLEs).
- On Unix: maps file descriptors through `dup2` before forking.
- Both platforms: return child PID or `-1` on failure.

**Rationale:**
- **Approach A (full descriptor table):** Would require shadow fd→HANDLE table with complex cross-platform plumbing. High complexity, significant binary size risk.
- **Approach B (explicit spawn-with-stdio):** Direct HANDLE→fd mapping at spawn boundary. Simpler, smaller, no shadow table.

**Impact:**
- Cross-platform shell and process support now works without leaking abstraction on Windows.
- `test/sh.c` simplified: no longer hand-rolls Win32 `CreateProcessW` + `DuplicateHandle` logic.
- Windows redirection and single-pipe tests passing.
- Unix child-fd leak fixed; full CI green.

**Trade-offs:**
- Callers cannot use `dup2`-style descriptor manipulation as a general-purpose escape hatch. Instead, they call `l_spawn_stdio` with explicit redirection.
- This is a reasonable constraint for a freestanding library and aligns well with actual usage in shell and process utilities.


### Linux setenv: Static Pool vs Dynamic Allocation

**Date:** 2026-03-31  
**Status:** Implemented  

Use a static pool (128 entries, 8KB string buffer) for l_setenv/l_unsetenv on Linux instead of attempting dynamic allocation.

**Rationale:** In freestanding mode we have no malloc. Using l_mmap per entry would be wasteful. A fixed pool of 128 env vars and 8KB for strings covers realistic use cases while keeping the implementation simple. First l_setenv call copies the original l_envp pointers into the pool and repoints l_envp to it.

**Trade-off:** Programs needing >128 env vars or >8KB of new env string data will get -1 returns. This is acceptable for a freestanding library.

---

### AArch64 poll: ppoll Instead of poll

**Date:** 2026-03-31  
**Status:** Implemented  

Use ppoll (NR=73) on AArch64 instead of poll, since AArch64 Linux has no poll syscall.

**Rationale:** AArch64 kernel only exposes ppoll. We convert timeout_ms to a timespec and pass NULL for the signal mask to get poll-equivalent behavior.

---

### Signal Handling: Minimal rt_sigaction, No Restorer

**Date:** 2026-03-31  
**Status:** Implemented  

Use rt_sigaction with flags=0 (no SA_RESTORER) on Linux. On Windows, only support SIGINT/SIGTERM via SetConsoleCtrlHandler.

**Rationale:** Full sigaction with custom restorer trampoline (rt_sigreturn) is complex and architecture-specific. The simple approach of flags=0 works on modern kernels for basic signal handler registration. Windows has no concept of Unix signals beyond the console ctrl handler.

---

### L_Map: No Resize

**Date:** 2026-03-31  
**Status:** Implemented  

L_Map is fixed-capacity (set at init, rounded to power of 2). No resize or rehash.

**Rationale:** Arena-backed allocation makes resize impractical (arena doesn't support free). Users must estimate capacity upfront. 75% load factor limit prevents excessive probing.

---

### l_localtime: Simple TZ Parsing

**Date:** 2026-03-31  
**Status:** Implemented  

On Linux, l_localtime parses simple TZ env var formats (e.g., "UTC+5", "EST5") or defaults to UTC. On Windows, uses GetTimeZoneInformation for full DST support.

**Rationale:** Reading /etc/localtime (Olson TZ database binary format) is complex. Simple TZ parsing covers the common case. Users needing precise timezone handling can use l_gmtime and apply their own offset.

---

### 2026-04-13: Console Graphics — Terminal Pixel Backend (backend=2)

**By:** Ripley (Lead), Dallas (Implementation), Lambert (Testing)  
**Date:** 2026-04-13T09:40:00Z  
**Status:** Implemented

**What:** Added terminal graphics backend (backend=2) to l_gfx.h. Renders pixel buffers using Unicode half-block characters (▀ U+2580) with 24-bit ANSI truecolor escape sequences. All existing drawing primitives, l_ui.h, and l_img.h work unchanged.

**Design:**
- **Linux:** Auto-fallback chain: X11 → framebuffer → terminal (if stdin/stdout are TTYs)
- **Windows:** Opt-in via `L_GFX_TERM=1` environment variable (GDI remains default)
- **Rendering:** Half-block characters + ANSI escape sequences (24-bit RGB)
- **Optimization:** Skip redundant ANSI sequences per-cell (5-15KB typical frame vs. 48KB naive)
- **L_Canvas Windows:** Added `backend`, `saved_tty`, `term_buf`, `term_buf_size` fields
- **L_Canvas Linux:** Added `term_buf`, `term_buf_size` fields
- **Platform helpers:** `l_ansi_color_rgb()` in l_os.h; `l_term_flush_pixels()`, `l_term_canvas_init()`, `l_term_canvas_cleanup()` in l_gfx.h

**Why:** Existing architecture cleanly separates pixel buffer operations from display backend. Terminal rendering enables graphics and UI widgets in terminal contexts (SSH, tmux, CI logs) with minimal effort. All drawing code works unchanged.

**Cross-Platform:** Supports all modern terminals (Windows Terminal, ConEmu, cmd.exe on Windows; gnome-terminal, kitty, alacritty, xterm on Linux). ANSI 24-bit color widely supported.

**Impact:**
- Zero changes to existing drawing code or l_ui.h
- All tests pass unchanged
- Terminal backend fully functional and tested (30 assertions, test_term_gfx.c)
- `examples/term_demo.c` demonstrates capability

---

### 2026-04-01: Windows Cross-Target Compiler Aliasing

**By:** Dallas  
**Date:** 2026-04-01  
**Status:** Implemented

**What:** Normalize symbolic `gcc` inputs in `Taskfile` cross-build functions. `build_arm` maps `gcc` → `arm-linux-gnueabihf-gcc`, `build_aarch64` maps `gcc` → `aarch64-linux-gnu-gcc`. Also tighten `verify_arm` to require `ELF 32-bit` verification.

**Why:** Windows CI and ad-hoc WSL invocations use high-level compiler selectors. Host `gcc` compiling to `bin/*.armhf` produced mislabeled host binaries that later failed under `qemu-arm` with `Invalid ELF image for this architecture`.

**Impact:** All ARM/AArch64 cross-compilation now produces correct ELF bitness. CI catches architecture mismatches.

---

### 2026-04-01T12:53:03.936Z: User Directive

**By:** Luca Bolognese (via Copilot)  
**Status:** Directive

Fix everything; don't leave anything local. (Captured for team memory.)

---

## Governance

- All meaningful changes require team consensus
- Document architectural decisions here
- Keep history focused on work, decisions focused on direction

