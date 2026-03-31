#define L_MAINFILE
#include "l_os.h"

// led — laststanding editor: a modal text editor with vim keybindings
// Usage: led [filename]

#define CMD_BUF_LEN    64
#define YANK_LINES     64

enum Mode { NORMAL, INSERT, COMMAND };

typedef struct {
    char **lines;         // arena-allocated array of line pointers
    int  *line_caps;      // arena-allocated array of per-line capacities
    int  num_lines;
    int  lines_cap;       // capacity of lines/line_caps arrays
    int  cx, cy;          // cursor col, row in file
    int  scroll_y;        // first visible line
    int  rows, cols;      // terminal size
    int  modified;
    enum Mode mode;
    char filename[256];
    char status[256];
    char cmd[CMD_BUF_LEN];
    int  cmd_len;
    char *yank[YANK_LINES];
    int  yank_count;
    int  quit;
    int  count;           // numeric prefix for commands
    int  crlf;            // 1 = Windows CRLF, 0 = Unix LF
    L_Arena arena;
} Editor;

static Editor E;

// --- Screen buffer (L_Buf eliminates flicker by writing in one syscall) ---

static L_Buf screen_buf;

static void sb_clear(void)                { l_buf_clear(&screen_buf); }
static void sb_flush(void)                { write(STDOUT, screen_buf.data, screen_buf.len); l_buf_clear(&screen_buf); }

static void sb_append(const char *s, int len) {
    l_buf_push(&screen_buf, s, (size_t)len);
}

static void sb_str(const char *s) { sb_append(s, strlen(s)); }

static void sb_num(int n) {
    char buf[12];
    itoa(n, buf, 10);
    sb_str(buf);
}

static void sb_move(int r, int c) {
    sb_str("\033[");
    sb_num(r);
    sb_str(";");
    sb_num(c);
    sb_str("H");
}

// --- Direct output (for non-render operations) ---

static void outs(const char *s) { write(STDOUT, s, strlen(s)); }

static void set_status(const char *msg) {
    int i = 0;
    while (msg[i] && i < (int)sizeof(E.status) - 1) { E.status[i] = msg[i]; i++; }
    E.status[i] = '\0';
}

// --- Arena line helpers ---

static void ensure_lines_cap(int needed) {
    if (needed <= E.lines_cap) return;
    int new_cap = E.lines_cap;
    while (new_cap < needed) new_cap *= 2;
    char **nl = l_arena_alloc(&E.arena, (size_t)new_cap * sizeof(char *));
    int *nc = l_arena_alloc(&E.arena, (size_t)new_cap * sizeof(int));
    if (!nl || !nc) return;
    if (E.lines_cap > 0) {
        l_memcpy(nl, E.lines, (size_t)E.lines_cap * sizeof(char *));
        l_memcpy(nc, E.line_caps, (size_t)E.lines_cap * sizeof(int));
    }
    E.lines = nl;
    E.line_caps = nc;
    E.lines_cap = new_cap;
}

static void ensure_line_cap(int row, int needed) {
    if (needed <= E.line_caps[row]) return;
    int new_cap = E.line_caps[row] ? E.line_caps[row] : 256;
    while (new_cap < needed) new_cap *= 2;
    char *np = l_arena_alloc(&E.arena, (size_t)new_cap);
    if (!np) return;
    l_memcpy(np, E.lines[row], (size_t)E.line_caps[row]);
    E.lines[row] = np;
    E.line_caps[row] = new_cap;
}

static void alloc_line_at(int idx) {
    E.lines[idx] = l_arena_alloc(&E.arena, 256);
    E.line_caps[idx] = E.lines[idx] ? 256 : 0;
    if (E.lines[idx]) E.lines[idx][0] = '\0';
}

// --- File I/O ---

