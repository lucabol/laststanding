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

    L_Buf line, out;
    l_buf_init(&line);
    l_buf_init(&out);
    char buf[4096];
    int line_num = 0;
    int matches = 0;
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                char nul = '\0';
                l_buf_push(&line, &nul, 1);
                line_num++;

                if (strstr((char *)line.data, pattern) != NULL) {
                    matches++;
                    l_buf_push_int(&out, line_num);
                    l_buf_push_cstr(&out, ":");
                    l_buf_push(&out, line.data, line.len - 1);
                    l_buf_push_cstr(&out, "\n");
                    write(STDOUT, out.data, out.len);
                    l_buf_clear(&out);
                }
                l_buf_clear(&line);
            } else {
                l_buf_push(&line, &buf[i], 1);
            }
        }
    }

    // Handle last line without trailing newline
    if (line.len > 0) {
        char nul = '\0';
        l_buf_push(&line, &nul, 1);
        line_num++;
        if (strstr((char *)line.data, pattern) != NULL) {
            matches++;
            l_buf_push_int(&out, line_num);
            l_buf_push_cstr(&out, ":");
            l_buf_push(&out, line.data, line.len - 1);
            l_buf_push_cstr(&out, "\n");
            write(STDOUT, out.data, out.len);
            l_buf_clear(&out);
        }
    }

    l_buf_free(&out);
    l_buf_free(&line);
    close(fd);
    return matches > 0 ? 0 : 1;
}
