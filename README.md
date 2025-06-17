# laststanding

A minimal C runtime and test suite for exploring freestanding, static, and cross-platform C code. This project provides:

- Minimal implementations of common C library functions (with `l_` prefix) in `l_os.h`
- Direct syscall wrappers for Linux (x86_64, ARM, AArch64) and Windows
- Test programs for verifying correctness and portability
- Build scripts for Linux and Windows

## Features
- No dependency on libc or glibc: all binaries are statically linked and freestanding
- Cross-platform syscall support: x86_64, ARM, AArch64, and Windows
- Simple build and test automation via `Taskfile` (bash) and `test_all.bat` (Windows)

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

## Adding New Tests
Add new `.c` files to the `test/` directory. They will be automatically built and run by the scripts.

## License
MIT License
