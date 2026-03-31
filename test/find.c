#define L_MAINFILE
#include "l_os.h"

// find: recursively search a directory tree for filenames matching a glob pattern
// Usage: find <directory> <pattern>
// Example: find . "*.c"
// NOTE: pattern matching is case-sensitive, uses l_fnmatch

static void find_recursive(const char *dir, const char *pattern, char *pathbuf, int pathmax) {
    L_Dir d;
    if (l_opendir(dir, &d) != 0) return;

    L_DirEntry *ent;
    while ((ent = l_readdir(&d)) != (L_DirEntry *)0) {
        if (l_strcmp(ent->d_name, ".") == 0 || l_strcmp(ent->d_name, "..") == 0)
            continue;

        l_path_join(pathbuf, (size_t)pathmax, dir, ent->d_name);

        if (l_fnmatch(pattern, ent->d_name) == 0) {
            l_puts(pathbuf);
            l_puts("\n");
        }

        // Recurse into subdirectories
        if (ent->d_type == L_DT_DIR) {
            find_recursive(pathbuf, pattern, pathbuf + l_strlen(pathbuf) + 1,
                           pathmax - (int)l_strlen(pathbuf) - 1);
        } else {
            // d_type may be unknown — try stat
            L_Stat st;
            if (l_stat(pathbuf, &st) == 0 && L_S_ISDIR(st.st_mode)) {
                find_recursive(pathbuf, pattern, pathbuf + l_strlen(pathbuf) + 1,
                               pathmax - (int)l_strlen(pathbuf) - 1);
            }
        }
    }
    l_closedir(&d);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        l_puts("Usage: find <directory> <pattern>\n");
        l_puts("Recursively find files matching a glob pattern.\n");
        l_puts("Example: find . \"*.c\"\n");
        return 0;
    }

    char pathbuf[4096];
    find_recursive(argv[1], argv[2], pathbuf, (int)sizeof(pathbuf));
    return 0;
}
