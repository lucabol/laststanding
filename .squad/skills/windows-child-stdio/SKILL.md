---
name: "windows-child-stdio"
description: "Freestanding Win32 stdio built on descriptor slots plus explicit child-handle duplication"
domain: "systems"
confidence: "high"
source: "manual"
---

## Context

When a freestanding Windows codebase wants Unix-like redirection or `cmd1 | cmd2`, the unsafe default is to treat `L_FD` as a raw cast `HANDLE`, special-case `0/1/2`, and then make pipe handles inheritable for `CreateProcessW(..., TRUE, ...)`. That makes `dup2()` semantics inconsistent and can leak unused pipe ends into children.

## Pattern

### Keep a descriptor table above Win32 handles

Make Windows `L_FD` values stable descriptor slots owned by the library. Initialize slots `0/1/2` from duplicated stdio handles, and allocate later slots for `l_open*()` and `l_pipe()`.

### Keep parent-owned handles non-inheritable

Create pipes and files with the normal non-inheritable defaults in the parent process. Do not rely on global inheritable handles as ambient state.

### Make `dup2()` replace descriptor slots, not global process state

Implement `l_dup2(oldfd, newfd)` by duplicating the source `HANDLE` and storing that duplicate in `newfd`'s slot. That gives Unix-like slot remapping semantics even when stdout and stderr started as the same console handle.

### Expose `l_dup()` cross-platform for save/restore

Callers that want Unix-style `dup2()` + `spawn()` composition need a portable way to save the current stdin/stdout/stderr before temporarily remapping them. Make `l_dup()` available on Windows too by duplicating the underlying `HANDLE` into a fresh descriptor slot.

### Duplicate only the child's three stdio handles

Right before `CreateProcessW`, resolve the chosen descriptor slots to `HANDLE`s, duplicate just those three with `DuplicateHandle(..., TRUE, ...)`, and put only those inheritable copies into `STARTUPINFOW`.

### Pass the executable path separately from the command line

Use `lpApplicationName` for the resolved executable path and build the command line from `argv`. That keeps Windows path lookup and child `argv[0]` semantics from getting tangled together.

### Close non-stdio originals after Unix `dup2`

If the same helper also has a Unix `fork/dup2/execve` path, close the original redirected descriptors in the child after `dup2()`. Otherwise a pipeline can keep an extra writer open and delay EOF.

## Proof Consumer

- `l_os.h` — Windows descriptor table, cross-platform `l_dup`, `l_dup2`, `l_spawn_stdio`, and non-inheritable `l_pipe`
- `test/sh.c` — shell redirection and pipe execution via save/redirect/restore around plain `l_spawn()`
- `test/test.c` — arbitrary-slot `dup2()` coverage plus plain `l_spawn()` inheritance and pipeline checks
- `test/showcase_smoke.ps1` / `test/showcase_smoke.sh` — real shell single-pipe smoke coverage

## Anti-Patterns

- **Treating `L_FD` as a raw `HANDLE` on Windows** — breaks Unix-style `dup2()` semantics.
- **Saving stdio by guessing spare fd numbers** — brittle once Windows `L_FD` values stop being raw handles; prefer `l_dup()`.
- **Making every pipe inheritable by default** — easy to implement, easy to hang.
- **Using `SetStdHandle()` as the real fd model** — it mutates ambient process state instead of giving stable descriptor slots.
- **Ignoring `path` in `l_spawn` and hoping argv[0] resolves** — breaks the cross-platform contract.
- **Leaving the original pipe fd open after `dup2` on Unix** — reader EOF becomes timing-dependent.
