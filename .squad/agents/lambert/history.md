# Lambert — History

## Core Context
- **Project:** laststanding — freestanding C runtime, no libc, direct syscalls
- **Stack:** C (freestanding), inline asm, GCC/Clang/MinGW, Win32 API
- **User:** Luca Bolognese
- **Test macros:** TEST_ASSERT, TEST_FUNCTION, TEST_SECTION_PASS (in test/test.c)
- **Build/test:** ./Taskfile test (Linux), test_all.bat (Windows)
- **Verify:** ./Taskfile verify (check no stdlib deps)

## Learnings
