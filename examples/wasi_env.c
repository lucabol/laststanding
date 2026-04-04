// WASI Environment — prints command-line args and environment variables.
// Demonstrates WASI argument and environment access.
//
// Build: clang --target=wasm32-wasi --sysroot=/path/to/wasi-sdk/share/wasi-sysroot
//        -I.. -O2 -ffreestanding -nostdlib -o wasi_env.wasm wasi_env.c
// Run:   wasmtime --env FOO=bar --env BAZ=qux wasi_env.wasm arg1 arg2

#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    puts("=== Command-line arguments ===\n");
    for (int i = 0; i < argc; i++) {
        puts("  argv[");
        char num[16];
        l_itoa(i, num, 10);
        puts(num);
        puts("] = ");
        puts(argv[i]);
        puts("\n");
    }

    puts("\n=== Environment variables ===\n");
    void *handle = l_env_start();
    void *iter = handle;
    char buf[4096];
    const char *entry;
    while ((entry = l_env_next(&iter, buf, sizeof(buf))) != NULL) {
        puts("  ");
        puts(entry);
        puts("\n");
    }
    l_env_end(handle);

    return 0;
}
