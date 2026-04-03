#define L_MAINFILE
#include "l_os.h"

// checksum: computes SHA-256 hash of a file via l_sha256
// Usage: checksum <filename>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: checksum <filename>\n");
        return 0;
    }

    L_FD fd = open_read(argv[1]);
    exitif(fd < 0, 1, "Error: cannot open file\n");

    L_Sha256 ctx;
    l_sha256_init(&ctx);

    unsigned char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        l_sha256_update(&ctx, buf, (size_t)n);

    close(fd);

    unsigned char hash[32];
    l_sha256_final(&ctx, hash);

    char out[65];
    l_bin2hex(out, hash, 32);
    puts(out);
    puts("  ");
    puts(argv[1]);
    puts("\n");

    return 0;
}