static void editor_load(const char *path) {
    strcpy(E.filename, path);
    L_FD fd = open_read(path);
    if (fd < 0) {
        E.num_lines = 1;
        E.lines[0][0] = '\0';
        set_status("[New file]");
        return;
    }

    char buf[4096];
    ssize_t n;
    int line = 0, col = 0;
    int saw_cr = 0;
    E.crlf = -1;  // unknown until first newline
    E.lines[0][0] = '\0';

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (saw_cr) {
                saw_cr = 0;
                if (buf[i] == '\n') {
                    if (E.crlf < 0) E.crlf = 1;
                    continue;  // skip the \n after \r
                }
            }
            if (buf[i] == '\r') {
                // End this line; check next char for \n
                E.lines[line][col] = '\0';
                line++;
                ensure_lines_cap(line + 1);
                alloc_line_at(line);
                col = 0;
                saw_cr = 1;
                continue;
            }
            if (buf[i] == '\n') {
                if (E.crlf < 0) E.crlf = 0;
                E.lines[line][col] = '\0';
                line++;
                ensure_lines_cap(line + 1);
                alloc_line_at(line);
                col = 0;
            } else {
                ensure_line_cap(line, col + 2);
                E.lines[line][col++] = buf[i];
                E.lines[line][col] = '\0';
            }
        }
    }
    close(fd);
    if (E.crlf < 0) E.crlf = 0;  // default to LF for empty/no-newline files
    E.num_lines = line + 1;
    if (E.num_lines == 0) E.num_lines = 1;
    E.modified = 0;
    set_status("");
}

static int editor_save(void) {
    if (E.filename[0] == '\0') {
        set_status("No filename");
        return -1;
    }
    L_FD fd = open_trunc(E.filename);
    if (fd < 0) {
        set_status("Error: cannot save");
        return -1;
    }
    const char *eol = E.crlf ? "\r\n" : "\n";
    int eol_len = E.crlf ? 2 : 1;
    for (int i = 0; i < E.num_lines; i++) {
        write(fd, E.lines[i], strlen(E.lines[i]));
        write(fd, eol, eol_len);
    }
    close(fd);
    E.modified = 0;

    char msg[128] = "Wrote ";
    char numbuf[12];
    itoa(E.num_lines, numbuf, 10);
    int p = 6;
    for (int i = 0; numbuf[i]; i++) msg[p++] = numbuf[i];
    msg[p++] = ' '; msg[p++] = 'l'; msg[p++] = 'i'; msg[p++] = 'n';
    msg[p++] = 'e'; msg[p++] = 's'; msg[p] = '\0';
    set_status(msg);
    return 0;
}

// --- Rendering ---

static void render(void) {
    sb_clear();
    sb_str("\033[?25l");  // hide cursor
    sb_move(1, 1);

    int text_rows = E.rows - 1;  // last row = status bar

    for (int y = 0; y < text_rows; y++) {
        int file_line = E.scroll_y + y;
        sb_str("\033[2K");  // clear line
        if (file_line < E.num_lines) {
            char *line = E.lines[file_line];
            int len = strlen(line);
            int show = len < E.cols ? len : E.cols;
            if (show > 0) sb_append(line, show);
        } else {
            sb_str("~");
        }
        if (y < text_rows - 1) sb_str("\r\n");
    }

    // Status bar (inverted colors)
    sb_str("\r\n");
    sb_str("\033[2K");  // clear line
    sb_str("\033[7m");  // reverse video

    const char *mode_str = "NORMAL";
    if (E.mode == INSERT) mode_str = "INSERT";
    else if (E.mode == COMMAND) mode_str = "COMMAND";

    sb_str(" ");
    sb_str(mode_str);
    sb_str(" | ");
    if (E.filename[0]) sb_str(E.filename);
    else sb_str("[No Name]");
    if (E.modified) sb_str(" [+]");
    sb_str(" | L");
    sb_num(E.cy + 1);
    sb_str(",C");
    sb_num(E.cx + 1);
    sb_str(" | ");
    sb_str(E.crlf ? "CRLF" : "LF");
    sb_str(" ");
    if (E.status[0]) { sb_str("| "); sb_str(E.status); sb_str(" "); }

    // Pad the rest of status bar
    sb_str("                                        ");
    sb_str("\033[0m");  // reset colors

    // Position cursor
    int screen_y = E.cy - E.scroll_y + 1;
    int screen_x = E.cx + 1;
    sb_move(screen_y, screen_x);
    // Cursor shape: bar in insert mode, block in normal/command
    sb_str(E.mode == INSERT ? "\033[5 q" : "\033[1 q");
    sb_str("\033[?25h");  // show cursor

    sb_flush();  // single write() for the entire frame
}

