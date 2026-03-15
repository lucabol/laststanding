#define L_MAINFILE
#include "l_os.h"

// printenv — prints environment variables
// Usage: printenv [VAR1 VAR2 ...]  (no args = print all)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        // Print all environment variables
#ifdef _WIN32
        wchar_t *env = GetEnvironmentStringsW();
        if (env) {
            char buf[4096];
            wchar_t *p = env;
            while (*p) {
                int len = WideCharToMultiByte(CP_UTF8, 0, p, -1, buf, sizeof(buf), NULL, NULL);
                if (len > 0) { puts(buf); puts("\n"); }
                while (*p) p++;
                p++;
            }
            FreeEnvironmentStringsW(env);
        }
#else
        for (char **ep = l_envp; *ep; ep++) {
            puts(*ep);
            puts("\n");
        }
#endif
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        char *val = l_getenv(argv[i]);
        puts(argv[i]);
        puts("=");
        if (val)
            puts(val);
        else
            puts("(not set)");
        puts("\n");
    }

    return 0;
}
