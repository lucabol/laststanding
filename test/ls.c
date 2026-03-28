#define L_MAINFILE
#include "l_os.h"

// ls: lists directory contents like the default Linux ls
// Usage: ls [-a] [-l] [directory]
//   -a  show hidden files (starting with '.')
//   -l  long format (type, size, name)

#define MAX_ENTRIES 512

typedef struct {
    char name[256];
    char type;       // 'd', '-', or '?'
    long long size;
} Entry;

static void swap_entries(Entry *a, Entry *b) {
    Entry tmp;
    l_memcpy(&tmp, a, sizeof(Entry));
    l_memcpy(a, b, sizeof(Entry));
    l_memcpy(b, &tmp, sizeof(Entry));
}

static void sort_entries(Entry *entries, int count) {
    // Case-insensitive sort, skip leading '.'
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - 1 - i; j++) {
            const char *a = entries[j].name;
            const char *b = entries[j+1].name;
            if (*a == '.') a++;
            if (*b == '.') b++;
            if (l_strcasecmp(a, b) > 0)
                swap_entries(&entries[j], &entries[j + 1]);
        }
}

int main(int argc, char *argv[]) {
    int show_all = 0, long_fmt = 0;
    const char *path = ".";

    int c;
    l_opterr = 1;
    while ((c = l_getopt(argc, argv, "alh")) != -1) {
        switch (c) {
            case 'a': show_all = 1; break;
            case 'l': long_fmt = 1; break;
            case 'h':
                l_puts("Usage: ls [-al] [directory]\n");
                l_puts("List directory contents.\n\n");
                l_puts("  -a        show hidden files (starting with '.')\n");
                l_puts("  -l        long format (type, size, name)\n");
                l_puts("  --help    display this help and exit\n");
                return 0;
            default: return 1;
        }
    }
    if (l_optind < argc) {
        if (l_strcmp(argv[l_optind], "--help") == 0) {
            l_puts("Usage: ls [-al] [directory]\n");
            l_puts("List directory contents.\n\n");
            l_puts("  -a        show hidden files (starting with '.')\n");
            l_puts("  -l        long format (type, size, name)\n");
            l_puts("  --help    display this help and exit\n");
            return 0;
        }
        path = argv[l_optind];
    }

    L_Dir dir;
    if (l_opendir(path, &dir) != 0) {
        l_puts("ls: cannot access '");
        l_puts(path);
        l_puts("': No such file or directory\n");
        return 2;
    }

    Entry entries[MAX_ENTRIES];
    int count = 0;
    L_DirEntry *ent;

    while ((ent = l_readdir(&dir)) != (L_DirEntry *)0 && count < MAX_ENTRIES) {
        // Skip hidden files unless -a
        if (!show_all && ent->d_name[0] == '.') continue;

        l_strncpy(entries[count].name, ent->d_name, 255);
        entries[count].name[255] = '\0';

        if (long_fmt) {
            // Build full path for stat
            char fullpath[512];
            size_t plen = l_strlen(path);
            l_strncpy(fullpath, path, 400);
            fullpath[400] = '\0';
            if (plen > 0 && path[plen-1] != '/' && path[plen-1] != '\\') {
                fullpath[plen] = '/';
                fullpath[plen + 1] = '\0';
            }
            l_strncat(fullpath, ent->d_name, 511 - l_strlen(fullpath));
            fullpath[511] = '\0';

            L_Stat st;
            if (l_stat(fullpath, &st) == 0) {
                entries[count].size = st.st_size;
                entries[count].type = L_S_ISDIR(st.st_mode) ? 'd' : L_S_ISREG(st.st_mode) ? '-' : '?';
            } else {
                entries[count].size = 0;
                entries[count].type = (ent->d_type == L_DT_DIR) ? 'd' : (ent->d_type == L_DT_REG) ? '-' : '?';
            }
        }
        count++;
    }
    l_closedir(&dir);

    sort_entries(entries, count);

    if (long_fmt) {
        for (int i = 0; i < count; i++) {
            char t[2] = {entries[i].type, 0};
            l_puts(t);
            // Right-align size in 10 chars — use 32-bit math (files < 4GB)
            unsigned int sz = (unsigned int)entries[i].size;
            char sbuf[20];
            int slen = 0;
            if (sz == 0) { sbuf[slen++] = '0'; }
            else { unsigned int v = sz; while (v > 0) { sbuf[slen++] = '0' + (v % 10); v /= 10; } }
            for (int p = 0; p < 10 - slen; p++) l_puts(" ");
            for (int p = slen - 1; p >= 0; p--) { char c[2] = {sbuf[p], 0}; l_puts(c); }
            l_puts(" ");
            l_puts(entries[i].name);
            l_puts("\n");
        }
    } else {
        // Default: names only, space-separated
        for (int i = 0; i < count; i++) {
            if (i > 0) l_puts("  ");
            l_puts(entries[i].name);
        }
        if (count > 0) l_puts("\n");
    }

    return 0;
}
