# Cross-target compiler alias mapping

## When to use
When a build script accepts friendly compiler selectors like `gcc` / `clang` but emits cross-target artifacts (`.armhf`, `.aarch64`, Windows, etc.).

## Pattern
1. Normalize generic selectors to the real cross compiler early in the build function.
   - `gcc` -> `arm-linux-gnueabihf-gcc`
   - `gcc` -> `aarch64-linux-gnu-gcc`
2. Keep target-specific `clang` handling explicit with `--target` and `--sysroot`.
3. Validate the produced artifact architecture with `file`, not just the filename suffix.
4. Make verify checks assert the exact class/arch (`ELF 32-bit` + `ARM`, `ARM aarch64`, etc.).

## Why
Without normalization, host compilers can silently emit native binaries into cross-target filenames. The failure often appears later in runtime steps (`qemu-arm: Invalid ELF image for this architecture`) instead of at build time.

## Validation recipe
- Build with the friendly selector from Windows/WSL.
- Run `file bin/<name>.<suffix>` to confirm the real architecture.
- Run the target test command (`test_arm`, `test_aarch64`, or CI target wrapper).
