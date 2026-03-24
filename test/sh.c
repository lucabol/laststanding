#define L_MAINFILE
#include "l_os.h"

/* sh: a freestanding shell — the ultimate laststanding showcase.
 * Usage: sh [--help]
 * Builtins: cd, pwd, exit, echo
 * External commands with PATH search, quoted arguments.
 * Unix: I/O redirection (> >> <) and single pipe (cmd1 | cmd2). */

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_PATH_BUF 512

static int last_exit;
#ifdef __unix__
static char **sh_envp;
#endif

/* --- Helpers ------------------------------------------------------------ */

static void eputs(const char *s) {
    l_write(L_STDERR, s, l_strlen(s));
}

/* Return last component of path (like \W in bash) */
static const char *path_tail(const char *path) {
    const char *t = path;
    for (const char *p = path; *p; p++)
        if ((*p == '/' || *p == '\\') && *(p + 1))
            t = p + 1;
    return t;
}

static void print_prompt(void) {
    char cwd[MAX_PATH_BUF];
    l_puts(l_getcwd(cwd, sizeof(cwd)) ? path_tail(cwd) : "?");
    l_puts("$ ");
}

/* Read line from stdin; handles backspace. Returns -1 on EOF. */
static int read_line(char *buf, int max) {
    int pos = 0;
    while (pos < max - 1) {
        char c;
        if (l_read(L_STDIN, &c, 1) <= 0) return -1;
        if (c == '\n') break;
        if (c == '\r') continue;
        if ((c == 0x7f || c == 0x08) && pos > 0) { pos--; continue; }
        if (c != 0x7f && c != 0x08) buf[pos++] = c;
    }
    buf[pos] = '\0';
    return pos;
}

/* Split line into argv; handles single and double quotes. */
static int parse_line(char *line, char *argv[]) {
    int ac = 0;
    char *p = line;
    while (*p && ac < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        char q = 0;
        if (*p == '"' || *p == '\'') q = *p++;
        argv[ac++] = p;
        while (*p && (q ? *p != q : (*p != ' ' && *p != '\t'))) p++;
        if (*p) *p++ = '\0';
    }
    argv[ac] = (char *)0;
    return ac;
}

/* --- Command resolution ------------------------------------------------- */

static int has_sep(const char *s) {
    for (; *s; s++) if (*s == '/' || *s == '\\') return 1;
    return 0;
}

static int file_exists(const char *p) {
    L_FD f = l_open_read(p);
    if (f < 0) return 0;
    l_close(f);
    return 1;
}

#ifdef _WIN32
static int has_win_ext(const char *s) {
    const char *d = (const char *)0;
    for (; *s; s++) if (*s == '.') d = s;
    return d && (l_strcasecmp(d, ".exe") == 0 ||
                 l_strcasecmp(d, ".bat") == 0 ||
                 l_strcasecmp(d, ".cmd") == 0);
}
#endif

/* Find cmd in PATH; writes full path to out. Returns 1 if found. */
static int resolve_cmd(const char *cmd, char *out, int sz) {
    /* Command with explicit path — use directly */
    if (has_sep(cmd)) {
        l_strncpy(out, cmd, (size_t)(sz - 1));
        out[sz - 1] = '\0';
#ifdef _WIN32
        if (!file_exists(out) && !has_win_ext(cmd) &&
            (int)l_strlen(out) + 5 < sz)
            l_strncat(out, ".exe", 4);
#endif
        return 1;
    }

    /* Search PATH directories */
    char *path = l_getenv("PATH");
    if (!path) return 0;

#ifdef _WIN32
    const char sep = ';', ds = '\\';
#else
    const char sep = ':', ds = '/';
#endif

    const char *p = path;
    while (*p) {
        const char *e = l_strchr(p, sep);
        int dlen = e ? (int)(e - p) : (int)l_strlen(p);
        if (dlen > 0 && dlen + 2 < sz) {
            l_strncpy(out, p, (size_t)dlen);
            out[dlen] = ds;
            out[dlen + 1] = '\0';
            l_strncat(out, cmd, (size_t)(sz - dlen - 2));
#ifdef _WIN32
            if (!has_win_ext(cmd) && (int)l_strlen(out) + 5 < sz)
                l_strncat(out, ".exe", 4);
#endif
            if (file_exists(out)) return 1;
        }
        if (!e) break;
        p = e + 1;
    }
    return 0;
}

