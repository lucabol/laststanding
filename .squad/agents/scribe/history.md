# Project Context

- **Project:** laststanding
- **Created:** 2026-03-11

## Core Context

Agent Scribe initialized and ready for work.

## Recent Updates

📌 Team initialized on 2026-03-11

## Learnings

## Learnings

Initial setup complete.

### 2026-03-25: Showcase Smoke Tests Complete

**Milestone:** Phase 1 items 1–2 shipped.
- AArch64 CI coverage wired into build, test, and verification scripts.
- Deterministic smoke tests added for non-interactive showcase programs (base64, checksum, countlines, grep, hexdump, ls, printenv, sort, upper, wc).
- Snake and led remain build-only; sh tested through `--help`.
- **Key fix:** `test_all.bat` batch errorlevel handling corrected using delayed-expansion `!ERRORLEVEL! neq 0` to catch Windows `exit(-1)` failures.
- Full `ci.ps1` passing on Windows, Linux (gcc/clang), ARM32, and AArch64.

**Next:** Phase 2 — Windows process and stdio abstraction (l_dup2 descriptor mapping, l_spawn path/envp consistency, sh.c simplification).

### 2026-03-26: Windows Process/Stdio Complete (Approach B/Hybrid)

**Milestone:** Phase 2 shipped.
- **Design delivered:** `l_spawn_stdio(cmd, stdin, stdout, stderr, workdir)` — explicit spawn-with-stdio API (Approach B/hybrid).
- Avoided full descriptor-table complexity; instead offers direct Win32 HANDLE ↔ Unix fd mapping at the call boundary.
- Windows redirection and single-pipe tests added and passing on all platforms.
- `test/sh.c` simplified: removed hand-rolled Win32 redirection logic; now uses library primitives.
- Unix child-fd leak fixed; full cross-platform CI green after final fixes.
- Lambert confirmed functional readiness.

**Next:** Phase 3, item 1 — Portable primitives (Windows versions of `l_dup`, `l_lseek`, `l_mkdir`, `l_sched_yield`).
**Pending cleanup:** `docs-sync` (Phase 1, item 3) — README manual sections and assertion counts.
