# Orchestration Log — Dallas (Exec Resolution & Cleanup)

**Agent:** Dallas (general-purpose)  
**Session:** 2026-03-25T12:30:00Z  
**Status:** Completed ✓

## Deliverables

### l_os.h — l_find_executable API

New cross-platform executable discovery function:

```c
int l_find_executable(const char *cmd, char *out, size_t outsz);
```

- Returns 1 if found (path written to `out`), 0 if not found
- Searches PATH with platform-appropriate separators (`;` on Windows, `:` on Unix)
- **Windows:** Tries `.exe`, `.bat`, `.com` extensions *before* bare name (critical for cross-compilation scenarios)
- **Unix:** Bare name only; no extension magic

### sh.c Portability Cleanup

Removed `sh_envp` global and related environment-passing infrastructure:

- `before:` Global `sh_envp`, `spawn_envp()` function, multiple `#ifdef __unix__` blocks
- `after:` Call `l_spawn(..., NULL)` which inherits environment on both platforms
- **Lines removed:** ~65 lines of platform-specific envp plumbing
- **#ifdef blocks removed:** 5 Unix-specific environment blocks (only HOME/USERPROFILE remains — genuine OS difference)

### test_find_executable

New test binary exercising all code paths:
- PATH search with multiple separators
- Extension handling on Windows (`.exe`, `.bat`, `.com`, bare)
- Not-found cases
- Buffer overflow protection

## Code Changes

- **l_os.h** — Added `l_find_executable` public function
- **l_os.h** — Added internal PATH search logic (platform-aware separators)
- **test/sh.c** — Removed `sh_envp` global variable
- **test/sh.c** — Removed `spawn_envp()` helper function
- **test/sh.c** — Removed 5 `#ifdef __unix__` blocks handling envp
- **test/test.c** — Added `test_find_executable()` comprehensive test

## Verification

- **All 21 CI targets pass**
- **Cross-compilation scenario:** Windows binary searches find `.exe` not extensionless Linux binaries
- **Baseline:** No regression; all existing tests still passing
- **Code quality:** Strict compile flags enforced

## Decisions Captured

- **Extension search order (Windows):** Try `.exe`, `.bat`, `.com`, then bare name. This is critical because cross-compiled binaries may coexist in same directory.
- **Platform asymmetry:** Unix has no extension concept; Windows supports multiple executable types. Design embraces platform differences where unavoidable.
- **Simplified envp handling:** `l_spawn(cmd, NULL)` inheritance is cleaner than explicit global envp passing.

## Design Rationale

- **Why try extensions first:** In cross-compile scenarios, a directory might contain:
  - `./bin/foo` (Linux ARM binary, no extension)
  - `./bin/foo.exe` (Windows x64 binary)
  - Both are valid; bare name should match platform-appropriate binary
- **Why extend _start capture (deferred):** Full `l_getenv` implementation deferred (documented in decisions.md) but executor resolution is independent and high-value.

## Commit

Commit hash: (included in batch commit)

### Message
```
api: add l_find_executable; sh.c envp cleanup

- Added l_find_executable() for cross-platform PATH resolution
- Windows: tries .exe/.bat/.com before bare name (critical for cross-compile)
- Removed sh.c sh_envp global and 5 #ifdef __unix__ blocks
- l_spawn(..., NULL) inherits env on both platforms now
- All 21 CI targets pass

Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>
```

## Next Steps

- Monitor for other spawn-related code that could benefit from `l_find_executable`
- Document cross-compilation implications in team guide
- Consider extracting PATH search as separate utility if needed elsewhere
