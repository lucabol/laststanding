# Squad Decisions

## Active Decisions

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
**Status:** Pending Decision  
**Issue:** #12

`l_strstr("", "")` returns `NULL`, while standard libc `strstr("", "")` returns a pointer to the empty haystack. This is because the implementation's `while (*s1)` loop doesn't execute when the haystack is empty.

**Impact:** Minor deviation from POSIX/C standard behavior. Won't affect typical usage but could surprise callers who depend on the standard contract.

**Assigned to:** Dallas to decide whether to fix the implementation (add a check for `len == 0` before the loop) or document as intentional.

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

## Governance

- All meaningful changes require team consensus
- Document architectural decisions here
- Keep history focused on work, decisions focused on direction
