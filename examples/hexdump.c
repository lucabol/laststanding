#define L_MAINFILE
#include "l_os.h"

// Minimal hexdump: displays file contents in hex + ASCII format
// Usage: hexdump <filename>

static void write_hex_byte(unsigned char b) {
    char hex[3];
    const char *digits = "0123456789abcdef";
    hex[0] = digits[(b >> 4) & 0xf];
    hex[1] = digits[b & 0xf];
    hex[2] = ' ';
    write(STDOUT, hex, 3);
}

static void write_offset(unsigned long off) {
    char buf[9];
    const char *digits = "0123456789abcdef";
    for (int i = 7; i >= 0; i--) {
        buf[i] = digits[off & 0xf];
        off >>= 4;
    }
    buf[8] = ' ';
    write(STDOUT, buf, 9);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: hexdump <filename>\n");
        return 0;
    }

    L_FD fd = open_read(argv[1]);
    exitif(fd < 0, 1, "Error: cannot open file\n");

    unsigned char buf[16];
    ssize_t n;
    unsigned long offset = 0;

    while ((n = read(fd, buf, 16)) > 0) {
        write_offset(offset);

        // Hex section
        for (int i = 0; i < 16; i++) {
            if (i == 8) write(STDOUT, " ", 1);
            if (i < n)
                write_hex_byte(buf[i]);
            else
                write(STDOUT, "   ", 3);
        }

        write(STDOUT, " |", 2);

        // ASCII section
        for (int i = 0; i < n; i++) {
            char c = l_isprint(buf[i]) ? buf[i] : '.';
            write(STDOUT, &c, 1);
        }

        write(STDOUT, "|\n", 2);
        offset += n;
    }

    // Final offset line
    write_offset(offset);
    write(STDOUT, "\n", 1);

    close(fd);
    return 0;
}
