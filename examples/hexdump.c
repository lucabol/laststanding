#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_os.h"

// Minimal hexdump: displays file contents in hex + ASCII format
// Usage: hexdump <filename>

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
        l_printf("%08lx ", offset);

        // Hex section
        for (int i = 0; i < 16; i++) {
            if (i == 8) l_printf(" ");
            if (i < n)
                l_printf("%02x ", (unsigned int)buf[i]);
            else
                l_printf("   ");
        }

        l_printf(" |");

        // ASCII section
        for (int i = 0; i < n; i++)
            l_printf("%c", l_isprint(buf[i]) ? (char)buf[i] : '.');

        l_printf("|\n");
        offset += (unsigned long)n;
    }

    // Final offset line
    l_printf("%08lx\n", offset);

    close(fd);
    return 0;
}
