#define L_MAINFILE
#include "l_os.h"

// printenv — prints environment variables
// Usage: printenv [VAR1 VAR2 ...]  (no args = print all)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        // Print all environment variables
        void *handle = l_env_start();
        void *iter = handle;
        char buf[4096];
        const char *entry;
        while ((entry = l_env_next(&iter, buf, sizeof(buf))) != NULL) {
            puts(entry);
            puts("\n");
        }
        l_env_end(handle);
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
