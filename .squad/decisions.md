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

### l_strstr empty-string behavior

**From:** Lambert (Tester)  
**Date:** 2026-03-11  
**Status:** Pending Decision  
**Issue:** #12

`l_strstr("", "")` returns `NULL`, while standard libc `strstr("", "")` returns a pointer to the empty haystack. This is because the implementation's `while (*s1)` loop doesn't execute when the haystack is empty.

**Impact:** Minor deviation from POSIX/C standard behavior. Won't affect typical usage but could surprise callers who depend on the standard contract.

**Assigned to:** Dallas to decide whether to fix the implementation (add a check for `len == 0` before the loop) or document as intentional.

## Governance

- All meaningful changes require team consensus
- Document architectural decisions here
- Keep history focused on work, decisions focused on direction
