#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"

// Minimal ls: lists directory entries with type, size, and name
// Usage: ls [directory]

#define MAX_ENTRIES 256

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
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - 1 - i; j++)
            if (l_strcmp(entries[j].name, entries[j + 1].name) > 0)
                swap_entries(&entries[j], &entries[j + 1]);
}

int main(int argc, char *argv[]) {
    const char *path = (argc > 1) ? argv[1] : ".";

    L_Dir dir;
    if (l_opendir(path, &dir) != 0) {
        l_puts("ls: cannot open directory\n");
        return 1;
    }

    Entry entries[MAX_ENTRIES];
    int count = 0;
    L_DirEntry *ent;

    while ((ent = l_readdir(&dir)) != (L_DirEntry *)0 && count < MAX_ENTRIES) {
        l_strncpy(entries[count].name, ent->d_name, 255);
        entries[count].name[255] = '\0';

        // Build full path for stat
        char fullpath[512];
        if (l_strcmp(path, ".") == 0) {
            l_strncpy(fullpath, ent->d_name, 511);
        } else {
            l_strncpy(fullpath, path, 400);
            fullpath[400] = '\0';
            size_t plen = l_strlen(fullpath);
            if (plen > 0 && fullpath[plen - 1] != '/' && fullpath[plen - 1] != '\\') {
                fullpath[plen] = '/';
                fullpath[plen + 1] = '\0';
            }
            l_strncat(fullpath, ent->d_name, 511 - l_strlen(fullpath));
        }
        fullpath[511] = '\0';

        L_Stat st;
        if (l_stat(fullpath, &st) == 0) {
            entries[count].size = st.st_size;
            if (L_S_ISDIR(st.st_mode))      entries[count].type = 'd';
            else if (L_S_ISREG(st.st_mode)) entries[count].type = '-';
            else                             entries[count].type = '?';
        } else {
            entries[count].size = 0;
            if (ent->d_type == L_DT_DIR)      entries[count].type = 'd';
            else if (ent->d_type == L_DT_REG) entries[count].type = '-';
            else                               entries[count].type = '?';
        }
        count++;
    }
    l_closedir(&dir);

    sort_entries(entries, count);

    for (int i = 0; i < count; i++) {
        char line[512];
        l_snprintf(line, sizeof(line), "%c %8lld %s\n",
                   entries[i].type, entries[i].size, entries[i].name);
        l_puts(line);
    }

    return 0;
}
