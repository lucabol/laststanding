#define L_MAINFILE
#include "l_os.h"

// config: simple INI file parser using L_Map and L_Str
// Usage: config <file.ini>
// Parses [section] headers and key = value pairs.
// Stores keys as "section.key", prints all parsed values.

static L_Str trim_comment(L_Str line) {
    // Strip inline comments (';' or '#')
    ptrdiff_t semi = l_str_chr(line, ';');
    ptrdiff_t hash = l_str_chr(line, '#');
    ptrdiff_t pos = -1;
    if (semi >= 0 && hash >= 0) pos = semi < hash ? semi : hash;
    else if (semi >= 0) pos = semi;
    else if (hash >= 0) pos = hash;
    if (pos >= 0) line = l_str_sub(line, 0, (size_t)pos);
    return l_str_trim(line);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        l_puts("Usage: config <file.ini>\n");
        l_puts("Parse an INI file and print all key=value pairs.\n");
        return 0;
    }

    L_FD fd = l_open_read(argv[1]);
    if (fd < 0) {
        l_puts("Error: cannot open '");
        l_puts(argv[1]);
        l_puts("'\n");
        return 1;
    }

    // Read entire file
    L_Buf file_buf;
    l_buf_init(&file_buf);
    char buf[4096];
    ssize_t n;
    while ((n = l_read(fd, buf, sizeof(buf))) > 0)
        l_buf_push(&file_buf, buf, (size_t)n);
    l_close(fd);

    L_Arena arena = l_arena_init(256 * 1024);
    if (!arena.base) { l_puts("Out of memory\n"); l_buf_free(&file_buf); return 1; }

    L_Map map = l_map_init(&arena, 256);

    L_Str content = l_buf_as_str(&file_buf);
    L_Str *lines;
    int nlines = l_str_split(&arena, content, l_str("\n"), &lines);

    char section[128] = "";

    for (int i = 0; i < nlines; i++) {
        L_Str line = l_str_trim(lines[i]);
        if (line.len == 0) continue;

        // Strip \r from line endings
        if (line.len > 0 && line.data[line.len - 1] == '\r')
            line.len--;
        line = l_str_trim(line);
        if (line.len == 0) continue;

        // Comment line
        if (line.data[0] == ';' || line.data[0] == '#') continue;

        // Section header
        if (line.data[0] == '[') {
            ptrdiff_t close = l_str_chr(line, ']');
            if (close > 1) {
                L_Str sec = l_str_sub(line, 1, (size_t)(close - 1));
                sec = l_str_trim(sec);
                if (sec.len < sizeof(section)) {
                    l_memcpy(section, sec.data, sec.len);
                    section[sec.len] = '\0';
                }
            }
            continue;
        }

        // Key = value pair
        line = trim_comment(line);
        ptrdiff_t eq = l_str_chr(line, '=');
        if (eq < 0) continue;

        L_Str key = l_str_trim(l_str_sub(line, 0, (size_t)eq));
        L_Str val = l_str_trim(l_str_sub(line, (size_t)(eq + 1), line.len - (size_t)(eq + 1)));

        // Build "section.key" in arena
        L_Buf keybuf;
        l_buf_init(&keybuf);
        if (section[0]) {
            l_buf_push_cstr(&keybuf, section);
            l_buf_push_cstr(&keybuf, ".");
        }
        l_buf_push_str(&keybuf, key);

        char *key_cstr = l_str_cstr(&arena, l_buf_as_str(&keybuf));
        char *val_cstr = l_str_cstr(&arena, val);

        l_map_put(&map, key_cstr, l_strlen(key_cstr), val_cstr);
        l_buf_free(&keybuf);
    }

    // Print all parsed values by re-scanning (map doesn't support iteration)
    l_memset(section, 0, sizeof(section));
    for (int i = 0; i < nlines; i++) {
        L_Str line = l_str_trim(lines[i]);
        if (line.len == 0) continue;
        if (line.len > 0 && line.data[line.len - 1] == '\r') line.len--;
        line = l_str_trim(line);
        if (line.len == 0) continue;
        if (line.data[0] == ';' || line.data[0] == '#') continue;

        if (line.data[0] == '[') {
            ptrdiff_t close = l_str_chr(line, ']');
            if (close > 1) {
                L_Str sec = l_str_trim(l_str_sub(line, 1, (size_t)(close - 1)));
                if (sec.len < sizeof(section)) {
                    l_memcpy(section, sec.data, sec.len);
                    section[sec.len] = '\0';
                }
                l_puts("["); l_puts(section); l_puts("]\n");
            }
            continue;
        }

        line = trim_comment(line);
        ptrdiff_t eq = l_str_chr(line, '=');
        if (eq < 0) continue;

        L_Str key = l_str_trim(l_str_sub(line, 0, (size_t)eq));

        // Reconstruct full key
        L_Buf keybuf;
        l_buf_init(&keybuf);
        if (section[0]) {
            l_buf_push_cstr(&keybuf, section);
            l_buf_push_cstr(&keybuf, ".");
        }
        l_buf_push_str(&keybuf, key);

        char *key_cstr = l_str_cstr(&arena, l_buf_as_str(&keybuf));
        char *val = l_map_get(&map, key_cstr, l_strlen(key_cstr));
        if (val) {
            l_puts("  ");
            l_buf_push_cstr(&keybuf, ""); // reuse
            l_puts(key_cstr);
            l_puts(" = ");
            l_puts(val);
            l_puts("\n");
        }
        l_buf_free(&keybuf);
    }

    l_buf_free(&file_buf);
    l_arena_free(&arena);
    return 0;
}
