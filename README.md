# laststanding

A minimal C runtime and test suite for exploring freestanding, static, and cross-platform C code. This project provides:

- Minimal implementations of common C library functions (with `l_` prefix) in `l_os.h`
- Direct syscall wrappers for Linux (x86_64, ARM/32-bit, AArch64/64-bit ARM) and Windows
- Test programs for verifying correctness and portability
- Build scripts for Linux and Windows

## Features
- No dependency on libc or glibc: all binaries are statically linked and freestanding
- Cross-platform syscall support: x86_64, ARM, AArch64, and Windows
- Simple build and test automation via `Taskfile` (bash) and `test_all.bat` (Windows)
- AArch64 (64-bit ARM) fully supported and tested via QEMU

## Scope: What's Included and Not Included

### Included
- String and memory operations (`strlen`, `memcpy`, `strchr`, `memset`, etc.)
- Number conversion (`atoi`, `itoa`, etc.)
- File I/O operations (`open`, `read`, `write`, `close`, `fstat`, `seek`)
- Syscall wrappers for Linux and Windows
- Platform startup code and entry points for all supported architectures

### Not Included (by Design)
- `printf`/`sprintf` — use direct write syscalls or minimal formatting
- `malloc`/`free` — no dynamic memory allocation
- `getenv` — environment variable access
- `fork`/`exec` — process spawning and execution
- Networking functions — sockets and network I/O
- Multithreading primitives — threads, mutexes, condition variables

## Directory Structure
- `l_os.h` — Minimal C/OS abstraction header
- `test/` — Test programs for various functions
- `bin/` — Compiled test binaries
- `misc/` — Extra tools and experiments
- `Taskfile` — Bash build and test automation
- `test_all.bat` — Windows batch build and test script

## Building and Testing

### On Linux/macOS (with bash):
```sh
./Taskfile test
```

### On Windows (with Git Bash and MinGW):
```bat
test_all.bat
```

### Using ci.ps1 (PowerShell)

`ci.ps1` provides unified cross-platform CI from PowerShell. It builds, tests, and verifies across Windows, Linux (gcc + clang), and ARM (gcc + clang), delegating Linux/ARM targets to WSL.

**Parameters:**
- `-Target` — Build target: `windows`, `linux`, `arm`, or `all` (default: `all`)
- `-Action` — Action to perform: `build`, `test`, `verify`, or `all` (default: `all`)
- `-Compiler` — Compiler for Linux/ARM: `gcc`, `clang`, or `all` (default: `all` — runs both)
- `-OptLevel` — Optimization level 0–3 (default: 3)

**Examples:**
```powershell
# Build, test, and verify all targets with all compilers (default)
.\ci.ps1

# Build and test Windows only
.\ci.ps1 -Target windows -Action test

# Build Linux with clang only at -O2
.\ci.ps1 -Target linux -Action build -Compiler clang -OptLevel 2

# Test ARM binaries (via WSL + QEMU)
.\ci.ps1 -Target arm -Action test
```

> **Note:** `build.ps1` is kept as a backward-compatible wrapper that forwards to `ci.ps1`.

**Requirements:**
- Windows builds work natively (requires clang on PATH or Visual Studio Build Tools)
- Linux and ARM builds require WSL (Windows Subsystem for Linux)
- ARM target requires `arm-linux-gnueabihf-gcc` in WSL (provides sysroot for both gcc and clang cross-compilation)
- ARM clang cross-compilation requires `clang` installed in WSL

## Adding New Tests
Add new `.c` files to the `test/` directory. They will be automatically built and run by the scripts.

## License
MIT License
