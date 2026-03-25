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
    if (pos >= 3 &&
        (unsigned char)buf[0] == 0xEF &&
        (unsigned char)buf[1] == 0xBB &&
        (unsigned char)buf[2] == 0xBF) {
        l_memmove(buf, buf + 3, (size_t)(pos - 2));
        pos -= 3;
    }
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

/* Find cmd in PATH; writes full path to out. Returns 1 if found. */
static int resolve_cmd(const char *cmd, char *out, int sz) {
    return l_find_executable(cmd, out, (size_t)sz);
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

static void close_redir_fd(L_FD fd) {
    if (fd >= 0 && fd != L_STDIN && fd != L_STDOUT && fd != L_STDERR)
        l_close(fd);
}

static int redirect_std_fd(L_FD target_fd, L_FD source_fd, L_FD *saved_fd) {
    *saved_fd = -1;
    if (source_fd == L_SPAWN_INHERIT || source_fd == target_fd)
        return 0;

    *saved_fd = l_dup(target_fd);
    if (*saved_fd < 0)
        return -1;

    if (l_dup2(source_fd, target_fd) != target_fd) {
        l_close(*saved_fd);
        *saved_fd = -1;
        return -1;
    }
    return 0;
}

static int restore_std_fd(L_FD target_fd, L_FD saved_fd) {
    int ok = 1;
    if (saved_fd < 0)
        return 0;
    if (l_dup2(saved_fd, target_fd) != target_fd)
        ok = 0;
    if (l_close(saved_fd) < 0)
        ok = 0;
    return ok ? 0 : -1;
}

static L_PID spawn_cmd(char *argv[], L_FD stdin_fd, L_FD stdout_fd) {
    char path[MAX_PATH_BUF];
    L_PID pid = -1;
    L_FD saved_in = -1;
    L_FD saved_out = -1;
    int restore_failed = 0;

    if (!resolve_cmd(argv[0], path, MAX_PATH_BUF)) {
        eputs("sh: command not found: ");
        eputs(argv[0]); eputs("\n");
        return -1;
    }

    if (redirect_std_fd(L_STDIN, stdin_fd, &saved_in) < 0)
        goto done;
    if (redirect_std_fd(L_STDOUT, stdout_fd, &saved_out) < 0)
        goto done;

    pid = l_spawn(path, argv, (char *const *)0);

done:
    if (restore_std_fd(L_STDOUT, saved_out) < 0)
        restore_failed = 1;
    if (restore_std_fd(L_STDIN, saved_in) < 0)
        restore_failed = 1;
    if (restore_failed)
        eputs("sh: failed to restore stdio\n");
    if (pid < 0) {
        eputs("sh: failed to execute '");
        eputs(path); eputs("'\n");
    }
    return pid;
}

static int exec_cmd(char *argv[], const Redir *r) {
    L_FD in_fd = L_SPAWN_INHERIT;
    L_FD out_fd = L_SPAWN_INHERIT;

    if (r->redir_in) {
        in_fd = l_open_read(r->redir_in);
        if (in_fd < 0) {
            eputs("sh: cannot open '");
            eputs(r->redir_in); eputs("'\n");
            return 1;
        }
    }
    if (r->redir_out) {
        out_fd = r->append ? l_open_append(r->redir_out) : l_open_trunc(r->redir_out);
        if (out_fd < 0) {
            close_redir_fd(in_fd);
            eputs("sh: cannot open '");
            eputs(r->redir_out); eputs("'\n");
            return 1;
        }
    }

    L_PID pid = spawn_cmd(argv, in_fd, out_fd);
    close_redir_fd(in_fd);
    close_redir_fd(out_fd);
    if (pid < 0) return 127;

    int ec = 0;
    if (l_wait(pid, &ec) < 0) {
        eputs("sh: wait failed\n");
        return 1;
    }
    return ec;
}

static int exec_pipe(char *argv[], int at) {
    char **left = argv, **right = argv + at + 1;
    if (!left[0] || !right[0]) {
        eputs("sh: invalid pipe\n"); return 1;
    }

    L_FD pfd[2];
    if (l_pipe(pfd) < 0) { eputs("sh: pipe failed\n"); return 1; }

    L_PID p1 = spawn_cmd(left, L_SPAWN_INHERIT, pfd[1]);
    l_close(pfd[1]);
    if (p1 < 0) {
        l_close(pfd[0]);
        return 127;
    }

    L_PID p2 = spawn_cmd(right, pfd[0], L_SPAWN_INHERIT);
    l_close(pfd[0]);
    if (p2 < 0) {
        int e1 = 0;
        l_wait(p1, &e1);
        return 127;
    }

    int e1 = 0, e2 = 0;
    if (l_wait(p1, &e1) < 0 || l_wait(p2, &e2) < 0) {
        eputs("sh: wait failed\n");
        return 1;
    }
    return e2;
}

/* --- Usage & main ------------------------------------------------------- */

static void usage(void) {
    l_puts("Usage: sh [--help]\n");
    l_puts("A freestanding shell built entirely on l_os.h.\n\n");
    l_puts("Built-in commands:\n");
    l_puts("  cd [dir]     change directory\n");
    l_puts("  pwd          print working directory\n");
    l_puts("  exit [code]  exit the shell\n");
    l_puts("  echo [...]   print arguments\n\n");
    l_puts("Features: PATH search, quoted arguments, > >> < redirection, cmd|cmd piping\n");
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

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