// --- Cursor & scroll ---

static int line_len(int y) {
    if (y < 0 || y >= E.num_lines) return 0;
    return strlen(E.lines[y]);
}

static void clamp_cursor(void) {
    if (E.cy < 0) E.cy = 0;
    if (E.cy >= E.num_lines) E.cy = E.num_lines - 1;
    int ll = line_len(E.cy);
    if (E.mode == NORMAL && ll > 0) ll--;
    if (E.cx > ll) E.cx = ll;
    if (E.cx < 0) E.cx = 0;
}

static void scroll_to_cursor(void) {
    int text_rows = E.rows - 1;
    if (E.cy < E.scroll_y) E.scroll_y = E.cy;
    if (E.cy >= E.scroll_y + text_rows) E.scroll_y = E.cy - text_rows + 1;
}

// --- Line editing ---

static void insert_char(int row, int col, char c) {
    char *line = E.lines[row];
    int len = strlen(line);
    ensure_line_cap(row, len + 2);
    line = E.lines[row]; // pointer may have changed
    memmove(line + col + 1, line + col, len - col + 1);
    line[col] = c;
    E.modified = 1;
}

static void delete_char(int row, int col) {
    char *line = E.lines[row];
    int len = strlen(line);
    if (col >= len) return;
    memmove(line + col, line + col + 1, len - col);
    E.modified = 1;
}

static void insert_line(int at) {
    ensure_lines_cap(E.num_lines + 1);
    for (int i = E.num_lines; i > at; i--) {
        E.lines[i] = E.lines[i - 1];
        E.line_caps[i] = E.line_caps[i - 1];
    }
    alloc_line_at(at);
    E.num_lines++;
    E.modified = 1;
}

static void delete_line(int at) {
    if (E.num_lines <= 1) { E.lines[0][0] = '\0'; E.modified = 1; return; }
    for (int i = at; i < E.num_lines - 1; i++) {
        E.lines[i] = E.lines[i + 1];
        E.line_caps[i] = E.line_caps[i + 1];
    }
    E.num_lines--;
    E.modified = 1;
}

static void split_line(int row, int col) {
    insert_line(row + 1);
    int tail_len = strlen(E.lines[row] + col) + 1;
    ensure_line_cap(row + 1, tail_len);
    strcpy(E.lines[row + 1], E.lines[row] + col);
    E.lines[row][col] = '\0';
    E.modified = 1;
}

static void join_lines(int row) {
    if (row >= E.num_lines - 1) return;
    int len1 = strlen(E.lines[row]);
    int len2 = strlen(E.lines[row + 1]);
    ensure_line_cap(row, len1 + len2 + 1);
    strcpy(E.lines[row] + len1, E.lines[row + 1]);
    delete_line(row + 1);
}

// --- Command mode ---

