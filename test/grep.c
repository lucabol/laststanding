#define L_MAINFILE
#include "l_os.h"

// Minimal grep: filters lines matching a substring pattern
// Usage: grep <pattern> <filename>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        puts("Usage: grep <pattern> <filename>\n");
        return 0;
    }

    char *pattern = argv[1];
    L_FD fd = open_read(argv[2]);
    exitif(fd < 0, 1, "Error: cannot open file\n");

    char buf[4096];
    char line[4096];
    int line_len = 0;
    int line_num = 0;
    int matches = 0;
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n' || line_len >= (int)sizeof(line) - 1) {
                line[line_len] = '\0';
                line_num++;

                if (strstr(line, pattern) != NULL) {
                    matches++;
                    char num[12];
                    itoa(line_num, num, 10);
                    write(STDOUT, num, strlen(num));
                    write(STDOUT, ":", 1);
                    write(STDOUT, line, line_len);
                    write(STDOUT, "\n", 1);
                }
                line_len = 0;
            } else {
                line[line_len++] = buf[i];
            }
        }
    }

    // Handle last line without trailing newline
    if (line_len > 0) {
        line[line_len] = '\0';
        line_num++;
        if (strstr(line, pattern) != NULL) {
            matches++;
            char num[12];
            itoa(line_num, num, 10);
            write(STDOUT, num, strlen(num));
            write(STDOUT, ":", 1);
            write(STDOUT, line, line_len);
            write(STDOUT, "\n", 1);
        }
    }

    close(fd);
    return matches > 0 ? 0 : 1;
}
