// WASI Hello World — demonstrates basic output under WebAssembly/WASI.
//
// Build: clang --target=wasm32-wasi --sysroot=/path/to/wasi-sdk/share/wasi-sysroot
//        -I.. -O2 -ffreestanding -nostdlib -o wasi_hello.wasm wasi_hello.c
// Run:   wasmtime wasi_hello.wasm

#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    puts("Hello from WebAssembly!\n");
    return 0;
}
