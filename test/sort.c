#define L_MAINFILE
#include "l_os.h"

// sort: line-based text sort
// Usage: sort [-r] [-f] [-n] [-u] [-h/--help] [file]

#define SORT_MAX_INPUT (64 * 1024)
#define MAX_LINES 4096

static char buf[SORT_MAX_INPUT];
static char *lines[MAX_LINES];
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

/* Shell sort with Knuth's gap sequence — avoids quicksort worst case */
static void shell_sort(int n) {
    int gap = 1;
    while (gap * 3 + 1 < n) gap = gap * 3 + 1;

    while (gap > 0) {
        for (int i = gap; i < n; i++) {
            char *tmp = lines[i];
            int j = i;
            while (j >= gap && compare(lines[j - gap], tmp) > 0) {
                lines[j] = lines[j - gap];
                j -= gap;
            }
            lines[j] = tmp;
        }
        gap = gap / 3; /* constant divisor — compiler uses multiply-shift, ARM-safe */
    }
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

    /* Read all input */
    int total = 0;
    while (total < SORT_MAX_INPUT - 1) {
        ssize_t r = l_read(fd, buf + total, (size_t)(SORT_MAX_INPUT - 1 - total));
        if (r <= 0) break;
        total += (int)r;
    }
    buf[total] = '\0';
    if (file) l_close(fd);

    if (total == 0) return 0;

    /* Split into lines — \n is a terminator, not a separator */
    int nlines = 0;
    char *p = buf;
    while (*p && nlines < MAX_LINES) {
        lines[nlines++] = p;
        char *nl = l_strchr(p, '\n');
        if (nl) {
            *nl = '\0';
            p = nl + 1;
        } else {
            break;
        }
    }

    shell_sort(nlines);

    /* Output with optional unique filtering */
    for (int i = 0; i < nlines; i++) {
        if (opt_unique && i > 0 && compare(lines[i], lines[i - 1]) == 0)
            continue;
        l_puts(lines[i]);
        l_puts("\n");
    }

    return 0;
}
