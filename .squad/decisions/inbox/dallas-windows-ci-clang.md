# Decision: Windows CI uses clang on GHA windows-latest

**Author:** Dallas
**Date:** 2026-03-11
**Issue:** #14

## Context

The Windows build (`build.bat`) already uses `clang` with `-ffreestanding -lkernel32`. GitHub Actions `windows-latest` runners ship with LLVM/clang pre-installed, so no additional toolchain install step is needed.

## Decision

The Windows CI workflow (`windows-ci.yml`) uses the runner's built-in clang rather than installing MSVC or a separate LLVM package. Steps use `shell: cmd` with `call` to run the existing batch scripts directly.

## Implications

- If the runner image drops clang in the future, we'll need to add an install step.
- MSVC-specific tools like `dumpbin` may or may not be on PATH; `verify.bat` already handles that gracefully with fallbacks.
