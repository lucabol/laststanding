#define L_MAINFILE
#include "l_os.h"

// Minimal checksum: computes XOR and additive checksums of a file
// Usage: checksum <filename>

static void write_hex32(unsigned int val) {
    char hex[9];
    const char *digits = "0123456789abcdef";
    for (int i = 7; i >= 0; i--) {
        hex[i] = digits[val & 0xf];
        val >>= 4;
    }
    hex[8] = '\0';
    write(STDOUT, hex, 8);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: checksum <filename>\n");
        return 0;
    }

    L_FD fd = open_read(argv[1]);
    exitif(fd < 0, 1, "Error: cannot open file\n");

    unsigned char buf[4096];
    ssize_t n;
    unsigned int xor_sum = 0;
    unsigned int add_sum = 0;
    unsigned long total_bytes = 0;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            xor_sum ^= buf[i];
            add_sum += buf[i];
        }
        total_bytes += n;
    }

    close(fd);

    puts("XOR: ");
    write_hex32(xor_sum);
    puts("  SUM: ");
    write_hex32(add_sum);
    puts("  ");

    char numbuf[20];
    itoa(total_bytes, numbuf, 10);
    puts(numbuf);
    puts(" bytes  ");
    puts(argv[1]);
    puts("\n");

    return 0;
}
