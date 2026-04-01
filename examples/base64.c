#define L_MAINFILE
#include "l_os.h"

// base64: RFC 4648 base64 encoder/decoder
// Usage: base64 [-d] [-h/--help] [file]

static const char b64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Buffered input */
#define IBUF_SIZE 4096
static unsigned char ibuf[IBUF_SIZE];
static int ibuf_pos, ibuf_len;
static L_FD ibuf_fd;

static void ibuf_init(L_FD fd) {
    ibuf_fd = fd;
    ibuf_pos = ibuf_len = 0;
}

static int ibuf_get(void) {
    if (ibuf_pos >= ibuf_len) {
        ssize_t r = l_read(ibuf_fd, ibuf, IBUF_SIZE);
        if (r <= 0) return -1;
        ibuf_len = (int)r;
        ibuf_pos = 0;
    }
    return ibuf[ibuf_pos++];
}

/* Buffered output */
#define OBUF_SIZE 4096
static char obuf[OBUF_SIZE];
static int obuf_pos;

static void obuf_flush(void) {
    if (obuf_pos > 0) {
        l_write(L_STDOUT, obuf, (size_t)obuf_pos);
        obuf_pos = 0;
    }
}

static void obuf_put(char c) {
    obuf[obuf_pos++] = c;
    if (obuf_pos >= OBUF_SIZE) obuf_flush();
}

/* Decode one base64 character to 0..63, or -1 for invalid */
static int b64_val(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

static void do_encode(L_FD fd) {
    ibuf_init(fd);
    obuf_pos = 0;
    int col = 0;

    for (;;) {
        int b0 = ibuf_get();
        if (b0 < 0) break;
        int b1 = ibuf_get();
        int b2 = (b1 >= 0) ? ibuf_get() : -1;

        int i1 = (b1 >= 0) ? b1 : 0;
        int i2 = (b2 >= 0) ? b2 : 0;

        obuf_put(b64_table[b0 >> 2]);
        obuf_put(b64_table[((b0 & 0x03) << 4) | (i1 >> 4)]);
        obuf_put((b1 >= 0) ? b64_table[((i1 & 0x0F) << 2) | (i2 >> 6)] : '=');
        obuf_put((b2 >= 0) ? b64_table[i2 & 0x3F] : '=');
        col += 4;

        if (col >= 76) {
            obuf_put('\n');
            col = 0;
        }

        if (b1 < 0 || b2 < 0) break;
    }

    if (col > 0) obuf_put('\n');
    obuf_flush();
}

static void do_decode(L_FD fd) {
    ibuf_init(fd);
    obuf_pos = 0;

    for (;;) {
        int vals[4];
        int pads = 0;
        int got = 0;

        while (got < 4) {
            int c = ibuf_get();
            if (c < 0) {
                if (got == 0) goto done;
                while (got < 4) { vals[got++] = 0; pads++; }
                break;
            }
            if (c == '=') {
                vals[got++] = 0;
                pads++;
            } else if (l_isspace(c)) {
                continue;
            } else {
                int v = b64_val(c);
                if (v < 0) continue;
                vals[got++] = v;
            }
        }

        obuf_put((char)((vals[0] << 2) | (vals[1] >> 4)));
        if (pads < 2)
            obuf_put((char)(((vals[1] & 0x0F) << 4) | (vals[2] >> 2)));
        if (pads < 1)
            obuf_put((char)(((vals[2] & 0x03) << 6) | vals[3]));

        if (pads > 0) break;
    }
done:
    obuf_flush();
}

static void usage(void) {
    l_puts("Usage: base64 [-d] [-h/--help] [file]\n");
    l_puts("Base64 encode or decode.\n\n");
    l_puts("  -d        decode mode\n");
    l_puts("  --help    display this help and exit\n");
    l_puts("  file      read from file instead of stdin\n");
}

int main(int argc, char *argv[]) {
    int decode = 0;
    const char *file = (const char *)0;

    for (int i = 1; i < argc; i++) {
        if (l_strcmp(argv[i], "-d") == 0) {
            decode = 1;
        } else if (l_strcmp(argv[i], "-h") == 0 ||
                   l_strcmp(argv[i], "--help") == 0) {
            usage();
            return 0;
        } else if (argv[i][0] == '-') {
            l_puts("base64: unknown option '");
            l_puts(argv[i]);
            l_puts("'\n");
            return 1;
        } else {
            file = argv[i];
        }
    }

    L_FD fd = L_STDIN;
    if (file) {
        fd = l_open_read(file);
        if (fd < 0) {
            l_puts("base64: cannot open '");
            l_puts(file);
            l_puts("'\n");
            return 1;
        }
    }

    if (decode)
        do_decode(fd);
    else
        do_encode(fd);

    if (file) l_close(fd);
    return 0;
}