static void handle_command(void) {
    E.cmd[E.cmd_len] = '\0';

    if (strcmp(E.cmd, "q") == 0) {
        if (E.modified) { set_status("Unsaved changes (use :q! to force)"); return; }
        E.quit = 1;
    } else if (strcmp(E.cmd, "q!") == 0) {
        E.quit = 1;
    } else if (strcmp(E.cmd, "w") == 0) {
        editor_save();
    } else if (strcmp(E.cmd, "wq") == 0 || strcmp(E.cmd, "x") == 0) {
        if (editor_save() == 0) E.quit = 1;
    } else if (E.cmd[0] == 'e' && E.cmd[1] == ' ') {
        editor_load(E.cmd + 2);
        E.cx = 0; E.cy = 0; E.scroll_y = 0;
    } else {
        set_status("Unknown command");
    }
}

// --- Input handling ---

static void handle_insert(char c) {
    if (c == 27) {  // ESC
        E.mode = NORMAL;
        if (E.cx > 0) E.cx--;
        set_status("");
        return;
    }
    if (c == 127 || c == 8) {  // Backspace
        if (E.cx > 0) {
            delete_char(E.cy, E.cx - 1);
            E.cx--;
        } else if (E.cy > 0) {
            E.cx = line_len(E.cy - 1);
            join_lines(E.cy - 1);
            E.cy--;
        }
        return;
    }
    if (c == '\r' || c == '\n') {
        split_line(E.cy, E.cx);
        E.cy++;
        E.cx = 0;
        return;
    }
    if (c == '\t') {
        for (int i = 0; i < 4; i++) {
            insert_char(E.cy, E.cx, ' ');
            E.cx++;
        }
        return;
    }
    if (c >= 32) {
        insert_char(E.cy, E.cx, c);
        E.cx++;
    }
}

static void handle_command_input(char c) {
    if (c == 27) {  // ESC
        E.mode = NORMAL;
        E.cmd_len = 0;
        set_status("");
        return;
    }
    if (c == '\r' || c == '\n') {
        E.mode = NORMAL;
        handle_command();
        E.cmd_len = 0;
        return;
    }
    if (c == 127 || c == 8) {  // Backspace
        if (E.cmd_len > 0) {
            E.cmd_len--;
            E.cmd[E.cmd_len] = '\0';
            char status[CMD_BUF_LEN + 2] = ":";
            strcpy(status + 1, E.cmd);
            set_status(status);
        } else {
            E.mode = NORMAL;
            set_status("");
        }
        return;
    }
    if (E.cmd_len < CMD_BUF_LEN - 1 && c >= 32) {
        E.cmd[E.cmd_len++] = c;
        E.cmd[E.cmd_len] = '\0';
        char status[CMD_BUF_LEN + 2] = ":";
        strcpy(status + 1, E.cmd);
        set_status(status);
    }
}

static int prev_d = 0;  // for dd command