/* --- Redirection parsing ------------------------------------------------ */

typedef struct {
    const char *redir_in;   /* < file  */
    const char *redir_out;  /* > file  */
    int append;             /* >> vs > */
    int pipe_at;            /* index of | separator, or -1 */
} Redir;

static int parse_redir(char *argv[], int ac, Redir *r) {
    r->redir_in = r->redir_out = (const char *)0;
    r->append = 0;
    r->pipe_at = -1;
    int d = 0;
    for (int i = 0; i < ac; i++) {
        if (l_strcmp(argv[i], "|") == 0) {
            r->pipe_at = d;
            argv[d++] = (char *)0;
        } else if (l_strcmp(argv[i], ">>") == 0 && i + 1 < ac) {
            r->redir_out = argv[++i]; r->append = 1;
        } else if (l_strcmp(argv[i], ">") == 0 && i + 1 < ac) {
            r->redir_out = argv[++i];
        } else if (l_strcmp(argv[i], "<") == 0 && i + 1 < ac) {
            r->redir_in = argv[++i];
        } else {
            argv[d++] = argv[i];
        }
    }
    argv[d] = (char *)0;
    return d;
}

/* --- Built-in commands -------------------------------------------------- */

static int try_builtin(char *argv[], int ac) {
    if (l_strcmp(argv[0], "exit") == 0)
        l_exit(ac > 1 ? l_atoi(argv[1]) : last_exit);

    if (l_strcmp(argv[0], "cd") == 0) {
        const char *dir = ac > 1 ? argv[1] : l_getenv("HOME");
#ifdef _WIN32
        if (!dir) dir = l_getenv("USERPROFILE");
#endif
        if (!dir) return 1;
        if (l_chdir(dir) < 0) {
            eputs("sh: cd: "); eputs(dir);
            eputs(": no such directory\n");
            last_exit = 1;
        } else {
            last_exit = 0;
        }
        return 1;
    }

    if (l_strcmp(argv[0], "pwd") == 0) {
        char cwd[MAX_PATH_BUF];
        if (l_getcwd(cwd, sizeof(cwd))) {
            l_puts(cwd); l_puts("\n");
        }
        last_exit = 0;
        return 1;
    }

    if (l_strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < ac; i++) {
            if (i > 1) l_puts(" ");
            l_puts(argv[i]);
        }
        l_puts("\n");
        last_exit = 0;
        return 1;
    }

    return 0;
}

/* --- Execution ---------------------------------------------------------- */

#ifdef __unix__

static int exec_cmd(char *argv[], const Redir *r) {
    char path[MAX_PATH_BUF];
    if (!resolve_cmd(argv[0], path, MAX_PATH_BUF)) {
        eputs("sh: command not found: ");
        eputs(argv[0]); eputs("\n");
        return 127;
    }
    L_PID pid = l_fork();
    if (pid < 0) { eputs("sh: fork failed\n"); return 1; }
    if (pid == 0) {
        if (r->redir_in) {
            L_FD f = l_open_read(r->redir_in);
            if (f < 0) {
                eputs("sh: cannot open '");
                eputs(r->redir_in); eputs("'\n");
                l_exit(1);
            }
            l_dup2(f, L_STDIN); l_close(f);
        }
        if (r->redir_out) {
            L_FD f = r->append
                ? l_open_append(r->redir_out)
                : l_open_trunc(r->redir_out);
            if (f < 0) {
                eputs("sh: cannot open '");
                eputs(r->redir_out); eputs("'\n");
                l_exit(1);
            }
            l_dup2(f, L_STDOUT); l_close(f);
        }
        l_execve(path, argv, sh_envp);
        eputs("sh: failed to execute '");
        eputs(path); eputs("'\n");
        l_exit(127);
    }
    int ec = 0;
    l_wait(pid, &ec);
    return ec;
}

