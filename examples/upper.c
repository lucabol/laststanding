#define L_MAINFILE
#include "l_os.h"

// Minimal upper: converts file contents to uppercase, writes to stdout or file
// Usage: upper <input> [output]

static char to_upper(char c) {
    if (c >= 'a' && c <= 'z') return c - ('a' - 'A');
    return c;
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        puts("Usage: upper <input> [output]\n");
        return 0;
    }

    L_FD in_fd = open_read(argv[1]);
    exitif(in_fd < 0, 1, "Error: cannot open input file\n");

    L_FD out_fd = STDOUT;
    if (argc == 3) {
        out_fd = open_write(argv[2]);
        exitif(out_fd < 0, 1, "Error: cannot open output file\n");
    }

    char buf[4096];
    ssize_t n;

    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++)
            buf[i] = to_upper(buf[i]);
        write(out_fd, buf, n);
    }

    close(in_fd);
    if (out_fd != STDOUT) close(out_fd);

    return 0;
}
