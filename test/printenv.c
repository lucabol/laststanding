#define L_MAINFILE
#include "l_os.h"

// printenv — prints environment variables
// Usage: printenv [VAR1 VAR2 ...]  (no args = print all)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        // Print all environment variables
#ifdef _WIN32
        puts("(printenv --all not supported on Windows, specify variable names)\n");
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
