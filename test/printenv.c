#define L_MAINFILE
#include "l_os.h"

// printenv — prints the value of environment variables
// Usage: printenv VAR1 [VAR2 ...]

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        puts("Usage: printenv VAR1 [VAR2 ...]\n");
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
