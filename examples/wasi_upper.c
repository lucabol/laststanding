// WASI Uppercase — reads a file and outputs uppercase text.
// Demonstrates file I/O under WASI (requires --dir . for file access).
//
// Build: clang --target=wasm32-wasi --sysroot=/path/to/wasi-sdk/share/wasi-sysroot
//        -I.. -O2 -ffreestanding -nostdlib -o wasi_upper.wasm wasi_upper.c
// Run:   wasmtime --dir . wasi_upper.wasm input.txt

#define L_MAINFILE
#include "l_os.h"

static char to_upper(char c) {
    if (c >= 'a' && c <= 'z') return c - ('a' - 'A');
    return c;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Usage: wasi_upper <input-file>\n");
        return 1;
    }

    L_FD in_fd = l_open_read(argv[1]);
    if (in_fd < 0) {
        puts("Error: cannot open input file\n");
        return 1;
    }

    char buf[4096];
    ssize_t n;

    while ((n = l_read(in_fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++)
            buf[i] = to_upper(buf[i]);
        l_write(L_STDOUT, buf, (size_t)n);
    }

    l_close(in_fd);
    return 0;
}
