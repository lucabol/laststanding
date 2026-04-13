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

## Work Session — 2026-03-26T14:00:00Z

Reviewed draft PR #61 (l_strpbrk + l_chmod). Verdict: **APPROVED**. Both functions fit the freestanding architecture, compile cleanly, and have adequate test coverage. Windows `l_chmod` takes a pragmatic write-bit-only model (standard limitation for cross-platform code targeting NTFS). Syscall numbers for ARM/AArch64 follow the existing pattern used in `l_rename` and `l_access` (already validated in CI).

Detailed review written to `.squad/decisions/inbox/ripley-pr61-review.md`.

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->
- **Windows ERRORLEVEL gotcha:** `l_exit(-1)` calls `ExitProcess(-1)` on Windows, setting ERRORLEVEL to -1 (signed). Batch `if errorlevel 1` tests >= 1 (signed), so it misses -1. Linux `l_exit` masks with `& 255`, so exit(-1) → 255 which bash catches. Any Windows batch test runner must use `!ERRORLEVEL! neq 0` with `enabledelayedexpansion`.
- **Taskfile `test/*` glob pattern:** The Taskfile uses `for f in test/*` (not `test/*.c`). CI workflows matching this pattern are consistent with the project. Currently test/ only contains .c files.
- **`test_all.bat` doesn't check error codes:** It runs all exes but doesn't detect failures. CI workflows should NOT rely on it for pass/fail gating.
- **PR review on own repos:** GitHub API rejects `--approve` and `--request-changes` on your own PRs. Use `gh pr comment` for detailed reviews instead.
- **2026-03-12 — Dallas follow-up fixes.** All PR #27 blocking issues resolved: (1) `build.ps1` CRLF now pure PowerShell (ReadAllBytes/WriteAllBytes); (2) l_os.h x86_64/AArch64 syscall statement expressions + startup asm; (3) `verify.bat` exit code. Tests passing across all platforms.
- **Windows chmod limitation:** NTFS has no user/group/other permissions. `l_chmod` on Windows exposes the write-bit only (`S_IWUSR`, 0200), mapped to the `FILE_ATTRIBUTE_READONLY` flag. This is pragmatic for freestanding code — don't pretend to support features that don't exist. Document this if a future API needs finer control.
- **Composing string functions:** `l_strtok_r` (PR #74) composes cleanly from `l_strspn` + `l_strcspn`. This is the preferred pattern — build higher-level string functions on existing primitives rather than reimplementing character scanning. Keeps the codebase DRY and reduces the surface for bugs.

## Work Session — 2026-07-25 (DNS API Review)

Reviewed the full socket API surface and designed the DNS resolution API. Decision written to `.squad/decisions/inbox/ripley-dns-api.md`.

**Verdict:** Approved a single-function API: `int l_resolve(const char *hostname, char *ip_out)`. Rejected `l_getaddrinfo`, auto-resolve inside `l_socket_connect`, and separate `l_dns_server`/`l_dns_query` decomposition.

**Key architecture decisions:**
- Linux: Build on existing UDP primitives (`l_socket_udp`, `l_socket_sendto`, `l_socket_recvfrom`). Parse `/etc/resolv.conf`. No new syscalls.
- Windows: Use `getaddrinfo` from ws2_32.dll (already linked for sockets). Pragmatic, same as `l_chmod` approach.
- Gate behind `L_WITHSOCKETS` — no separate `L_WITHDNS` guard.
- IP passthrough: if input is already a dotted-quad, copy and return 0.
- v1 scope: No CNAME chasing, no caching, no IPv6.
- Flagged `l_inet_addr("0.0.0.0")` returning 0 as an edge case Dallas needs to handle.

## Learnings

- **Socket API is L_WITHSOCKETS-gated:** All socket code (TCP, UDP, poll, helpers) lives behind `#ifdef L_WITHSOCKETS` / `#endif` blocks at lines ~17, 259, 775, 2959, 4400, 5611, 5742 of `l_os.h`. New socket features must follow the same pattern.
- **Windows sockets use Winsock2 directly:** `ws2_32.dll` functions called directly — `socket`, `connect`, `send`, `recv`, `getaddrinfo` all available without new DLL deps. `l_wsa_init()` ensures one-time WSAStartup.
- **DNS composes from existing UDP primitives on Linux:** `l_socket_udp` + `l_socket_sendto` + `l_socket_recvfrom` + `l_poll` provide everything needed for raw DNS queries. No new syscalls.
- **`l_inet_addr` returns 0 for both error and `0.0.0.0`:** Any code that uses `l_inet_addr` return value as an error indicator has a bug for `0.0.0.0`. This affects `l_socket_connect`, `l_socket_sendto`, and the proposed `l_resolve` passthrough check.
- **http_get.c is the reference socket client:** Located at `test/http_get.c`, demonstrates TCP connect → send → poll-recv loop. Will need updating once DNS lands to accept hostnames.

## Work Session — 2026-07-25 (Console Graphics Feasibility Analysis)

Performed a thorough architecture analysis of `l_gfx.h` to assess whether it can support console/terminal rendering in addition to graphical displays.

**Key findings:**
- `l_gfx.h` has a clean 3-backend architecture: Windows GDI, X11 wire protocol, Linux framebuffer. Backend selected at `l_canvas_open` time via `c->backend` field (0=fb, 1=X11).
- The core abstraction (`L_Canvas`) is a **pixel buffer** (`uint32_t *pixels`, ARGB). All drawing primitives (`l_pixel`, `l_fill_rect`, `l_circle`, `l_draw_text`, `l_blit`) operate purely on this buffer — they are platform-independent.
- The backend only matters for: (1) `l_canvas_open` — allocating the buffer + connecting to display, (2) `l_canvas_flush` — copying pixels to screen, (3) `l_canvas_key`/`l_canvas_mouse` — input, (4) `l_canvas_close` — cleanup.
- `l_os.h` already provides: `l_term_raw`, `l_term_restore`, `l_term_size`, `l_read_nonblock`, `l_ansi_move`, `l_ansi_color`, `L_ANSI_CLEAR`, `L_ANSI_HOME`, `L_ANSI_HIDE_CUR`, `L_ANSI_SHOW_CUR` — all cross-platform (Linux+Windows+WASI stubs).
- Windows `l_term_raw` already enables `ENABLE_VIRTUAL_TERMINAL_PROCESSING` for ANSI escape support.
- A terminal backend (backend=2) could be added by: allocating an in-memory pixel buffer, and in `l_canvas_flush`, converting pixels to terminal output using Unicode half-block characters (▀▄█) with 24-bit ANSI color sequences (`\033[38;2;R;G;Bm` / `\033[48;2;R;G;Bm`).
- Resolution: each terminal cell = 1 char wide × 2 pixels tall (using ▀/▄). An 80×24 terminal → 80×48 pixel canvas. A 160-col terminal → 160×96 pixels — enough for simple UI.
- `l_ui.h` would work unchanged because it operates entirely on `L_Canvas` via drawing primitives. No modifications needed.

## Learnings

- **l_gfx.h backend architecture:** Backend dispatch is via `c->backend` integer field. Currently: 0=framebuffer, 1=X11 (Linux); Windows has no backend field (separate `#ifdef _WIN32` compile-time split). Adding backend=2 (terminal) on Linux is straightforward. On Windows, would need a runtime backend field added to the `#ifdef _WIN32` struct.
- **Drawing primitives are backend-agnostic:** All `l_pixel`, `l_fill_rect`, `l_circle`, `l_line`, `l_draw_text`, `l_blit`, `l_blit_alpha` operate purely on the `uint32_t *pixels` buffer. No backend-specific code. This means any new backend only needs to implement the 5 platform functions (open/close/alive/flush/key/mouse).
- **ANSI 24-bit color is already half-supported:** `l_ansi_color` in l_os.h only does 8-color (0-7). For terminal pixel rendering, we'd need `\033[38;2;R;G;Bm` (truecolor) — a new `l_ansi_color_rgb` helper.
- **Terminal backend has zero new syscall requirements:** All needed primitives (`l_write` to stdout, `l_term_raw`, `l_term_size`, `l_read_nonblock`, `l_ansi_move`) already exist in l_os.h across all platforms.
