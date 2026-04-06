---
name: "windows-socket-poll-regression"
description: "Reproduce and validate Windows socket poll failures without disturbing the working tree"
domain: "testing"
confidence: "high"
source: "manual"
---

## Context

When a Windows networking regression appears in this repo, the first failing symptom may be deep inside `l_poll()` rather than in the higher-level socket helper that the test names. A minimal loopback repro is faster and more trustworthy than debugging through the full demo stack.

## Pattern

### 1. Pin the old implementation from git

Write the previous header version to a scratch file in the repo, for example:

- `git show <bad-or-parent-commit>:l_os.h > l_os_old.h`

Do not edit tracked sources just to reproduce the old behavior.

### 2. Build a tiny loopback-socket repro

Use a one-file program that:

- defines `L_WITHSOCKETS` and `L_MAINFILE`
- opens a TCP listener on loopback
- discovers the assigned port with `getsockname`
- connects a client socket to `127.0.0.1`
- calls `l_poll()` on the listening socket
- exits 0 only if `l_poll()` reports readiness

This isolates the polling contract from the rest of the test matrix.

### 3. Expect the old Windows bug to fail as `ready == -1`

If the old implementation routes Winsock sockets through `WaitForMultipleObjects`, the minimal repro should fail before accept/read/write logic matters. In practice the concrete signal is:

- `l_poll(...) == -1`

That identifies the first real failure and avoids blaming later cascading assertions.

### 4. Re-run the exact repro against current `l_os.h`

Keep the repro source the same and only switch the included header from the old snapshot to the current one. If the fix is correct, the same program should return success.

### 5. Then validate the real regression tests

After the minimal repro passes, run the higher-level Windows checks:

- `build.bat`
- `test_all.bat`
- `verify.bat`
- `bin\test_net.exe`

This proves both the primitive and the user-facing coverage are fixed.

## Why this works

It separates three questions cleanly:

1. Did the old code really fail?
2. What was the first concrete fault?
3. Does the current change fix that exact fault without regressing the rest of Windows?

## Anti-Patterns

- **Only reading the larger test failure** — you risk mistaking a downstream assertion for the real bug.
- **Editing tracked files to recreate the old behavior** — makes validation noisy and risky.
- **Stopping after `test_all.bat`** — that default suite may miss socket-only regressions unless `test_net.exe` is also run.
