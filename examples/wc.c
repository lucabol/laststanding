#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"

// Minimal wc: counts lines, words, and bytes in a file (like Unix wc)
// Usage: wc <filename> [filename2 ...]

int main(int argc, char *argv[]) {
    if (argc < 2) {
        puts("Usage: wc <filename> [filename2 ...]\n");
        return 0;
    }

    long total_lines = 0, total_words = 0, total_bytes = 0;

    for (int f = 1; f < argc; f++) {
        L_FD fd = open_read(argv[f]);
        if (fd < 0) {
            puts("wc: cannot open ");
            puts(argv[f]);
            puts("\n");
            continue;
        }

        char buf[4096];
        ssize_t n;
        long lines = 0, words = 0, bytes = 0;
        int in_word = 0;

        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            bytes += n;
            for (int i = 0; i < n; i++) {
                if (buf[i] == '\n') lines++;
                if (buf[i] == ' ' || buf[i] == '\t' || buf[i] == '\n' || buf[i] == '\r') {
                    in_word = 0;
                } else if (!in_word) {
                    in_word = 1;
                    words++;
                }
            }
        }

        close(fd);

        l_printf("%8ld%8ld%8ld %s\n", lines, words, bytes, argv[f]);

        total_lines += lines;
        total_words += words;
        total_bytes += bytes;
    }

    if (argc > 2) {
        l_printf("%8ld%8ld%8ld total\n", total_lines, total_words, total_bytes);
    }

    return 0;
}