static int exec_pipe(char *argv[], int at) {
    char **left = argv, **right = argv + at + 1;
    if (!left[0] || !right[0]) {
        eputs("sh: invalid pipe\n"); return 1;
    }

    char lp[MAX_PATH_BUF], rp[MAX_PATH_BUF];
    if (!resolve_cmd(left[0], lp, MAX_PATH_BUF)) {
        eputs("sh: command not found: ");
        eputs(left[0]); eputs("\n"); return 127;
    }
    if (!resolve_cmd(right[0], rp, MAX_PATH_BUF)) {
        eputs("sh: command not found: ");
        eputs(right[0]); eputs("\n"); return 127;
    }

    L_FD pfd[2];
    if (l_pipe(pfd) < 0) { eputs("sh: pipe failed\n"); return 1; }

    L_PID p1 = l_fork();
    if (p1 == 0) {
        l_close(pfd[0]);
        l_dup2(pfd[1], L_STDOUT); l_close(pfd[1]);
        l_execve(lp, left, sh_envp);
        l_exit(127);
    }
    L_PID p2 = l_fork();
    if (p2 == 0) {
        l_close(pfd[1]);
        l_dup2(pfd[0], L_STDIN); l_close(pfd[0]);
        l_execve(rp, right, sh_envp);
        l_exit(127);
    }
    l_close(pfd[0]); l_close(pfd[1]);

    int e1 = 0, e2 = 0;
    l_wait(p1, &e1);
    l_wait(p2, &e2);
    return e2;
}

#else /* Windows */

static int exec_cmd(char *argv[], const Redir *r) {
    if (r->redir_in || r->redir_out) {
        eputs("sh: I/O redirection not yet supported on Windows\n");
        return 1;
    }
    char path[MAX_PATH_BUF];
    if (!resolve_cmd(argv[0], path, MAX_PATH_BUF)) {
        eputs("sh: command not found: ");
        eputs(argv[0]); eputs("\n");
        return 127;
    }
    char *save = argv[0];
    argv[0] = path;
    L_PID pid = l_spawn(path, argv, (char *const *)0);
    argv[0] = save;
    if (pid < 0) {
        eputs("sh: failed to execute '");
        eputs(path); eputs("'\n");
        return 127;
    }
    int ec = 0;
    l_wait(pid, &ec);
    return ec;
}

static int exec_pipe(char *argv[], int at) {
    (void)argv; (void)at;
    eputs("sh: piping not yet supported on Windows\n");
    return 1;
}

#endif

/* --- Usage & main ------------------------------------------------------- */

static void usage(void) {
    l_puts("Usage: sh [--help]\n");
    l_puts("A freestanding shell built entirely on l_os.h.\n\n");
    l_puts("Built-in commands:\n");
    l_puts("  cd [dir]     change directory\n");
    l_puts("  pwd          print working directory\n");
    l_puts("  exit [code]  exit the shell\n");
    l_puts("  echo [...]   print arguments\n\n");
    l_puts("Features: PATH search, quoted arguments");
#ifdef __unix__
    l_puts(", > >> < redirection, cmd|cmd piping");
#endif
    l_puts("\n");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
#ifdef __unix__
    sh_envp = argv + argc + 1;
#endif

    if (argc > 1 && l_strcmp(argv[1], "--help") == 0) {
        usage();
        return 0;
    }

    char line[MAX_LINE];
    char *args[MAX_ARGS];

    for (;;) {
        print_prompt();
        if (read_line(line, MAX_LINE) < 0) break;
        if (!line[0]) continue;

        int ac = parse_line(line, args);
        if (ac == 0) continue;

        Redir r;
        ac = parse_redir(args, ac, &r);
        if (ac == 0 || !args[0]) continue;

        if (r.pipe_at >= 0)
            last_exit = exec_pipe(args, r.pipe_at);
        else if (!try_builtin(args, ac))
            last_exit = exec_cmd(args, &r);
    }
    l_puts("\n");
    return last_exit;
}
