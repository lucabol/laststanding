#define L_MAINFILE
#include "l_os.h"

// Minimal grep: filters lines matching a substring or glob pattern
// Usage: grep [-g] <pattern> <filename>
//   -g  use glob/fnmatch matching instead of substring search

int main(int argc, char *argv[]) {
    int use_glob = 0;
    int c;
    l_opterr = 1;
    while ((c = l_getopt(argc, argv, "g")) != -1) {
        switch (c) {
            case 'g': use_glob = 1; break;
            default:
                puts("Usage: grep [-g] <pattern> <filename>\n");
                return 1;
        }
    }

    if (argc - l_optind != 2) {
        puts("Usage: grep [-g] <pattern> <filename>\n");
        puts("  -g  use glob matching instead of substring\n");
        return 0;
    }

    char *pattern = argv[l_optind];
    L_FD fd = open_read(argv[l_optind + 1]);
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

                int matched = use_glob
                    ? (l_fnmatch(pattern, (char *)line.data) == 0)
                    : (strstr((char *)line.data, pattern) != NULL);

                if (matched) {
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

        int matched = use_glob
            ? (l_fnmatch(pattern, (char *)line.data) == 0)
            : (strstr((char *)line.data, pattern) != NULL);

        if (matched) {
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
