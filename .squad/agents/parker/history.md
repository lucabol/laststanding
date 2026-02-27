# Parker — History

## Core Context
- **Project:** laststanding — freestanding C runtime, no libc, direct syscalls
- **Stack:** C (freestanding), inline asm, GCC/Clang/MinGW, Win32 API
- **User:** Luca Bolognese
- **Key file:** l_os.h (single header, ~900 lines)
- **Architectures:** x86_64, ARM (armhf), AArch64, Windows
- **Syscall macros:** my_syscall0–my_syscall6 per arch
- **Build flags:** -Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin

## Learnings
