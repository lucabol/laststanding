#define L_MAINFILE
#include "l_os.h"

// sort: line-based text sort
// Usage: sort [-r] [-f] [-n] [-u] [-h/--help] [file]

static int opt_reverse, opt_fold, opt_numeric, opt_unique;

static int compare(const char *a, const char *b) {
    int r;
    if (opt_numeric) {
        int na = l_atoi(a);
        int nb = l_atoi(b);
        r = (na > nb) - (na < nb);
    } else if (opt_fold) {
        r = l_strcasecmp(a, b);
    } else {
        r = l_strcmp(a, b);
    }
    if (opt_reverse) r = -r;
    return r;
}

static int compare_ptr(const void *a, const void *b) {
    return compare(*(const char *const *)a, *(const char *const *)b);
}

static void usage(void) {
    l_puts("Usage: sort [-r] [-f] [-n] [-u] [-h/--help] [file]\n");
    l_puts("Sort lines of text.\n\n");
    l_puts("  -r        reverse sort order\n");
    l_puts("  -f        case-insensitive sort\n");
    l_puts("  -n        numeric sort\n");
    l_puts("  -u        suppress duplicate lines\n");
    l_puts("  --help    display this help and exit\n");
}

int main(int argc, char *argv[]) {
    const char *file = (const char *)0;
    opt_reverse = opt_fold = opt_numeric = opt_unique = 0;

    int c;
    l_opterr = 1;
    while ((c = l_getopt(argc, argv, "rfnuh")) != -1) {
        switch (c) {
            case 'r': opt_reverse = 1; break;
            case 'f': opt_fold = 1; break;
            case 'n': opt_numeric = 1; break;
            case 'u': opt_unique = 1; break;
            case 'h': usage(); return 0;
            default: return 1;
        }
    }
    if (l_optind < argc) {
        if (l_strcmp(argv[l_optind], "--help") == 0) { usage(); return 0; }
        file = argv[l_optind];
    }

    L_FD fd = L_STDIN;
    if (file) {
        fd = l_open_read(file);
        if (fd < 0) {
            l_puts("sort: cannot open '");
            l_puts(file);
            l_puts("'\n");
            return 1;
        }
    }

    /* Read all input into growable buffer */
    L_Buf input;
    l_buf_init(&input);
    {
        char tmp[4096];
        ssize_t r;
        while ((r = l_read(fd, tmp, sizeof(tmp))) > 0)
            l_buf_push(&input, tmp, (size_t)r);
    }
    if (file) l_close(fd);

    if (input.len == 0) { l_buf_free(&input); return 0; }

    /* Null-terminate so we can use string functions on the data */
    { char nul = '\0'; l_buf_push(&input, &nul, 1); }

    /* Split into lines — \n is a terminator, not a separator */
    L_Arena arena = l_arena_init(1024 * 1024);
    if (!arena.base) { l_puts("sort: out of memory\n"); l_buf_free(&input); return 1; }

    size_t lines_cap = 4096;
    char **lines = l_arena_alloc(&arena, lines_cap * sizeof(char *));
    int nlines = 0;
    char *p = (char *)input.data;
    while (*p) {
        if ((size_t)nlines >= lines_cap) {
            size_t new_cap = lines_cap * 2;
            char **nl = l_arena_alloc(&arena, new_cap * sizeof(char *));
            l_memcpy(nl, lines, (size_t)nlines * sizeof(char *));
            lines = nl;
            lines_cap = new_cap;
        }
        lines[nlines++] = p;
        char *nl = l_strchr(p, '\n');
        if (nl) {
            *nl = '\0';
            p = nl + 1;
        } else {
            break;
        }
    }

    l_qsort(lines, (size_t)nlines, sizeof(char *), compare_ptr);

    /* Output with optional unique filtering */
    for (int i = 0; i < nlines; i++) {
        if (opt_unique && i > 0 && compare(lines[i], lines[i - 1]) == 0)
            continue;
        l_puts(lines[i]);
        l_puts("\n");
    }

    l_arena_free(&arena);
    l_buf_free(&input);
    return 0;
}