static void handle_normal(char c) {
    int n = E.count > 0 ? E.count : 1;
    E.count = 0;
    set_status("");

    // Numeric prefix accumulation
    if (c >= '1' && c <= '9' && prev_d == 0) {
        E.count = c - '0';
        return;
    }
    if (c >= '0' && c <= '9' && n > 1) {
        E.count = n * 10 + (c - '0');
        return;
    }

    switch (c) {
    // Movement
    case 'h': E.cx -= n; break;
    case 'l': E.cx += n; break;
    case 'j': E.cy += n; break;
    case 'k': E.cy -= n; break;
    case '0': E.cx = 0; break;
    case '$': E.cx = line_len(E.cy); break;
    case 'g': // gg
        if (prev_d == 0) { prev_d = 'g'; return; }
        if (prev_d == 'g') { E.cy = 0; E.cx = 0; prev_d = 0; }
        return;
    case 'G': E.cy = E.num_lines - 1; E.cx = 0; break;

    // Word movement
    case 'w':
        for (int i = 0; i < n; i++) {
            char *line = E.lines[E.cy];
            int len = line_len(E.cy);
            // Skip current word
            while (E.cx < len && line[E.cx] != ' ') E.cx++;
            // Skip spaces
            while (E.cx < len && line[E.cx] == ' ') E.cx++;
            if (E.cx >= len && E.cy < E.num_lines - 1) { E.cy++; E.cx = 0; }
        }
        break;
    case 'b':
        for (int i = 0; i < n; i++) {
            if (E.cx == 0 && E.cy > 0) { E.cy--; E.cx = line_len(E.cy); }
            char *line = E.lines[E.cy];
            if (E.cx > 0) E.cx--;
            while (E.cx > 0 && line[E.cx] == ' ') E.cx--;
            while (E.cx > 0 && line[E.cx - 1] != ' ') E.cx--;
        }
        break;

    // Editing
    case 'i':
        E.mode = INSERT;
        set_status("-- INSERT --");
        break;
    case 'a':
        E.mode = INSERT;
        if (line_len(E.cy) > 0) E.cx++;
        set_status("-- INSERT --");
        break;
    case 'A':
        E.mode = INSERT;
        E.cx = line_len(E.cy);
        set_status("-- INSERT --");
        break;
    case 'I':
        E.mode = INSERT;
        E.cx = 0;
        set_status("-- INSERT --");
        break;
    case 'o':
        insert_line(E.cy + 1);
        E.cy++;
        E.cx = 0;
        E.mode = INSERT;
        set_status("-- INSERT --");
        break;
    case 'O':
        insert_line(E.cy);
        E.cx = 0;
        E.mode = INSERT;
        set_status("-- INSERT --");
        break;
    case 'x':
        for (int i = 0; i < n; i++) {
            if (line_len(E.cy) > 0) delete_char(E.cy, E.cx);
        }
        break;
    case 'd':
        if (prev_d == 0) { prev_d = 'd'; E.count = n; return; }
        if (prev_d == 'd') {
            // Yank lines first
            E.yank_count = 0;
            for (int i = 0; i < n && E.cy < E.num_lines; i++) {
                if (E.yank_count < YANK_LINES) {
                    int sl = strlen(E.lines[E.cy]) + 1;
                    char *y = l_arena_alloc(&E.arena, (size_t)sl);
                    if (y) { l_memcpy(y, E.lines[E.cy], (size_t)sl); E.yank[E.yank_count++] = y; }
                }
                delete_line(E.cy);
            }
            if (E.cy >= E.num_lines && E.cy > 0) E.cy--;
            E.cx = 0;
            prev_d = 0;
        }
        return;
    case 'J':
        join_lines(E.cy);
        break;

    // Yank & paste
    case 'y':
        if (prev_d == 0) { prev_d = 'y'; E.count = n; return; }
        if (prev_d == 'y') {
            E.yank_count = 0;
            for (int i = 0; i < n && (E.cy + i) < E.num_lines; i++) {
                if (E.yank_count < YANK_LINES) {
                    int sl = strlen(E.lines[E.cy + i]) + 1;
                    char *y = l_arena_alloc(&E.arena, (size_t)sl);
                    if (y) { l_memcpy(y, E.lines[E.cy + i], (size_t)sl); E.yank[E.yank_count++] = y; }
                }
            }
            char msg[32] = "";
            itoa(E.yank_count, msg, 10);
            int ml = strlen(msg);
            msg[ml++]=' '; msg[ml++]='l'; msg[ml++]='i'; msg[ml++]='n';
            msg[ml++]='e'; msg[ml++]='s'; msg[ml++]=' '; msg[ml++]='y';
            msg[ml++]='a'; msg[ml++]='n'; msg[ml++]='k'; msg[ml++]='e';
            msg[ml++]='d'; msg[ml]='\0';
            set_status(msg);
            prev_d = 0;
        }
        return;
    case 'p':
        for (int r = 0; r < n; r++) {
            for (int i = 0; i < E.yank_count; i++) {
                insert_line(E.cy + 1);
                int yl = strlen(E.yank[i]) + 1;
                ensure_line_cap(E.cy + 1, yl);
                strcpy(E.lines[E.cy + 1], E.yank[i]);
                E.cy++;
            }
        }
        E.cx = 0;
        break;
    case 'P':
        for (int r = 0; r < n; r++) {
            for (int i = E.yank_count - 1; i >= 0; i--) {
                insert_line(E.cy);
                int yl = strlen(E.yank[i]) + 1;
                ensure_line_cap(E.cy, yl);
                strcpy(E.lines[E.cy], E.yank[i]);
            }
        }
        E.cx = 0;
        break;

    // Case toggle
    case '~': {
        char *line = E.lines[E.cy];
        for (int i = 0; i < n && E.cx < line_len(E.cy); i++) {
            char ch = line[E.cx];
            if (ch >= 'a' && ch <= 'z') line[E.cx] = ch - 32;
            else if (ch >= 'A' && ch <= 'Z') line[E.cx] = ch + 32;
            E.cx++;
            E.modified = 1;
        }
        break;
    }

    // Page movement
    case 6:  // Ctrl-F
        E.cy += E.rows - 2;
        E.scroll_y += E.rows - 2;
        break;
    case 2:  // Ctrl-B
        E.cy -= E.rows - 2;
        E.scroll_y -= E.rows - 2;
        if (E.scroll_y < 0) E.scroll_y = 0;
        break;

    // Command mode
    case ':':
        E.mode = COMMAND;
        E.cmd_len = 0;
        E.cmd[0] = '\0';
        set_status(":");
        break;

    // Search
    case '/': {
        E.mode = COMMAND;
        E.cmd_len = 0;
        E.cmd[0] = '\0';
        set_status("/");
        // Reuse command mode but with search prefix
        // We'll handle this specially in handle_command
        break;
    }
    }
    prev_d = 0;
}

