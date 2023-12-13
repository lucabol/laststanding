REM Works in x64 Native Tools Command Prompt
clang -I. test\countlines.c -O3 -lkernel32 -ffreestanding -o bin\countlines.exe
clang -I. test\test.c -O3 -lkernel32 -ffreestanding -o bin\test.exe
