#define L_MAINFILE
#include "l_os.h"

// led — laststanding editor: a modal text editor with vim keybindings
// Usage: led [filename]

#define MAX_LINES    4096
#define MAX_LINE_LEN  256
#define CMD_BUF_LEN    64
#define YANK_LINES     64

enum Mode { NORMAL, INSERT, COMMAND };

typedef struct {
    char lines[MAX_LINES][MAX_LINE_LEN];
    int  num_lines;
    int  cx, cy;          // cursor col, row in file
    int  scroll_y;        // first visible line
    int  rows, cols;      // terminal size
    int  modified;
    enum Mode mode;
    char filename[256];
    char status[256];
    char cmd[CMD_BUF_LEN];
    int  cmd_len;
    char yank[YANK_LINES][MAX_LINE_LEN];
    int  yank_count;
    int  quit;
    int  count;           // numeric prefix for commands
} Editor;

static Editor E;

// --- Output helpers ---

static void out(const char *s, int len) { write(STDOUT, s, len); }
static void outs(const char *s)         { out(s, strlen(s)); }

static void out_num(int n) {
    char buf[12];
    itoa(n, buf, 10);
    outs(buf);
}

static void move_cursor(int r, int c) {
    outs("\033[");
    out_num(r);
    outs(";");
    out_num(c);
    outs("H");
}

static void clear_line(void) { outs("\033[2K"); }

static void set_status(const char *msg) {
    int i = 0;
    while (msg[i] && i < (int)sizeof(E.status) - 1) { E.status[i] = msg[i]; i++; }
    E.status[i] = '\0';
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
    E.lines[0][0] = '\0';

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            if (buf[i] == '\n' || buf[i] == '\r') {
                if (buf[i] == '\r' && i + 1 < n && buf[i + 1] == '\n') i++;
                E.lines[line][col] = '\0';
                line++;
                if (line >= MAX_LINES) goto done;
                col = 0;
                E.lines[line][0] = '\0';
            } else if (col < MAX_LINE_LEN - 1) {
                E.lines[line][col++] = buf[i];
                E.lines[line][col] = '\0';
            }
        }
    }
done:
    close(fd);
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
    for (int i = 0; i < E.num_lines; i++) {
        write(fd, E.lines[i], strlen(E.lines[i]));
        write(fd, "\n", 1);
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
    // Build full screen buffer to minimize flicker
    outs("\033[?25l");  // hide cursor
    move_cursor(1, 1);

    int text_rows = E.rows - 1;  // last row = status bar

    for (int y = 0; y < text_rows; y++) {
        int file_line = E.scroll_y + y;
        clear_line();
        if (file_line < E.num_lines) {
            char *line = E.lines[file_line];
            int len = strlen(line);
            // Only show what fits in terminal width
            int show = len < E.cols ? len : E.cols;
            if (show > 0) out(line, show);
        } else {
            outs("~");
        }
        if (y < text_rows - 1) outs("\r\n");
    }

    // Status bar (inverted colors)
    outs("\r\n");
    clear_line();
    outs("\033[7m");  // reverse video

    const char *mode_str = "NORMAL";
    if (E.mode == INSERT) mode_str = "INSERT";
    else if (E.mode == COMMAND) mode_str = "COMMAND";

    outs(" ");
    outs(mode_str);
    outs(" | ");
    if (E.filename[0]) outs(E.filename);
    else outs("[No Name]");
    if (E.modified) outs(" [+]");
    outs(" | L");
    out_num(E.cy + 1);
    outs(",C");
    out_num(E.cx + 1);
    outs(" ");
    if (E.status[0]) { outs("| "); outs(E.status); outs(" "); }

    // Pad the rest of status bar
    // (simple: just output enough spaces)
    outs("                                        ");
    outs("\033[0m");  // reset colors

    // Position cursor
    int screen_y = E.cy - E.scroll_y + 1;
    int screen_x = E.cx + 1;
    move_cursor(screen_y, screen_x);
    outs("\033[?25h");  // show cursor
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
    if (len >= MAX_LINE_LEN - 1) return;
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
    if (E.num_lines >= MAX_LINES) return;
    for (int i = E.num_lines; i > at; i--)
        memcpy(E.lines[i], E.lines[i - 1], MAX_LINE_LEN);
    E.lines[at][0] = '\0';
    E.num_lines++;
    E.modified = 1;
}

static void delete_line(int at) {
    if (E.num_lines <= 1) { E.lines[0][0] = '\0'; E.modified = 1; return; }
    for (int i = at; i < E.num_lines - 1; i++)
        memcpy(E.lines[i], E.lines[i + 1], MAX_LINE_LEN);
    E.num_lines--;
    E.modified = 1;
}

static void split_line(int row, int col) {
    if (E.num_lines >= MAX_LINES) return;
    insert_line(row + 1);
    strcpy(E.lines[row + 1], E.lines[row] + col);
    E.lines[row][col] = '\0';
    E.modified = 1;
}

static void join_lines(int row) {
    if (row >= E.num_lines - 1) return;
    int len1 = strlen(E.lines[row]);
    int len2 = strlen(E.lines[row + 1]);
    if (len1 + len2 < MAX_LINE_LEN) {
        strcpy(E.lines[row] + len1, E.lines[row + 1]);
        delete_line(row + 1);
    }
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
                if (E.yank_count < YANK_LINES)
                    strcpy(E.yank[E.yank_count++], E.lines[E.cy]);
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
                if (E.yank_count < YANK_LINES)
                    strcpy(E.yank[E.yank_count++], E.lines[E.cy + i]);
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
    l_term_size(&E.rows, &E.cols);
    if (E.rows < 3) E.rows = 24;
    if (E.cols < 10) E.cols = 80;

    E.num_lines = 1;
    E.lines[0][0] = '\0';

    if (argc < 2) {
        puts("Usage: led <filename>\n");
        return 0;
    }

    if (argc >= 2) {
        editor_load(argv[1]);
    }

    unsigned long old_mode = l_term_raw();
    outs("\033[2J");  // clear screen

    while (!E.quit) {
        render();
        char c;
        ssize_t n = l_read_nonblock(L_STDIN, &c, 1);
        if (n > 0) {
            handle_input(c);
        } else {
            l_sleep_ms(20);
        }
    }

    // Cleanup
    outs("\033[2J");   // clear screen
    move_cursor(1, 1);
    outs("\033[?25h"); // show cursor
    l_term_restore(old_mode);

    return 0;
}
