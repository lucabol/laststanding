# Orchestration Log — Dallas (Spawn Helpers)

**Agent:** Dallas (general-purpose)  
**Session:** 2026-03-25T12:30:00Z  
**Status:** Completed ✓

## Deliverables

### test/test.c — Self-Hosted Spawn Test Helpers

Added three command-line helper modes to the test binary itself, eliminating platform-specific OS commands:

- `--exit N` — Exit with code N (replaces `cmd.exe /c exit N` and `false`/`true`)
- `--echo-stderr MSG` — Write MSG to stderr and exit (replaces shell commands)
- `--hold-stdin` — Read all stdin and hold it; exit on EOF (for pipe/redirection testing)

### test_spawn_wait
Changed from using OS commands to helper modes:
- `before: os_spawn("cmd.exe /c exit 0")` → `after: os_spawn("./test --exit 0")`
- `before: os_spawn("/bin/false")` → `after: os_spawn("./test --exit 1")`

### test_spawn_inherits_dup2
Updated to use `--echo-stderr` instead of shell `>&2`:
- `before: os_spawn("sh -c 'echo spawndup >&2'")` → `after: os_spawn("./test --echo-stderr spawndup")`

### test_getcwd_chdir
Replaced OS-specific directory handling (hard-coded `C:\` vs `/`) with portable `l_mkdir`/`l_rmdir` temp directory pattern.

## Code Changes

- **test/test.c** — Removed 4 `#ifdef _WIN32` blocks
- **test/test.c** — Added `maybe_run_helper()` helper dispatch logic
- **Eliminated:** Dependency on `cmd.exe`, `/bin/sh`, `/bin/false`, `/bin/true`

## Verification

- **All 21 CI targets pass:**
  - Windows: MSVC, Clang (x64)
  - Linux: x86_64, ARM, AArch64 (GCC, Clang)
- **Cross-platform:** Output is byte-identical across all platforms (no CRLF conversion)
- **Reliability:** No external binary dependencies; works under QEMU and CI environments
- **Code quality:** Strict compile flags enforced

## Decisions Captured

- Self-hosted test helpers proven effective for cross-platform spawn testing
- Pattern consistent with existing `--spawn-stdio-close-stdout-helper` mode
- Output consistency more important than OS command re-use

## Commit

Commit hash: (included in batch commit)

### Message
```
test: self-hosted spawn helpers (--exit, --echo-stderr, --hold-stdin)

- Added --exit N helper mode to eliminate cmd.exe /c exit and false/true
- Added --echo-stderr MSG for cross-platform stderr redirection
- Added --hold-stdin for pipe/redirection testing
- Removed 4 #ifdef _WIN32 blocks from test/test.c
- All 21 CI targets pass; output byte-identical across platforms

Co-authored-by: Copilot <223556219+Copilot@users.noreply.github.com>
```

## Next Steps

- Monitor for additional test scenarios requiring self-hosted helper modes
- Document helper mode API in team wiki if other test files need similar patterns
