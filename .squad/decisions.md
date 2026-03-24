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
