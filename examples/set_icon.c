// Demonstrates l_set_exe_icon: builds a minimal 16x16 .ico file on disk and
// applies it to a target .exe (Windows). On non-Windows platforms
// l_set_exe_icon is a no-op and the program exits successfully.
//
// Usage:
//   set_icon                   -- builds test_icon.ico in the cwd only
//   set_icon <target.exe>      -- also patches the target executable's icon
//
// Note: the target exe must not be currently running (Windows locks it).
// Try copying a binary first, e.g.
//   copy bin\hello.exe bin\hello_copy.exe
//   bin\set_icon bin\hello_copy.exe

#define L_MAINFILE
#include "l_os.h"

// Build a minimal valid 1-bit 16x16 .ico (198 bytes, all pixels opaque white).
static size_t build_test_ico(unsigned char *p) {
    unsigned char *start = p;

    // ICONDIR: reserved=0, type=1 (icon), count=1
    *p++ = 0; *p++ = 0; *p++ = 1; *p++ = 0; *p++ = 1; *p++ = 0;

    // ICONDIRENTRY (16 bytes)
    *p++ = 16; *p++ = 16;                     // 16x16
    *p++ = 2;  *p++ = 0;                      // 2 colors, reserved
    *p++ = 1;  *p++ = 0;                      // planes = 1
    *p++ = 1;  *p++ = 0;                      // bitcount = 1
    *p++ = 176;*p++ = 0; *p++ = 0; *p++ = 0;  // bytesInRes = 176
    *p++ = 22; *p++ = 0; *p++ = 0; *p++ = 0;  // image offset = 22

    // BITMAPINFOHEADER (40 bytes): 16 wide, 32 tall (xor+and), 1bpp
    static const unsigned char bih[40] = {
        40,0,0,0,  16,0,0,0,  32,0,0,0,
         1,0,       1,0,       0,0,0,0,
         0,0,0,0,   0,0,0,0,   0,0,0,0,
         2,0,0,0,   0,0,0,0,
    };
    for (int i = 0; i < 40; i++) *p++ = bih[i];

    // Palette: index 0 = black, index 1 = white (BGRA)
    *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00;
    *p++ = 0xFF; *p++ = 0xFF; *p++ = 0xFF; *p++ = 0x00;

    // XOR mask: 16 rows, DWORD-aligned (2 data bytes + 2 padding).
    // All pixels set to color index 1 (white).
    for (int r = 0; r < 16; r++) {
        *p++ = 0xFF; *p++ = 0xFF; *p++ = 0x00; *p++ = 0x00;
    }

    // AND mask: 16 rows, DWORD-aligned, all zero = fully opaque.
    for (int i = 0; i < 64; i++) *p++ = 0x00;

    return (size_t)(p - start);
}

int main(int argc, char *argv[]) {
    unsigned char ico[256];
    size_t n = build_test_ico(ico);

    const char *ico_path = "test_icon.ico";
    L_FD fd = l_open_trunc(ico_path);
    if (fd < 0) {
        puts("error: could not create test_icon.ico\n");
        return 1;
    }
    if (l_write_all(fd, ico, n) != (ptrdiff_t)n) {
        l_close(fd);
        puts("error: could not write test_icon.ico\n");
        return 1;
    }
    l_close(fd);
    puts("wrote test_icon.ico\n");

    if (argc < 2) {
        puts("no target .exe given; skipping icon patch\n");
        return 0;
    }

    if (l_set_exe_icon(argv[1], ico_path) != 0) {
        puts("error: l_set_exe_icon failed\n");
        return 1;
    }
    puts("icon updated\n");
    return 0;
}