static void handle_input(char c) {
    switch (E.mode) {
    case NORMAL:  handle_normal(c); break;
    case INSERT:  handle_insert(c); break;
    case COMMAND: handle_command_input(c); break;
    }
    clamp_cursor();
    scroll_to_cursor();
}

// --- Main ---

int main(int argc, char *argv[]) {
    memset(&E, 0, sizeof(E));

    E.arena = l_arena_init(4 * 1024 * 1024); // 4MB
    if (!E.arena.base) {
        l_puts("led: cannot allocate memory\n");
        return 1;
    }

    E.lines_cap = 1024;
    E.lines = l_arena_alloc(&E.arena, (size_t)E.lines_cap * sizeof(char *));
    E.line_caps = l_arena_alloc(&E.arena, (size_t)E.lines_cap * sizeof(int));
    if (!E.lines || !E.line_caps) {
        l_puts("led: cannot allocate memory\n");
        l_arena_free(&E.arena);
        return 1;
    }

    l_buf_init(&screen_buf);
    l_term_size(&E.rows, &E.cols);
    if (E.rows < 3) E.rows = 24;
    if (E.cols < 10) E.cols = 80;

    alloc_line_at(0);
    E.num_lines = 1;

    if (argc < 2) {
        puts("Usage: led <filename>\n");
        l_buf_free(&screen_buf);
        l_arena_free(&E.arena);
        return 0;
    }

    if (argc >= 2) {
        editor_load(argv[1]);
    }

    unsigned long old_mode = l_term_raw();
    outs("\033[2J");  // clear screen
    int dirty = 1;

    while (!E.quit) {
        if (dirty) { render(); dirty = 0; }
        char c;
        ssize_t n = l_read_nonblock(L_STDIN, &c, 1);
        if (n > 0) {
            handle_input(c);
            dirty = 1;
        } else {
            l_sleep_ms(20);
        }
    }

    // Cleanup
    outs("\033[2J\033[H");  // clear screen + home
    outs("\033[1 q");       // restore block cursor
    outs("\033[?25h");      // show cursor
    l_term_restore(old_mode);
    l_buf_free(&screen_buf);
    l_arena_free(&E.arena);

    return 0;
}
