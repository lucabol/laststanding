# laststanding

A freestanding C runtime — zero dependencies, direct syscalls, tiny binaries. Five header files give you everything from `strlen` to pixel graphics to image decoding to TLS/HTTPS to interactive UI widgets, across **Linux** (x86_64, ARM, AArch64, RISC-V), **Windows**, and **WASI (WebAssembly)** (experimental), with no libc at all.

| Header | What it provides |
|--------|-----------------|
| `l_os.h` | String/memory functions, file I/O, processes, pipes, terminal control, environment access, hash maps, SHA-256, glob matching, time formatting |
| `l_gfx.h` | Pixel graphics — drawing primitives, scaled bitmap font, pixel blitting, alpha blending, keyboard/mouse input (X11 / Linux framebuffer / Windows GDI) |
| `l_img.h` | Image decoding — PNG, JPEG, BMP, GIF, TGA from memory buffers via vendored stb_image (freestanding, no libc) |
| `l_tls.h` | TLS/HTTPS client — SChannel on Windows, BearSSL on Linux (zero deps on both). Up to 8 simultaneous connections |
| `l_ui.h` | Immediate-mode UI — buttons, checkboxes, sliders, text inputs, layout helpers (built on `l_gfx.h`) |

Binaries are statically linked, stripped, and typically **2–10 KB**. The project includes 13 Unix-style utilities, 4 interactive programs (text editor, shell, snake, fractal renderer), 7 graphical demos, an image viewer, an HTTPS client, and 2 UI demos — all built without a single line of libc.

## Quick Start

### Hello World (`l_os.h`)

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    puts("Hello from laststanding!\n");
    return 0;
}
```

```sh
gcc -I. -Oz -ffreestanding -nostdlib -static -o hello hello.c   # Linux
clang -I. -Oz -lkernel32 -ffreestanding -o hello.exe hello.c    # Windows
```

### File I/O (`l_os.h`)

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    L_FD fd = l_open_write("greeting.txt");
    l_write(fd, "Hello!\n", 7);
    l_close(fd);

    char buf[64];
    fd = l_open_read("greeting.txt");
    ptrdiff_t n = l_read(fd, buf, sizeof(buf));
    l_close(fd);

    l_write(L_STDOUT, buf, n);
    return 0;
}
```

### Spawn a Child Process (`l_os.h`)

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    // Run "upper" from the same bin/ directory
    char *child_argv[] = { "upper", NULL };
    L_FD pid = l_spawn("bin/upper", child_argv, NULL);
    int code;
    l_wait(pid, &code);

    char msg[32];
    l_itoa(code, msg, 10);
    puts("Exit code: "); puts(msg); puts("\n");
    return 0;
}
```

### Pixel Graphics (`l_gfx.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"       // pulls in l_os.h automatically

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Demo") != 0) return 1;

    l_canvas_clear(&c, L_BLACK);
    l_fill_rect(&c, 50, 50, 100, 80, L_RED);
    l_circle(&c, 200, 120, 60, L_GREEN);
    l_draw_text(&c, 60, 85, "Hello!", L_WHITE);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);
    l_canvas_close(&c);
    return 0;
}
```

### Animation Loop (`l_gfx.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Bounce") != 0) return 1;

    int x = 160, y = 120, dx = 2, dy = 1;
    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;
        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, x, y, 10, L_RED);
        l_canvas_flush(&c);
        x += dx; y += dy;
        if (x <= 10 || x >= 310) dx = -dx;
        if (y <= 10 || y >= 230) dy = -dy;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
```

### Image Decoding (`l_img.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"
#include "l_img.h"       // pulls in l_os.h + stb_image (freestanding)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    // Read file into memory, decode PNG/JPEG/BMP/GIF/TGA
    L_FD fd = l_open("photo.jpg", 0, 0);
    long long sz = l_lseek(fd, 0, 2); l_lseek(fd, 0, 0);
    unsigned char *buf = (unsigned char *)l_mmap(0, sz, L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);

    int w, h;
    uint32_t *pixels = l_img_load_mem(buf, (int)sz, &w, &h);  // → ARGB pixels
    l_munmap(buf, sz);
    if (!pixels) return 1;

    // Display in a window
    L_Canvas c;
    l_canvas_open(&c, w > 800 ? 800 : w, h > 600 ? 600 : h, "Image");
    l_blit(&c, 0, 0, w, h, pixels, w * 4);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27) l_sleep_ms(16);
    l_canvas_close(&c);
    l_img_free_pixels(pixels, w, h);
    return 0;
}
```

### HTTPS Client (`l_tls.h`)

```c
#define L_MAINFILE
#include "l_tls.h"       // pulls in l_os.h automatically

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    if (l_tls_init() != 0) return 1;

    int h = l_tls_connect("example.com", 443);
    if (h < 0) return 1;

    const char *req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
    l_tls_send(h, req, l_strlen(req));

    char buf[4096];
    int n;
    while ((n = l_tls_recv(h, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        puts(buf);
    }

    l_tls_close(h);
    l_tls_cleanup();
    return 0;
}
```

### UI Form (`l_ui.h`)

```c
#define L_MAINFILE
#include "l_ui.h"         // pulls in l_gfx.h and l_os.h automatically

int main(int argc, char *argv[]) {
    L_Canvas canvas;
    if (l_canvas_open(&canvas, 400, 300, "My App") != 0) return 1;

    L_UI ui;
    l_ui_init(&ui);

    int clicked = 0, checked = 0, volume = 50;
    char name[256] = "";

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(30, 30, 34));
        if (ui.key == 27) break;

        l_ui_panel(&ui, 10, 10, 380, 280);

        l_ui_column_begin(&ui, 20, 20, 8);

        int y = l_ui_next(&ui, 10);
        l_ui_label(&ui, 20, y, "Settings");

        y = l_ui_next(&ui, 24);
        if (l_ui_button(&ui, 20, y, 120, 24, "Click Me"))
            clicked++;

        y = l_ui_next(&ui, 16);
        l_ui_checkbox(&ui, 20, y, "Dark mode", &checked);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Volume:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &volume, 0, 100);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Name:");
        y = l_ui_next(&ui, 20);
        l_ui_textbox(&ui, 20, y, 200, name, 256);

        l_ui_layout_end(&ui);

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }
    l_canvas_close(&canvas);
    return 0;
}
```

### RGB Color Mixer (`l_ui.h`)

```c
#define L_MAINFILE
#include "l_ui.h"

int main(int argc, char *argv[]) {
    L_Canvas canvas;
    if (l_canvas_open(&canvas, 480, 300, "Color Mixer") != 0) return 1;

    L_UI ui;
    l_ui_init(&ui);
    int r = 128, g = 64, b = 200;

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(25, 25, 28));
        if (ui.key == 27) break;

        l_ui_column_begin(&ui, 20, 20, 8);

        int y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Red:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &r, 0, 255);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Green:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &g, 0, 255);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Blue:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &b, 0, 255);

        l_ui_layout_end(&ui);

        // Color preview
        l_fill_rect(&canvas, 250, 20, 210, 260, L_RGB(r, g, b));

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }
    l_canvas_close(&canvas);
    return 0;
}
```

### Dynamic Memory (Arena Allocator)

laststanding provides an arena (bump) allocator for programs that need dynamic memory without the complexity of malloc/free:

```c
L_Arena arena = l_arena_init(1024 * 1024);  // 1MB arena
char *name = l_arena_alloc(&arena, 256);     // bump-allocate 256 bytes
l_arena_reset(&arena);                       // reuse all memory
l_arena_free(&arena);                        // release to OS

L_Buf buf;
l_buf_init(&buf);
l_buf_push(&buf, "hello", 5);               // growable buffer
l_buf_printf(&buf, " world %d", 42);        // formatted append (needs L_WITHSNPRINTF)
l_buf_free(&buf);
```

### Fat Strings (`L_Str`)

`L_Str` is a pointer+length pair (16 bytes, passed by value) backed by `L_Arena`. Zero-copy operations (trim, substring, find, compare) just return views into existing data. Arena-backed operations (dup, cat, split, join, upper/lower) allocate from an arena.

```c
L_Arena a = l_arena_init(4096);

// Constructors — zero-copy views
L_Str s = l_str("  hello world  ");     // from C string literal
L_Str t = l_str_from(buf, len);         // from pointer + length

// Trim, substring, find — zero-copy
L_Str trimmed = l_str_trim(s);          // "hello world"
L_Str sub = l_str_sub(trimmed, 0, 5);   // "hello"
ptrdiff_t pos = l_str_find(trimmed, l_str("world"));  // 6

// Compare
if (l_str_eq(sub, l_str("hello"))) { /* ... */ }
if (l_str_startswith(trimmed, l_str("hello"))) { /* ... */ }

// Arena-backed operations
L_Str upper = l_str_upper(&a, sub);     // "HELLO"
L_Str joined = l_str_cat(&a, sub, l_str("!")); // "hello!"
char *cstr = l_str_cstr(&a, sub);       // null-terminated copy

// Split and join
L_Str *parts; int n;
n = l_str_split(&a, l_str("a:b:c"), l_str(":"), &parts);  // 3 parts
L_Str rejoined = l_str_join(&a, parts, n, l_str("-"));     // "a-b-c"

// L_Buf integration
L_Buf out; l_buf_init(&out);
l_buf_push_cstr(&out, "line ");
l_buf_push_int(&out, 42);
l_buf_push_str(&out, l_str(": ok"));
L_Str view = l_buf_as_str(&out);        // view of buf contents
l_buf_free(&out);

l_arena_free(&a);
```

### Hash Map (`L_Map`)

Arena-backed hash table using FNV-1a hashing and open addressing with linear probing. Fixed capacity (set at init), no dynamic resize, 75% load factor limit. Ideal for lookup tables and caches in freestanding programs.

```c
L_Arena a = l_arena_init(4096);
L_Map map = l_map_init(&a, 64);           // 64-slot table

l_map_put(&map, "name", "Alice");
l_map_put(&map, "lang", "C");

const char *v = l_map_get(&map, "name");   // "Alice"
l_map_del(&map, "lang");                   // tombstone deletion

l_arena_free(&a);
```

### I/O Multiplexing (`l_poll`)

Monitors multiple file descriptors for readiness. Uses `poll`/`ppoll` syscalls on Linux, `WaitForMultipleObjects` on Windows.

```c
L_PollFd fds[2];
fds[0].fd = sock1; fds[0].events = L_POLLIN;
fds[1].fd = sock2; fds[1].events = L_POLLIN;

int ready = l_poll(fds, 2, 1000);          // 1-second timeout
if (fds[0].revents & L_POLLIN) { /* sock1 readable */ }
```

### Signal Handling (`l_signal`)

Installs signal handlers via `rt_sigaction` on Linux, `SetConsoleCtrlHandler` on Windows. Windows supports `SIGINT` and `SIGTERM` only.

```c
void handler(int sig) { /* handle Ctrl+C */ }
l_signal(L_SIGINT, handler);
l_signal(L_SIGINT, L_SIG_DFL);            // restore default
```

### Date/Time (`l_gmtime`, `l_localtime`, `l_strftime`)

Converts Unix timestamps to broken-down time using the Rata Die algorithm. `l_localtime` uses `GetTimeZoneInformation` on Windows and `TZ` env var parsing on Linux. `l_strftime` supports `%Y/%m/%d/%H/%M/%S/%a/%b/%%`.

```c
L_Tm tm;
long ts = l_time(NULL);
l_gmtime(ts, &tm);

char buf[64];
l_strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
l_puts(buf);  // "2026-07-26 14:30:00"
```

### Pattern Matching (`l_fnmatch`)

Glob-style pattern matching with `*`, `?`, `[abc]`, `[a-z]`, and `\` escaping. Uses iterative backtracking for `*`.

```c
l_fnmatch("*.c", "main.c");               // 0 (match)
l_fnmatch("test_[0-9]*", "test_3_ok");    // 0 (match)
l_fnmatch("*.h", "main.c");               // 1 (no match)
```

### SHA-256 (`l_sha256`)

FIPS 180-4 SHA-256 implementation. One-shot convenience function or incremental init/update/final for streaming.

```c
unsigned char hash[32];
l_sha256("hello", 5, hash);               // one-shot

L_Sha256 ctx;
l_sha256_init(&ctx);
l_sha256_update(&ctx, "hel", 3);
l_sha256_update(&ctx, "lo", 2);
l_sha256_final(&ctx, hash);               // same result
```

### Scatter-Gather I/O (`l_writev`/`l_readv`)

Write/read multiple buffers in a single syscall. Uses `writev`/`readv` on Linux; loops per-buffer on Windows.

```c
L_IoVec vecs[2];
vecs[0].base = "Hello "; vecs[0].len = 6;
vecs[1].base = "World\n"; vecs[1].len = 6;
l_writev(L_STDOUT, vecs, 2);              // single syscall on Linux
```

### Environment Variables (`l_setenv`/`l_unsetenv`)

Modify environment variables at runtime. On Windows, uses `SetEnvironmentVariableW`. On Linux, manages a static pool (128 entries, 8 KB buffer) over the envp array. See also `l_getenv` for reading and `l_env_start`/`l_env_next`/`l_env_end` for iteration.

```c
l_setenv("MY_VAR", "hello");
const char *v = l_getenv("MY_VAR");        // "hello"
l_unsetenv("MY_VAR");
```

### Terminal Detection (`l_isatty`)

Returns 1 if the file descriptor refers to a terminal, 0 otherwise. Uses `ioctl(TCGETS)` on Linux, `GetConsoleMode` on Windows. Complements the existing `l_term_raw`/`l_term_restore`/`l_term_size` functions.

```c
if (l_isatty(L_STDOUT)) {
    l_puts("Running in a terminal\n");
} else {
    l_puts("Output is redirected\n");
}
```

### Convenience Helpers

High-level functions for common file and string operations:

```c
// Bulk file I/O — retry on short reads/writes
L_FD fd = l_open_write("data.bin");
l_write_all(fd, buf, size);              // write exactly size bytes
l_close(fd);

fd = l_open_read("data.bin");
ptrdiff_t n = l_read_all(fd, buf, max);  // read until EOF or max
l_close(fd);

// File metadata
long long sz = l_file_size("data.bin");  // -1 on error
l_truncate("data.bin", 1024);            // truncate by path
l_ftruncate(fd, 512);                    // truncate by open fd

// Shell command execution
int code = l_system("echo hello");       // /bin/sh -c on Linux, cmd.exe /c on Windows

// Glob expansion
L_Arena a = l_arena_init(4096);
L_Str *paths; int count;
l_glob("src/*.c", &paths, &count, &a);   // single-level wildcard expansion

// String replacement
L_Str result = l_str_replace(&a, l_str("hello world"), l_str("world"), l_str("C"));
// result: "hello C"
l_arena_free(&a);
```

## Compile-Time Flags

| Define | Purpose |
|--------|---------|
| `L_MAINFILE` | Activates function definitions and platform startup code. **Exactly one** translation unit must define this. |
| `L_DONTOVERRIDE` | Prevents `#define strlen l_strlen` aliases — use when mixing with standard headers. |
| `L_WITHSNPRINTF` | Enables `l_snprintf` / `l_vsnprintf` (opt-in to keep binaries small). |

By default, `l_os.h` aliases standard names (`strlen`, `memcpy`, `exit`, `puts`, …) to their `l_` equivalents so you can write familiar C. Define `L_DONTOVERRIDE` to disable this.

In multi-file projects, only one `.c` file defines `L_MAINFILE`:

```c
// utils.c — gets type definitions and constants only
#include "l_os.h"

// main.c — the one file with startup code and function bodies
#define L_MAINFILE
#include "l_os.h"
```

## Key Types

| Type | Purpose |
|------|---------|
| `L_FD` | File descriptor (`ptrdiff_t`). On Windows, a library-managed slot — not a raw `HANDLE`. |
| `L_STDIN`, `L_STDOUT`, `L_STDERR` | Standard descriptor constants (0, 1, 2). |
| `L_Str` | Fat string — pointer+length pair (16 bytes, by value). Zero-copy views or arena-backed. |
| `L_Arena` | Bump allocator. Init with `l_arena_init(size)`, allocate with `l_arena_alloc`. |
| `L_Buf` | Growable byte buffer (heap-backed via `realloc`). |
| `L_Canvas` | Pixel graphics context (framebuffer on Linux, GDI window on Windows). |
| `L_PollFd` | Poll descriptor — fd, events, revents for `l_poll`. |
| `L_IoVec` | I/O vector — base pointer + length for scatter-gather I/O. |
| `L_Map` | Arena-backed hash map — FNV-1a, open addressing, linear probing. |
| `L_Tm` | Broken-down time — year, month, day, hour, min, sec, weekday, yearday. |
| `L_Sha256` | SHA-256 hash context for incremental hashing. |
| `L_UI` | Immediate-mode UI context (theme, input state, layout cursor). |
| `L_UI_Theme` | UI color theme (background, hover, active, text, border, accent, input). |

## Compiler Flags

Binaries must be compiled freestanding:

| Platform | Command |
|----------|---------|
| Linux (gcc/clang) | `gcc -I. -Oz -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o app app.c` |
| Windows (clang) | `clang -I. -Oz -lkernel32 -ffreestanding -Wall -Wextra -Wpedantic -o app.exe app.c` |
| ARM32 cross | `arm-linux-gnueabihf-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| AArch64 cross | `aarch64-linux-gnu-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| RISC-V cross | `riscv64-linux-gnu-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| WASI (wasm32) | `clang --target=wasm32-wasi --sysroot=/path/to/wasi-sdk/share/wasi-sysroot -I. -O2 -ffreestanding -nostdlib -o app.wasm app.c` |

> **WASI support is experimental.** Core I/O (`l_read`, `l_write`, `l_open`, `l_close`, `l_lseek`) and environment access work. Process creation (`l_fork`, `l_spawn`), pipes, sockets, signals, and terminal control are stubbed (return -1). Memory mapping for anonymous allocations (`L_Arena`) works via `memory.grow`. Run with: `wasmtime --dir . app.wasm`

## Function Reference — `l_os.h`

Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

<!-- BEGIN FUNCTION REFERENCE -->

| Function | Description | Platform |
|----------|-------------|----------|
| **String functions** | | |
| `l_wcslen` | Returns the length of a wide character string | All |
| `l_strlen` | Returns the length of a null-terminated string | All |
| `l_strcpy` | Copies src string to dst, returns dst | All |
| `l_strncpy` | Copies up to n characters from src to dst, padding with nulls | All |
| `l_strcat` | Appends src string to dst, returns dst | All |
| `l_strncat` | Appends at most n characters of src to dst, always null-terminates, returns dst | All |
| `l_strchr` | Returns pointer to first occurrence of c in s, or NULL | All |
| `l_strrchr` | Returns pointer to last occurrence of c in s, or NULL | All |
| `l_strstr` | Returns pointer to first occurrence of s2 in s1, or NULL | All |
| `l_strcmp` | Compares two strings, returns <0, 0, or >0 | All |
| `l_strncmp` | Compares up to n characters of two strings | All |
| `l_strcasecmp` | Case-insensitive string comparison | All |
| `l_strncasecmp` | Case-insensitive comparison of up to n characters | All |
| `l_strspn` | Returns length of initial segment of s consisting entirely of bytes in accept | All |
| `l_strcspn` | Returns length of initial segment of s consisting entirely of bytes NOT in reject | All |
| `l_strpbrk` | Returns pointer to first occurrence in s of any character in accept, or NULL | All |
| `l_strtok_r` | Splits str into tokens delimited by any char in delim; saves state in *saveptr (reentrant) | All |
| `l_strsep` | Extracts token from *stringp delimited by any char in delim (BSD strsep); advances *stringp past delimiter | All |
| `l_bin2hex` | Converts binary data to lowercase hex string. NUL-terminates dst. Returns 2*len. | All |
| `l_hex2bin` | Converts hex string to binary data. Returns bytes written, or -1 on invalid input. | All |
| `l_basename` | Returns pointer to the filename component of path (after last '/' or '\') | All |
| `l_dirname` | Writes the directory component of path into buf (up to bufsize), returns buf | All |
| `l_path_join` | Joins directory and filename with separator, returns buf | All |
| `l_path_ext` | Returns pointer to extension including dot (".txt"), or "" if none | All |
| `l_path_exists` | Returns 1 if path exists, 0 if not | All |
| `l_path_isdir` | Returns 1 if path is a directory, 0 if not | All |
| `l_reverse` | Reverses a string in place | All |
| **Conversion functions** | | |
| `l_isspace` | Returns non-zero if c is a whitespace character (space, tab, newline, etc.) | All |
| `l_isdigit` | Returns non-zero if c is a digit ('0'-'9') | All |
| `l_isalpha` | Returns non-zero if c is an alphabetic character ('A'-'Z' or 'a'-'z') | All |
| `l_isalnum` | Returns non-zero if c is alphanumeric (l_isalpha or l_isdigit) | All |
| `l_isupper` | Returns non-zero if c is an uppercase letter ('A'-'Z') | All |
| `l_islower` | Returns non-zero if c is a lowercase letter ('a'-'z') | All |
| `l_toupper` | Converts c to uppercase; returns c unchanged if not a lowercase letter | All |
| `l_tolower` | Converts c to lowercase; returns c unchanged if not an uppercase letter | All |
| `l_isprint` | Returns non-zero if c is a printable ASCII character (0x20-0x7e) | All |
| `l_isxdigit` | Returns non-zero if c is a hexadecimal digit (0-9, a-f, A-F) | All |
| `l_abs` | Returns the absolute value of an integer | All |
| `l_labs` | Returns the absolute value of a long | All |
| `l_llabs` | Returns the absolute value of a long long | All |
| `l_atol` | Converts a string to a long integer, skipping leading whitespace | All |
| `l_atoi` | Converts a string to an integer | All |
| `l_strtoul` | Converts a string to an unsigned long, auto-detecting base when base==0 (0x=hex, 0=octal, else decimal); sets *endptr past last digit | All |
| `l_strtol` | Converts a string to a long, auto-detecting base when base==0; handles leading sign; sets *endptr past last digit | All |
| `l_strtoull` | Converts a string to an unsigned long long (64-bit); auto-detects base when base==0; sets *endptr past last digit | All |
| `l_strtoll` | Converts a string to a long long (64-bit); auto-detects base when base==0; handles leading sign; sets *endptr past last digit | All |
| `l_strtod` | Converts a string to a double; skips leading whitespace; handles sign, decimal point, and e/E exponent; sets *endptr past last digit | All |
| `l_atof` | Converts a string to a double (convenience wrapper around l_strtod) | All |
| `l_strtof` | Converts a string to a float; skips leading whitespace; handles sign, decimal point, and e/E exponent; sets *endptr past last digit | All |
| **Math functions** | | |
| `l_fabs` | Returns the absolute value of a double | All |
| `l_floor` | Rounds toward negative infinity | All |
| `l_ceil` | Rounds toward positive infinity | All |
| `l_fmod` | Floating-point remainder of x/y | All |
| `l_sqrt` | Square root via Newton-Raphson with IEEE bit hack seed | All |
| `l_sin` | Sine via range reduction and Taylor series | All |
| `l_cos` | Cosine: l_sin(x + pi/2) | All |
| `l_exp` | Exponential function via range reduction and Taylor series | All |
| `l_log` | Natural logarithm via mantissa/exponent decomposition | All |
| `l_pow` | Power: base^exp via exp(exp * log(base)) | All |
| `l_atan2` | Two-argument arctangent with quadrant handling | All |
| `l_tan` | Tangent: sin(x)/cos(x) | All |
| `l_asin` | Inverse sine via Newton's method, valid for [-1,1] | All |
| `l_acos` | Inverse cosine: pi/2 - asin(x) | All |
| `l_atan` | Inverse tangent: asin(x/sqrt(1+x*x)) | All |
| `l_log10` | Base-10 logarithm: log(x)/log(10) | All |
| `l_log2` | Base-2 logarithm: log(x)/log(2) | All |
| `l_round` | Round to nearest integer (halfway rounds away from zero) | All |
| `l_trunc` | Truncate toward zero | All |
| `l_hypot` | Euclidean distance, overflow-safe: sqrt(x*x+y*y) | All |
| `l_itoa` | Converts an integer to a string in the given radix (2-36) | All |
| **Memory functions** | | |
| `l_memmove` | Copies len bytes from src to dst, handling overlapping regions | All |
| `l_memset` | Fills len bytes of dst with byte value b | All |
| `l_memcmp` | Compares n bytes of s1 and s2, returns <0, 0, or >0 | All |
| `l_memcpy` | Copies len bytes from src to dst | All |
| `l_memchr` | Finds first occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_memrchr` | Finds last occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_strnlen` | Returns the length of s, but at most maxlen (does not scan past maxlen bytes) | All |
| `l_memmem` | Finds first occurrence of needle (needlelen bytes) in haystack (haystacklen bytes), or NULL | All |
| **Random number generation (xorshift32, single-threaded)** | | |
| `l_srand` | Seeds the pseudo-random number generator | All |
| `l_rand` | Returns a pseudo-random unsigned int (xorshift32) | All |
| `l_rand_ctx_init` | Initialize an independent RNG context | All |
| `l_srand_ctx` | Seed an independent RNG context | All |
| `l_rand_ctx` | Returns a pseudo-random unsigned int from an independent context | All |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | |
| `l_vsnprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| `l_snprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| `l_dprintf` | Writes formatted output to file descriptor fd. Returns number of bytes written. | All |
| `l_printf` | Writes formatted output to stdout. Returns number of bytes written. | All |
| `l_vfprintf` | Writes formatted output to file descriptor fd via va_list. Returns number of bytes written. | All |
| `l_vprintf` | Writes formatted output to stdout via va_list. Returns number of bytes written. | All |
| `l_fprintf` | Writes formatted output to file descriptor fd. Returns number of bytes written. | All |
| **System functions** | | |
| `l_exit` | Terminates the process with the given status code | All |
| `l_open` | Opens a file with the given flags and mode, returns file descriptor | All |
| `l_close` | Closes a file descriptor | All |
| `l_read` | Reads up to count bytes from fd into buf | All |
| `l_write` | Writes up to count bytes from buf to fd | All |
| `l_read_line` | Reads one line from fd into buf (up to bufsz-1 bytes). Strips the newline. | All |
| `l_linebuf_init` | Initialise a buffered line reader wrapping fd. | All |
| `l_linebuf_read` | Read one line into out (up to outsz-1 bytes). Strips newline. Buffers reads in 4096-byte chunks. | All |
| `l_time` | Returns current Unix timestamp (seconds since 1970-01-01). Also writes to *t if non-NULL. | All |
| `l_puts` | Writes a string to stdout | All |
| `l_exitif` | Exits with code and message if condition is true | All |
| `l_getenv` | Returns value of environment variable, or NULL if not found | All |
| `l_getenv_init` | Initializes environment variable access (call from main) | All |
| `l_env_start` | Begin iterating environment variables. Returns opaque handle (pass to l_env_end). | All |
| `l_env_next` | Get next "KEY=VALUE" string. buf/bufsz provide conversion space (Windows). | All |
| `l_env_end` | End iteration and free resources. | All |
| `l_find_executable` | Finds an executable by name, searching PATH if needed. | All |
| **Option parsing (single-threaded; state in static variables)** | | |
| `l_getopt` | Parses command-line options. optstring lists valid option chars; trailing ':' means the option | All |
| `l_getopt_ctx_init` | Initialize an independent option parser context | All |
| `l_getopt_ctx` | Reentrant getopt using an independent context. Same semantics as l_getopt. | All |
| **Convenience file openers** | | |
| `l_open_read` | Opens a file for reading | All |
| `l_open_write` | Opens or creates a file for writing | All |
| `l_open_readwrite` | Opens or creates a file for reading and writing | All |
| `l_open_append` | Opens or creates a file for appending | All |
| `l_open_trunc` | Opens or creates a file, truncating to zero length | All |
| **Error reporting** | | |
| `l_errno` | Returns the error code from the most recent failed syscall (0 if last call succeeded) | All |
| `l_strerror` | Returns a human-readable string for the given error code | All |
| **Terminal and timing functions (cross-platform)** | | |
| `l_sleep_ms` | Sleeps for the given number of milliseconds | All |
| `l_term_raw` | Sets stdin to raw mode (no echo, no line buffering), returns old mode | All |
| `l_term_restore` | Restores terminal mode from value returned by l_term_raw | All |
| `l_read_nonblock` | Reads from fd without blocking, returns 0 if no data available | All |
| `l_term_size` | Gets terminal size in rows and columns | All |
| **ANSI terminal helpers** | | |
| `l_ansi_move` | Writes cursor-move sequence into buf, returns bytes written | All |
| `l_ansi_color` | Writes color sequence into buf; fg/bg are 0-7 ANSI colors, -1 for default | All |
| **File system functions (cross-platform)** | | |
| `l_unlink` | Deletes a file, returns 0 on success, -1 on error | All |
| `l_rmdir` | Removes an empty directory, returns 0 on success, -1 on error | All |
| `l_rename` | Renames (or moves) a file or directory. Returns 0 on success, -1 on error. | All |
| `l_access` | Checks access to a file. mode: L_F_OK (exists), L_R_OK, L_W_OK, L_X_OK. Returns 0 if ok, -1 on error. | All |
| `l_chmod` | Changes permission bits of a file. Returns 0 on success, -1 on error. | All |
| `l_symlink` | Creates a symbolic link at linkpath pointing to target. Returns 0 on success, -1 on error. | All |
| `l_readlink` | Reads the target of a symbolic link into buf (up to bufsiz bytes). Returns number of bytes read, or -1 on error. | All |
| `l_realpath` | Resolves path to its canonical absolute form into resolved (at least L_PATH_MAX bytes). Returns resolved on success, NULL on error. | All |
| `l_stat` | Gets file metadata by path. Returns 0 on success, -1 on error. | All |
| `l_fstat` | Gets file metadata by open file descriptor. Returns 0 on success, -1 on error. | All |
| `l_truncate` | Truncates a file at the given path to the specified size. Returns 0 on success, -1 on error. | All |
| `l_ftruncate` | Truncates an open file descriptor to the specified size. Returns 0 on success, -1 on error. | All |
| `l_file_size` | Returns the size of a file in bytes, or -1 on error. | All |
| `l_read_all` | Reads exactly count bytes, retrying on short reads. Returns total bytes read, 0 on EOF, or negative on error. | All |
| `l_write_all` | Writes exactly count bytes, retrying on short writes. Returns total bytes written, or negative on error. | All |
| `l_opendir` | Opens a directory for reading. Returns 0 on success, -1 on error. | All |
| `l_readdir` | Reads the next directory entry. Returns pointer to L_DirEntry or NULL when done. | All |
| `l_closedir` | Closes a directory handle. | All |
| `l_mmap` | Maps a file or anonymous memory into the process address space | All |
| `l_munmap` | Unmaps a previously mapped region | All |
| `l_getrandom` | Fill buf with len bytes of cryptographic-quality random data (getrandom(2) on Linux, BCryptGenRandom on Windows). Returns 0 on success, -1 on error. | All |
| **Arena function declarations** | | |
| `l_arena_init` | Allocate an arena of `size` bytes via mmap. On failure, base=NULL. | All |
| `l_arena_alloc` | Bump-allocate n bytes (8-byte aligned). Returns NULL if arena is full. | All |
| `l_arena_reset` | Reset used to 0. Memory is NOT freed â€” arena can be reused. | All |
| `l_arena_free` | Free the backing memory. Sets base=NULL. | All |
| **Buffer function declarations** | | |
| `l_buf_init` | Zero-initialize a buffer. | All |
| `l_buf_push` | Append n bytes. Returns 0 on success, -1 on failure. | All |
| `l_buf_printf` | Formatted append using l_vsnprintf. Returns bytes written or -1. | All |
| `l_buf_clear` | Set len=0 (keep allocated memory). | All |
| `l_buf_free` | Free backing memory and zero the struct. | All |
| **L_Str â€” fat string (pointer + length) function declarations** | | |
| `l_str` | Wrap a C string (computes strlen). | All |
| `l_str_from` | Wrap pointer+length. | All |
| `l_str_null` | Return null string {NULL, 0}. | All |
| `l_str_eq` | 1 if equal, 0 otherwise. | All |
| `l_str_cmp` | Lexicographic compare (like strcmp). | All |
| `l_str_startswith` | 1 if s starts with prefix. | All |
| `l_str_endswith` | 1 if s ends with suffix. | All |
| `l_str_contains` | 1 if s contains needle. | All |
| `l_str_sub` | Substring (zero-copy). | All |
| `l_str_trim` | Trim leading+trailing whitespace (zero-copy). | All |
| `l_str_ltrim` | Trim leading whitespace (zero-copy). | All |
| `l_str_rtrim` | Trim trailing whitespace (zero-copy). | All |
| `l_str_chr` | Find char in string, -1 if not found. | All |
| `l_str_rchr` | Find last occurrence of char, -1 if not found. | All |
| `l_str_find` | Find substring, -1 if not found. | All |
| `l_str_dup` | Copy string into arena. | All |
| `l_str_cat` | Concatenate two strings into arena. | All |
| `l_str_cstr` | Null-terminated C string copy in arena. | All |
| `l_str_from_cstr` | strdup into arena as L_Str. | All |
| `l_str_split` | Split string by delimiter. Returns count; *out is arena-allocated array. | All |
| `l_str_join` | Join strings with separator. | All |
| `l_str_upper` | Uppercase copy in arena (ASCII). | All |
| `l_str_lower` | Lowercase copy in arena (ASCII). | All |
| `l_str_replace` | Replace all occurrences of find with repl in s. Result is arena-allocated. | All |
| `l_buf_push_str` | Append L_Str to buf. Returns 0 on success, -1 on failure. | All |
| `l_buf_push_cstr` | Append C string to buf. Returns 0 on success, -1 on failure. | All |
| `l_buf_push_int` | Append decimal int to buf. Returns 0 on success, -1 on failure. | All |
| `l_buf_as_str` | Return L_Str view of buf contents. | All |
| **I/O multiplexing** | | |
| `l_poll` | Poll file descriptors for events. Returns number ready, 0 on timeout, -1 on error. | All |
| **Signal handling** | | |
| `l_signal` | Set signal handler. Returns previous handler or L_SIG_DFL on error. | All |
| **Environment manipulation** | | |
| `l_setenv` | Set environment variable. Returns 0 on success, -1 on error. | All |
| `l_unsetenv` | Unset environment variable. Returns 0 on success, -1 on error. | All |
| **Scatter-gather I/O** | | |
| `l_writev` | Write from multiple buffers. Returns bytes written or -1 on error. | All |
| `l_readv` | Read into multiple buffers. Returns bytes read or -1 on error. | All |
| **Terminal detection** | | |
| `l_isatty` | Returns 1 if fd is a terminal, 0 otherwise. | All |
| **Hash map (arena-backed, fixed capacity)** | | |
| `l_map_init` | Initialize a map with given capacity (rounded to power of 2). | All |
| `l_map_get` | Get value by key. Returns value pointer or NULL if not found. | All |
| `l_map_put` | Put key-value pair. Returns 0 on success, -1 if full (>75% load). | All |
| `l_map_del` | Delete key. Returns 0 on success, -1 if not found. | All |
| **Time conversion** | | |
| `l_gmtime` | Convert Unix timestamp to UTC broken-down time. | All |
| `l_localtime` | Convert Unix timestamp to local broken-down time. | All |
| `l_mktime` | Convert UTC broken-down time to Unix timestamp (seconds since 1970-01-01 00:00:00 UTC). | All |
| `l_strftime` | Format time into buffer. Returns bytes written (excluding NUL). | All |
| **Glob pattern matching** | | |
| `l_fnmatch` | Match pattern against string. Returns 0 if matches, -1 if no match. | All |
| `l_glob` | Expand a glob pattern into matching paths. Single-level only (no recursive **). | All |
| **SHA-256** | | |
| `l_sha256_init` | Initialize SHA-256 context. | All |
| `l_sha256_update` | Feed data into SHA-256. | All |
| `l_sha256_final` | Finalize and produce 32-byte hash. | All |
| `l_sha256` | One-shot SHA-256. | All |
| `l_base64_encode` | Encode `len` bytes from `data` into standard Base64. Writes at most `outsz` bytes (including NUL) | All |
| `l_base64_decode` | Decode Base64 text of length `inlen` into `out`. Returns decoded byte count, or -1 on invalid input | All |
| `l_getcwd` | Gets the current working directory into buf (up to size bytes). Returns buf on success, NULL on error. | All |
| `l_chdir` | Changes the current working directory | All |
| `l_pipe` | Creates a pipe. fds[0] is the read end, fds[1] is the write end. Returns 0 on success, -1 on error. | All |
| `l_dup` | Duplicates fd, returning a new descriptor on success or -1 on error. | All |
| `l_dup2` | Duplicates oldfd onto newfd. Returns newfd on success, -1 on error. | All |
| `l_getpid` | Returns the current process ID. | All |
| `l_spawn_stdio` | Spawns a new process with explicit stdio. Use L_SPAWN_INHERIT to keep the parent's stream. | All |
| `l_spawn` | Spawns a new process, inheriting the current stdio descriptors. | All |
| `l_wait` | Waits for a spawned process to finish. Returns 0 on success, -1 on error. | All |
| `l_system` | Executes a shell command string. Returns the exit code, or -1 on spawn failure. | All |
| **Unix and WASI functions** | | |
| `l_lseek` | Repositions the file offset of fd | Unix |
| `l_mkdir` | Creates a directory with the given permissions | Unix |
| `l_sched_yield` | Yields the processor to other threads | Unix |
| `l_fork` | Fork the current process. Returns child pid to parent, 0 to child, -1 on error. | Unix |
| `l_execve` | Replace the current process image. Does not return on success. | Unix |
| `l_waitpid` | Wait for a child process. Returns child pid on success, -1 on error. | Unix |
| `l_getppid` | Returns the parent process ID. | Unix |
| `l_kill` | Sends signal sig to process pid. Returns 0 on success, -1 on error. | Unix |
| **Byte order helpers** | | |
| `l_htons` | Convert 16-bit value from host to network byte order | All |
| `l_htonl` | Convert 32-bit value from host to network byte order | All |
| `l_inet_addr` | Parse dotted-quad IP string to network-order u32. Returns 0 on error. | All |
| `l_resolve` | Resolve hostname to IPv4 dotted-quad string. ip_out must be at least 16 bytes. If hostname is already IPv4 text, copies it unchanged. Returns 0 on success, -1 on error. | All |
| **TCP socket functions** | | |
| `l_socket_tcp` | Create a TCP socket. Returns socket fd or -1 on error. | All |
| `l_socket_connect` | Connect to addr:port. Returns 0 on success, -1 on error. | All |
| `l_socket_bind` | Bind socket to port on all interfaces. Returns 0/-1. | All |
| `l_socket_listen` | Listen for connections. Returns 0/-1. | All |
| `l_socket_accept` | Accept connection. Returns new socket or -1. | All |
| `l_socket_send` | Send data. Returns bytes sent or -1. | All |
| `l_socket_recv` | Receive data. Returns bytes received, 0 on close, -1 on error. | All |
| `l_socket_close` | Close socket. | All |
| **UDP socket functions** | | |
| `l_socket_udp` | Create a UDP socket. Returns socket fd or -1 on error. | All |
| `l_socket_sendto` | Send data to addr:port via UDP. Returns bytes sent or -1. | All |
| `l_socket_recvfrom` | Receive data via UDP. addr_out (>=16 bytes) and port_out receive sender info. Returns bytes received or -1. | All |
| **Generic address-based socket API (IPv4 and IPv6)** | | |
| `l_sockaddr_ipv4` | Build an IPv4 L_SockAddr from dotted-quad string and port. Returns 0 on success, -1 on error. | All |
| `l_sockaddr_ipv6` | Build an IPv6 L_SockAddr from IPv6 text and port. Returns 0 on success, -1 on error. | All |
| `l_parse_ipv6` | Parse IPv6 text representation into 16-byte binary. Returns 1 on success, 0 on error. | All |
| `l_format_ipv6` | Format 16-byte IPv6 binary to text. buf must be at least L_INET6_ADDRSTRLEN bytes. Returns buf. | All |
| `l_socket_open` | Create a socket of the given family (L_AF_INET or L_AF_INET6) and type (L_SOCK_STREAM or L_SOCK_DGRAM). Returns socket fd or -1. | All |
| `l_socket_connect_addr` | Connect socket to an L_SockAddr. Returns 0 on success, -1 on error. | All |
| `l_socket_bind_addr` | Bind socket to an L_SockAddr. Returns 0 on success, -1 on error. | All |
| `l_socket_sendto_addr` | Send data to an L_SockAddr via UDP. Returns bytes sent or -1. | All |
| `l_socket_recvfrom_addr` | Receive data via UDP. src receives sender address. Returns bytes received or -1. | All |
| `l_socket_unix_connect` | Create a Unix domain socket and connect to the given path. Returns socket fd or -1. | All |

<!-- END FUNCTION REFERENCE -->

## Function Reference — `l_gfx.h`

Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

<!-- BEGIN GFX REFERENCE -->

| Function | Description |
|----------|-------------|
| **Color helpers** | |
| `L_RGB` | Composes a 32-bit ARGB color from red, green, blue (0-255). |
| `L_RGBA` | Composes a 32-bit ARGB color from red, green, blue, alpha (0-255). |
| **API declarations** | |
| `l_canvas_open` | Opens a canvas. Returns 0 on success, -1 on error (e.g. no display). |
| `l_canvas_close` | Closes the canvas and frees resources. |
| `l_canvas_alive` | Returns non-zero if the canvas is still alive (window not closed). |
| `l_canvas_flush` | Copies the pixel buffer to the screen. |
| `l_canvas_clear` | Fills the entire pixel buffer with a single color. |
| `l_canvas_key` | Returns the next key press (ASCII or arrow codes), or 0 if none. Non-blocking. |
| `l_canvas_mouse` | Returns mouse button bitmask (1=left, 2=right, 4=middle) and writes position to *x, *y. |
| **Drawing primitives (platform-independent, operate on pixels[])** | |
| `l_pixel` | Sets a single pixel at (x, y) to the given color. No-op if out of bounds. |
| `l_get_pixel` | Returns the color of the pixel at (x, y), or 0 if out of bounds. |
| `l_line` | Draws a line from (x0,y0) to (x1,y1) using Bresenham's algorithm. |
| `l_rect` | Draws an outline rectangle at (x,y) with width w and height h. |
| `l_fill_rect` | Draws a filled rectangle at (x,y) with width w and height h. |
| `l_circle` | Draws an outline circle centered at (cx,cy) with radius r (midpoint algorithm). |
| `l_fill_circle` | Draws a filled circle centered at (cx,cy) with radius r. |
| `l_hline` | Draws a horizontal line from x0 to x1 at row y (used internally by fill_circle). |
| **Text rendering** | |
| `l_draw_char` | Draws a single character at (x,y) using the embedded 8x8 bitmap font. |
| `l_draw_text` | Draws a null-terminated string at (x,y), advancing 8 pixels per character. |
| `l_draw_char_scaled` | Draws a single character at (x,y) scaled by (sx,sy) using nearest-neighbor. |
| `l_draw_text_scaled` | Draws a string at (x,y) with each glyph scaled by (sx,sy). |
| **Pixel blitting** | |
| `l_blit` | Blit a rectangle of ARGB pixels onto the canvas at (dx, dy). |
| `l_blit_alpha` | Blit with alpha blending (source-over). Assumes pre-multiplied alpha in the A channel. |

<!-- END GFX REFERENCE -->

Platform backends: **Linux** renders to `/dev/fb0` (framebuffer console — no X11 or Wayland). You may need to grant access first: `sudo chmod 666 /dev/fb0`. **Windows** opens a native GDI window (`user32.dll` + `gdi32.dll`). All graphical demos use **integer-only math** (no floats) for full ARM compatibility.

## Function Reference — `l_img.h`

Freestanding image decoding powered by vendored `stb_image.h`. Decodes PNG, JPEG, BMP, GIF, and TGA from in-memory buffers. Uses a 256 MB demand-paged bump allocator — no libc `malloc` needed.

| Function | Description |
|----------|-------------|
| `l_img_load_mem(data, len, w, h)` | Decodes an image from `data` (length `len` bytes). Sets `*w` and `*h` to pixel dimensions. Returns a `uint32_t*` ARGB pixel buffer, or `NULL` on failure. |
| `l_img_free_pixels(pixels, w, h)` | Releases memory used by a previously decoded image. |

**Usage notes:**
- Read your file into memory first (e.g. via `l_mmap` or `l_read`), then pass the buffer to `l_img_load_mem`.
- Pixels are 32-bit ARGB (compatible with `l_blit` and `l_blit_alpha` in `l_gfx.h`).
- The internal allocator reserves 256 MB virtual memory via `mmap`/`VirtualAlloc` (demand-paged — only touched pages use physical RAM).
- `#include "l_img.h"` automatically includes `stb_image.h` and `l_os.h`. On Linux, compile with `-Icompat` to provide freestanding shims for headers stb_image expects.

## Function Reference — `l_tls.h`

Freestanding TLS/HTTPS client. On **Windows**, uses SChannel (built-in OS TLS — zero external dependencies). On **Linux**, uses a vendored BearSSL amalgamation (zero external dependencies — constant-time AES, ECDHE+RSA, i31 bignum engine). On **WASI**, returns -1 (no sockets). Supports up to 8 simultaneous TLS connections.

| Function | Description |
|----------|-------------|
| `l_tls_init()` | Initialize the TLS subsystem. Call once before other `l_tls_*` functions. Returns 0 on success, -1 on failure. |
| `l_tls_connect(host, port)` | Connect to `host` (null-terminated hostname) on `port` over TLS. Returns a handle (0–7) on success, -1 on failure. DNS resolution uses `l_resolve`. |
| `l_tls_send(h, data, len)` | Send `len` bytes from `data` over TLS connection `h`. Returns bytes sent, -1 on failure. |
| `l_tls_recv(h, buf, len)` | Receive up to `len` bytes into `buf` from TLS connection `h`. Returns bytes read, 0 on connection close, -1 on error. |
| `l_tls_recv_byte(h)` | Receive a single byte. Returns byte value (0–255), or -1 on failure/close. |
| `l_tls_close(h)` | Close TLS connection `h`. Sends TLS shutdown notification. Safe with invalid handles. |
| `l_tls_cleanup()` | Shut down TLS subsystem. Closes all open connections. Call at program exit. |

**Platform availability:** Check `L_TLS_AVAILABLE` (1 on Windows and Linux, 0 on WASI). On Windows, compile with `-lsecur32 -lcrypt32 -lws2_32` (handled automatically by `build_parallel.ps1`). On Linux, BearSSL is compiled from the vendored `bearssl_amalg.c` amalgamation — no external libraries needed. Certificate verification is currently disabled (no-verify mode) — for production use, load CAs from `/etc/ssl/certs/ca-certificates.crt`.

## Function Reference — `l_ui.h`

Immediate-mode UI library built on `l_gfx.h`. No heap allocation, no widget tree — declare widgets every frame between `l_ui_begin`/`l_ui_end`. Widget functions return action state (e.g. `l_ui_button` returns 1 if clicked). Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

<!-- BEGIN UI REFERENCE -->

| Function | Description |
|----------|-------------|
| **Frame functions** | |
| `l_ui_begin` | Begins a UI frame. Call once per frame before declaring widgets. |
| `l_ui_end` | Ends a UI frame. Handles releasing active widget when mouse released. |
| `l_ui_init` | Initializes a UI context with the default dark theme and font scale 1. |
| **Widgets** | |
| `l_ui_label` | Draws a text label at (x,y). Returns 0 always. |
| `l_ui_button` | Draws a clickable button at (x,y) with given width and height. Returns 1 if clicked this frame. |
| `l_ui_checkbox` | Draws a checkbox at (x,y). *checked is toggled on click. Returns 1 if toggled this frame. |
| `l_ui_slider` | Draws a horizontal slider at (x,y) with given width. *value is clamped to [min_val, max_val]. Returns 1 if value changed. |
| `l_ui_textbox` | Draws a single-line text input at (x,y) with width w. buf is the text buffer, buf_len is max capacity. Returns 1 if text changed. |
| `l_ui_panel` | Draws a panel (filled rectangle with border) at (x,y). Returns 0. |
| `l_ui_separator` | Draws a horizontal separator line at (x,y) with width w. Returns 0. |
| **Auto-Layout Helpers** | |
| `l_ui_column_begin` | Begins a vertical (column) auto-layout at (x,y) with given spacing between widgets. |
| `l_ui_row_begin` | Begins a horizontal (row) auto-layout at (x,y) with given spacing. |
| `l_ui_next` | Advances auto-layout by `size` pixels. Returns the position before advancing (y for column, x for row). |
| `l_ui_layout_end` | Ends the current layout. |

<!-- END UI REFERENCE -->

## Platform Compatibility

Which `l_os.h` functions work on which platform. Generated from code annotations — run `.\gen-docs.ps1` to update.

- ✅ Implemented
- ⚠️ Stubbed — could be implemented (WASI preview 1 host call exists)
- ❌ Stubbed by design — platform fundamentally cannot support this
- — Not applicable (e.g., Unix-only function on Windows)

<!-- BEGIN COMPAT MATRIX -->

| Function | Linux | Windows | WASI |
|----------|-------|---------|------|
| **String functions** | | | |
| ``l_wcslen`` | ✅ | ✅ | ✅ |
| ``l_strlen`` | ✅ | ✅ | ✅ |
| ``l_strcpy`` | ✅ | ✅ | ✅ |
| ``l_strncpy`` | ✅ | ✅ | ✅ |
| ``l_strcat`` | ✅ | ✅ | ✅ |
| ``l_strncat`` | ✅ | ✅ | ✅ |
| ``l_strchr`` | ✅ | ✅ | ✅ |
| ``l_strrchr`` | ✅ | ✅ | ✅ |
| ``l_strstr`` | ✅ | ✅ | ✅ |
| ``l_strcmp`` | ✅ | ✅ | ✅ |
| ``l_strncmp`` | ✅ | ✅ | ✅ |
| ``l_strcasecmp`` | ✅ | ✅ | ✅ |
| ``l_strncasecmp`` | ✅ | ✅ | ✅ |
| ``l_strspn`` | ✅ | ✅ | ✅ |
| ``l_strcspn`` | ✅ | ✅ | ✅ |
| ``l_strpbrk`` | ✅ | ✅ | ✅ |
| ``l_strtok_r`` | ✅ | ✅ | ✅ |
| ``l_strsep`` | ✅ | ✅ | ✅ |
| ``l_bin2hex`` | ✅ | ✅ | ✅ |
| ``l_hex2bin`` | ✅ | ✅ | ✅ |
| ``l_basename`` | ✅ | ✅ | ✅ |
| ``l_dirname`` | ✅ | ✅ | ✅ |
| ``l_path_join`` | ✅ | ✅ | ✅ |
| ``l_path_ext`` | ✅ | ✅ | ✅ |
| ``l_path_exists`` | ✅ | ✅ | ✅ |
| ``l_path_isdir`` | ✅ | ✅ | ✅ |
| ``l_reverse`` | ✅ | ✅ | ✅ |
| **Conversion functions** | | | |
| ``l_isspace`` | ✅ | ✅ | ✅ |
| ``l_isdigit`` | ✅ | ✅ | ✅ |
| ``l_isalpha`` | ✅ | ✅ | ✅ |
| ``l_isalnum`` | ✅ | ✅ | ✅ |
| ``l_isupper`` | ✅ | ✅ | ✅ |
| ``l_islower`` | ✅ | ✅ | ✅ |
| ``l_toupper`` | ✅ | ✅ | ✅ |
| ``l_tolower`` | ✅ | ✅ | ✅ |
| ``l_isprint`` | ✅ | ✅ | ✅ |
| ``l_isxdigit`` | ✅ | ✅ | ✅ |
| ``l_abs`` | ✅ | ✅ | ✅ |
| ``l_labs`` | ✅ | ✅ | ✅ |
| ``l_llabs`` | ✅ | ✅ | ✅ |
| ``l_atol`` | ✅ | ✅ | ✅ |
| ``l_atoi`` | ✅ | ✅ | ✅ |
| ``l_strtoul`` | ✅ | ✅ | ✅ |
| ``l_strtol`` | ✅ | ✅ | ✅ |
| ``l_strtoull`` | ✅ | ✅ | ✅ |
| ``l_strtoll`` | ✅ | ✅ | ✅ |
| ``l_strtod`` | ✅ | ✅ | ✅ |
| ``l_atof`` | ✅ | ✅ | ✅ |
| ``l_strtof`` | ✅ | ✅ | ✅ |
| **Math functions** | | | |
| ``l_fabs`` | ✅ | ✅ | ✅ |
| ``l_floor`` | ✅ | ✅ | ✅ |
| ``l_ceil`` | ✅ | ✅ | ✅ |
| ``l_fmod`` | ✅ | ✅ | ✅ |
| ``l_sqrt`` | ✅ | ✅ | ✅ |
| ``l_sin`` | ✅ | ✅ | ✅ |
| ``l_cos`` | ✅ | ✅ | ✅ |
| ``l_exp`` | ✅ | ✅ | ✅ |
| ``l_log`` | ✅ | ✅ | ✅ |
| ``l_pow`` | ✅ | ✅ | ✅ |
| ``l_atan2`` | ✅ | ✅ | ✅ |
| ``l_tan`` | ✅ | ✅ | ✅ |
| ``l_asin`` | ✅ | ✅ | ✅ |
| ``l_acos`` | ✅ | ✅ | ✅ |
| ``l_atan`` | ✅ | ✅ | ✅ |
| ``l_log10`` | ✅ | ✅ | ✅ |
| ``l_log2`` | ✅ | ✅ | ✅ |
| ``l_round`` | ✅ | ✅ | ✅ |
| ``l_trunc`` | ✅ | ✅ | ✅ |
| ``l_hypot`` | ✅ | ✅ | ✅ |
| ``l_itoa`` | ✅ | ✅ | ✅ |
| **Memory functions** | | | |
| ``l_memmove`` | ✅ | ✅ | ✅ |
| ``l_memset`` | ✅ | ✅ | ✅ |
| ``l_memcmp`` | ✅ | ✅ | ✅ |
| ``l_memcpy`` | ✅ | ✅ | ✅ |
| ``l_memchr`` | ✅ | ✅ | ✅ |
| ``l_memrchr`` | ✅ | ✅ | ✅ |
| ``l_strnlen`` | ✅ | ✅ | ✅ |
| ``l_memmem`` | ✅ | ✅ | ✅ |
| **Random number generation (xorshift32, single-threaded)** | | | |
| ``l_srand`` | ✅ | ✅ | ✅ |
| ``l_rand`` | ✅ | ✅ | ✅ |
| ``l_rand_ctx_init`` | ✅ | ✅ | ✅ |
| ``l_srand_ctx`` | ✅ | ✅ | ✅ |
| ``l_rand_ctx`` | ✅ | ✅ | ✅ |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | | |
| ``l_vsnprintf`` | ✅ | ✅ | ✅ |
| ``l_snprintf`` | ✅ | ✅ | ✅ |
| ``l_dprintf`` | ✅ | ✅ | ✅ |
| ``l_printf`` | ✅ | ✅ | ✅ |
| ``l_vfprintf`` | ✅ | ✅ | ✅ |
| ``l_vprintf`` | ✅ | ✅ | ✅ |
| ``l_fprintf`` | ✅ | ✅ | ✅ |
| **System functions** | | | |
| ``l_exit`` | ✅ | ✅ | ✅ |
| ``l_open`` | ✅ | ✅ | ✅ |
| ``l_close`` | ✅ | ✅ | ✅ |
| ``l_read`` | ✅ | ✅ | ✅ |
| ``l_write`` | ✅ | ✅ | ✅ |
| ``l_read_line`` | ✅ | ✅ | ✅ |
| ``l_linebuf_init`` | ✅ | ✅ | ✅ |
| ``l_linebuf_read`` | ✅ | ✅ | ✅ |
| ``l_time`` | ✅ | ✅ | ✅ |
| ``l_puts`` | ✅ | ✅ | ✅ |
| ``l_exitif`` | ✅ | ✅ | ✅ |
| ``l_getenv`` | ✅ | ✅ | ✅ |
| ``l_getenv_init`` | ✅ | ✅ | ✅ |
| ``l_env_start`` | ✅ | ✅ | ✅ |
| ``l_env_next`` | ✅ | ✅ | ✅ |
| ``l_env_end`` | ✅ | ✅ | ✅ |
| ``l_find_executable`` | ✅ | ✅ | ✅ |
| **Option parsing (single-threaded; state in static variables)** | | | |
| ``l_getopt`` | ✅ | ✅ | ✅ |
| ``l_getopt_ctx_init`` | ✅ | ✅ | ✅ |
| ``l_getopt_ctx`` | ✅ | ✅ | ✅ |
| **Convenience file openers** | | | |
| ``l_open_read`` | ✅ | ✅ | ✅ |
| ``l_open_write`` | ✅ | ✅ | ✅ |
| ``l_open_readwrite`` | ✅ | ✅ | ✅ |
| ``l_open_append`` | ✅ | ✅ | ✅ |
| ``l_open_trunc`` | ✅ | ✅ | ✅ |
| **Error reporting** | | | |
| ``l_errno`` | ✅ | ✅ | ✅ |
| ``l_strerror`` | ✅ | ✅ | ✅ |
| **Terminal and timing functions (cross-platform)** | | | |
| ``l_sleep_ms`` | ✅ | ✅ | ✅ |
| ``l_term_raw`` | ✅ | ✅ | ✅ |
| ``l_term_restore`` | ✅ | ✅ | ✅ |
| ``l_read_nonblock`` | ✅ | ✅ | ✅ |
| ``l_term_size`` | ✅ | ✅ | ✅ |
| **ANSI terminal helpers** | | | |
| ``l_ansi_move`` | ✅ | ✅ | ✅ |
| ``l_ansi_color`` | ✅ | ✅ | ✅ |
| **File system functions (cross-platform)** | | | |
| ``l_unlink`` | ✅ | ✅ | ✅ |
| ``l_rmdir`` | ✅ | ✅ | ✅ |
| ``l_rename`` | ✅ | ✅ | ✅ |
| ``l_access`` | ✅ | ✅ | ✅ |
| ``l_chmod`` | ✅ | ✅ | ❌ |
| ``l_symlink`` | ✅ | ✅ | ✅ |
| ``l_readlink`` | ✅ | ✅ | ✅ |
| ``l_realpath`` | ✅ | ✅ | ✅ |
| ``l_stat`` | ✅ | ✅ | ✅ |
| ``l_fstat`` | ✅ | ✅ | ✅ |
| ``l_truncate`` | ✅ | ✅ | ✅ |
| ``l_ftruncate`` | ✅ | ✅ | ✅ |
| ``l_file_size`` | ✅ | ✅ | ✅ |
| ``l_read_all`` | ✅ | ✅ | ✅ |
| ``l_write_all`` | ✅ | ✅ | ✅ |
| ``l_opendir`` | ✅ | ✅ | ✅ |
| ``l_readdir`` | ✅ | ✅ | ✅ |
| ``l_closedir`` | ✅ | ✅ | ✅ |
| ``l_mmap`` | ✅ | ✅ | ✅ |
| ``l_munmap`` | ✅ | ✅ | ✅ |
| ``l_getrandom`` | ✅ | ✅ | ✅ |
| **Arena function declarations** | | | |
| ``l_arena_init`` | ✅ | ✅ | ✅ |
| ``l_arena_alloc`` | ✅ | ✅ | ✅ |
| ``l_arena_reset`` | ✅ | ✅ | ✅ |
| ``l_arena_free`` | ✅ | ✅ | ✅ |
| **Buffer function declarations** | | | |
| ``l_buf_init`` | ✅ | ✅ | ✅ |
| ``l_buf_push`` | ✅ | ✅ | ✅ |
| ``l_buf_printf`` | ✅ | ✅ | ✅ |
| ``l_buf_clear`` | ✅ | ✅ | ✅ |
| ``l_buf_free`` | ✅ | ✅ | ✅ |
| **L_Str â€” fat string (pointer + length) function declarations** | | | |
| ``l_str`` | ✅ | ✅ | ✅ |
| ``l_str_from`` | ✅ | ✅ | ✅ |
| ``l_str_null`` | ✅ | ✅ | ✅ |
| ``l_str_eq`` | ✅ | ✅ | ✅ |
| ``l_str_cmp`` | ✅ | ✅ | ✅ |
| ``l_str_startswith`` | ✅ | ✅ | ✅ |
| ``l_str_endswith`` | ✅ | ✅ | ✅ |
| ``l_str_contains`` | ✅ | ✅ | ✅ |
| ``l_str_sub`` | ✅ | ✅ | ✅ |
| ``l_str_trim`` | ✅ | ✅ | ✅ |
| ``l_str_ltrim`` | ✅ | ✅ | ✅ |
| ``l_str_rtrim`` | ✅ | ✅ | ✅ |
| ``l_str_chr`` | ✅ | ✅ | ✅ |
| ``l_str_rchr`` | ✅ | ✅ | ✅ |
| ``l_str_find`` | ✅ | ✅ | ✅ |
| ``l_str_dup`` | ✅ | ✅ | ✅ |
| ``l_str_cat`` | ✅ | ✅ | ✅ |
| ``l_str_cstr`` | ✅ | ✅ | ✅ |
| ``l_str_from_cstr`` | ✅ | ✅ | ✅ |
| ``l_str_split`` | ✅ | ✅ | ✅ |
| ``l_str_join`` | ✅ | ✅ | ✅ |
| ``l_str_upper`` | ✅ | ✅ | ✅ |
| ``l_str_lower`` | ✅ | ✅ | ✅ |
| ``l_str_replace`` | ✅ | ✅ | ✅ |
| ``l_buf_push_str`` | ✅ | ✅ | ✅ |
| ``l_buf_push_cstr`` | ✅ | ✅ | ✅ |
| ``l_buf_push_int`` | ✅ | ✅ | ✅ |
| ``l_buf_as_str`` | ✅ | ✅ | ✅ |
| **I/O multiplexing** | | | |
| ``l_poll`` | ✅ | ✅ | ✅ |
| **Signal handling** | | | |
| ``l_signal`` | ✅ | ✅ | ✅ |
| **Environment manipulation** | | | |
| ``l_setenv`` | ✅ | ✅ | ✅ |
| ``l_unsetenv`` | ✅ | ✅ | ✅ |
| **Scatter-gather I/O** | | | |
| ``l_writev`` | ✅ | ✅ | ✅ |
| ``l_readv`` | ✅ | ✅ | ✅ |
| **Terminal detection** | | | |
| ``l_isatty`` | ✅ | ✅ | ✅ |
| **Hash map (arena-backed, fixed capacity)** | | | |
| ``l_map_init`` | ✅ | ✅ | ✅ |
| ``l_map_get`` | ✅ | ✅ | ✅ |
| ``l_map_put`` | ✅ | ✅ | ✅ |
| ``l_map_del`` | ✅ | ✅ | ✅ |
| **Time conversion** | | | |
| ``l_gmtime`` | ✅ | ✅ | ✅ |
| ``l_localtime`` | ✅ | ✅ | ✅ |
| ``l_mktime`` | ✅ | ✅ | ✅ |
| ``l_strftime`` | ✅ | ✅ | ✅ |
| **Glob pattern matching** | | | |
| ``l_fnmatch`` | ✅ | ✅ | ✅ |
| ``l_glob`` | ✅ | ✅ | ✅ |
| **SHA-256** | | | |
| ``l_sha256_init`` | ✅ | ✅ | ✅ |
| ``l_sha256_update`` | ✅ | ✅ | ✅ |
| ``l_sha256_final`` | ✅ | ✅ | ✅ |
| ``l_sha256`` | ✅ | ✅ | ✅ |
| ``l_base64_encode`` | ✅ | ✅ | ✅ |
| ``l_base64_decode`` | ✅ | ✅ | ✅ |
| ``l_getcwd`` | ✅ | ✅ | ✅ |
| ``l_chdir`` | ✅ | ✅ | ❌ |
| ``l_pipe`` | ✅ | ✅ | ❌ |
| ``l_dup`` | ✅ | ✅ | ❌ |
| ``l_dup2`` | ✅ | ✅ | ❌ |
| ``l_getpid`` | ✅ | ✅ | ✅ |
| ``l_spawn_stdio`` | ✅ | ✅ | ❌ |
| ``l_spawn`` | ✅ | ✅ | ❌ |
| ``l_wait`` | ✅ | ✅ | ❌ |
| ``l_system`` | ✅ | ✅ | ✅ |
| **Unix and WASI functions** | | | |
| ``l_lseek`` | ✅ | — | ✅ |
| ``l_mkdir`` | ✅ | — | ✅ |
| ``l_sched_yield`` | ✅ | — | ✅ |
| ``l_fork`` | ✅ | — | ❌ |
| ``l_execve`` | ✅ | — | ❌ |
| ``l_waitpid`` | ✅ | — | ❌ |
| ``l_getppid`` | ✅ | — | ✅ |
| ``l_kill`` | ✅ | — | ❌ |
| **Byte order helpers** | | | |
| ``l_htons`` | ✅ | ✅ | ✅ |
| ``l_htonl`` | ✅ | ✅ | ✅ |
| ``l_inet_addr`` | ✅ | ✅ | ✅ |
| ``l_resolve`` | ✅ | ✅ | ✅ |
| **TCP socket functions** | | | |
| ``l_socket_tcp`` | ✅ | ✅ | ✅ |
| ``l_socket_connect`` | ✅ | ✅ | ✅ |
| ``l_socket_bind`` | ✅ | ✅ | ✅ |
| ``l_socket_listen`` | ✅ | ✅ | ✅ |
| ``l_socket_accept`` | ✅ | ✅ | ✅ |
| ``l_socket_send`` | ✅ | ✅ | ✅ |
| ``l_socket_recv`` | ✅ | ✅ | ✅ |
| ``l_socket_close`` | ✅ | ✅ | ✅ |
| **UDP socket functions** | | | |
| ``l_socket_udp`` | ✅ | ✅ | ✅ |
| ``l_socket_sendto`` | ✅ | ✅ | ✅ |
| ``l_socket_recvfrom`` | ✅ | ✅ | ✅ |
| **Generic address-based socket API (IPv4 and IPv6)** | | | |
| ``l_sockaddr_ipv4`` | ✅ | ✅ | ✅ |
| ``l_sockaddr_ipv6`` | ✅ | ✅ | ✅ |
| ``l_parse_ipv6`` | ✅ | ✅ | ✅ |
| ``l_format_ipv6`` | ✅ | ✅ | ✅ |
| ``l_socket_open`` | ✅ | ✅ | ✅ |
| ``l_socket_connect_addr`` | ✅ | ✅ | ✅ |
| ``l_socket_bind_addr`` | ✅ | ✅ | ✅ |
| ``l_socket_sendto_addr`` | ✅ | ✅ | ✅ |
| ``l_socket_recvfrom_addr`` | ✅ | ✅ | ✅ |
| ``l_socket_unix_connect`` | ✅ | ✅ | ✅ |

<!-- END COMPAT MATRIX -->

## Test Coverage

Which `l_os.h` functions are referenced in the test suite. Generated — run `.\gen-docs.ps1` to update.

<!-- BEGIN COVERAGE MATRIX -->

| Function | Tested | Test File |
|----------|--------|-----------|
| **String functions** | | |
| `l_wcslen` | ✅ | test_strings.c |
| `l_strlen` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
| `l_strcpy` | ✅ | test.c, test_strings.c |
| `l_strncpy` | ✅ | test_strings.c |
| `l_strcat` | ✅ | test.c, test_strings.c |
| `l_strncat` | ✅ | test_strings.c |
| `l_strchr` | ✅ | test_fs.c, test_strings.c |
| `l_strrchr` | ✅ | test_strings.c |
| `l_strstr` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_strcmp` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
| `l_strncmp` | ✅ | test_strings.c |
| `l_strcasecmp` | ✅ | test_strings.c |
| `l_strncasecmp` | ✅ | test_fs.c, test_strings.c |
| `l_strspn` | ✅ | test_strings.c |
| `l_strcspn` | ✅ | test_strings.c |
| `l_strpbrk` | ✅ | test_strings.c |
| `l_strtok_r` | ✅ | test_strings.c |
| `l_strsep` | ✅ | test_strings.c |
| `l_bin2hex` | ✅ | test_strings.c |
| `l_hex2bin` | ✅ | test_strings.c |
| `l_basename` | ✅ | test_strings.c |
| `l_dirname` | ✅ | test_strings.c |
| `l_path_join` | ✅ | test_fs.c |
| `l_path_ext` | ✅ | test_fs.c |
| `l_path_exists` | ✅ | test_fs.c |
| `l_path_isdir` | ✅ | test_fs.c |
| `l_reverse` | ✅ | test_strings.c |
| **Conversion functions** | | |
| `l_isspace` | ✅ | test_strings.c |
| `l_isdigit` | ✅ | test_strings.c |
| `l_isalpha` | ✅ | test_strings.c |
| `l_isalnum` | ✅ | test_strings.c |
| `l_isupper` | ✅ | test_strings.c |
| `l_islower` | ✅ | test_strings.c |
| `l_toupper` | ✅ | test_strings.c |
| `l_tolower` | ✅ | test_strings.c |
| `l_isprint` | ✅ | test_strings.c |
| `l_isxdigit` | ✅ | test_strings.c |
| `l_abs` | ✅ | test_strings.c |
| `l_labs` | ✅ | test_strings.c |
| `l_llabs` | ✅ | test.c |
| `l_atol` | ✅ | test_strings.c |
| `l_atoi` | ✅ | test.c, test_strings.c |
| `l_strtoul` | ✅ | test_strings.c |
| `l_strtol` | ✅ | test_strings.c |
| `l_strtoull` | ✅ | test_strings.c |
| `l_strtoll` | ✅ | test_strings.c |
| `l_strtod` | ✅ | test_strings.c |
| `l_atof` | ✅ | test_strings.c |
| `l_strtof` | ✅ | test_strings.c, test_utils.c |
| **Math functions** | | |
| `l_fabs` | ✅ | test_utils.c |
| `l_floor` | ✅ | test_utils.c |
| `l_ceil` | ✅ | test_utils.c |
| `l_fmod` | ✅ | test_utils.c |
| `l_sqrt` | ✅ | test_utils.c |
| `l_sin` | ✅ | test_utils.c |
| `l_cos` | ✅ | test_utils.c |
| `l_exp` | ✅ | test_utils.c |
| `l_log` | ✅ | test_utils.c |
| `l_pow` | ✅ | test_utils.c |
| `l_atan2` | ✅ | test_utils.c |
| `l_tan` | ✅ | test_utils.c |
| `l_asin` | ✅ | test_utils.c |
| `l_acos` | ✅ | test_utils.c |
| `l_atan` | ✅ | test_utils.c |
| `l_log10` | ✅ | test_utils.c |
| `l_log2` | ✅ | test_utils.c |
| `l_round` | ✅ | test_utils.c |
| `l_trunc` | ✅ | test_fs.c, test_utils.c |
| `l_hypot` | ✅ | test_utils.c |
| `l_itoa` | ✅ | test_strings.c |
| **Memory functions** | | |
| `l_memmove` | ✅ | test_strings.c |
| `l_memset` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
| `l_memcmp` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
| `l_memcpy` | ✅ | test_strings.c, test_utils.c |
| `l_memchr` | ✅ | test_strings.c |
| `l_memrchr` | ✅ | test_strings.c |
| `l_strnlen` | ✅ | test_strings.c |
| `l_memmem` | ✅ | test_strings.c |
| **Random number generation (xorshift32, single-threaded)** | | |
| `l_srand` | ✅ | test_strings.c, test_utils.c |
| `l_rand` | ✅ | test_strings.c, test_utils.c |
| `l_rand_ctx_init` | ✅ | test_utils.c |
| `l_srand_ctx` | ✅ | test_utils.c |
| `l_rand_ctx` | ✅ | test_utils.c |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | |
| `l_vsnprintf` | ✅ | test_strings.c |
| `l_snprintf` | ✅ | test_fs.c, test_strings.c |
| `l_dprintf` | ✅ | test.c |
| `l_printf` | ✅ | test_strings.c |
| `l_vfprintf` | ✅ | test_strings.c |
| `l_vprintf` | ✅ | test_strings.c |
| `l_fprintf` | ✅ | test_strings.c |
| **System functions** | | |
| `l_exit` | ✅ | test.c, test_fs.c |
| `l_open` | ✅ | test.c, test_fs.c |
| `l_close` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_read` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_write` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_read_line` | ✅ | test.c |
| `l_linebuf_init` | ✅ | test_strings.c |
| `l_linebuf_read` | ✅ | test_strings.c |
| `l_time` | ✅ | test_utils.c |
| `l_puts` | ✅ | test.c, test_fs.c |
| `l_exitif` | ✅ | test_fs.c |
| `l_getenv` | ✅ | gfx_test.c, test_fs.c, test_img.c, test_net.c, test_strings.c, test_tls.c, test_utils.c, test.c |
| `l_getenv_init` | ✅ | gfx_test.c, test_fs.c, test_img.c, test_net.c, test_strings.c, test_tls.c, test_utils.c, test.c |
| `l_env_start` | ✅ | test_fs.c |
| `l_env_next` | ✅ | test_fs.c |
| `l_env_end` | ✅ | test_fs.c |
| `l_find_executable` | ✅ | test.c |
| **Option parsing (single-threaded; state in static variables)** | | |
| `l_getopt` | ✅ | test.c, test_utils.c |
| `l_getopt_ctx_init` | ✅ | test_utils.c |
| `l_getopt_ctx` | ✅ | test_utils.c |
| **Convenience file openers** | | |
| `l_open_read` | ✅ | test.c, test_fs.c |
| `l_open_write` | ✅ | test.c, test_fs.c |
| `l_open_readwrite` | ✅ | test_fs.c |
| `l_open_append` | ✅ | test_fs.c |
| `l_open_trunc` | ✅ | test_fs.c |
| **Error reporting** | | |
| `l_errno` | ✅ | test.c |
| `l_strerror` | ✅ | test.c |
| **Terminal and timing functions (cross-platform)** | | |
| `l_sleep_ms` | ✅ | test_fs.c |
| `l_term_raw` | ✅ | test.c |
| `l_term_restore` | ✅ | test.c |
| `l_read_nonblock` | ✅ | test.c |
| `l_term_size` | ✅ | test.c |
| **ANSI terminal helpers** | | |
| `l_ansi_move` | ✅ | test_utils.c |
| `l_ansi_color` | ✅ | test_utils.c |
| **File system functions (cross-platform)** | | |
| `l_unlink` | ✅ | test.c, test_fs.c |
| `l_rmdir` | ✅ | test_fs.c |
| `l_rename` | ✅ | test_fs.c |
| `l_access` | ✅ | test.c, test_fs.c |
| `l_chmod` | ✅ | test_fs.c |
| `l_symlink` | ✅ | test_fs.c |
| `l_readlink` | ✅ | test_fs.c |
| `l_realpath` | ✅ | test_fs.c |
| `l_stat` | ✅ | test_fs.c |
| `l_fstat` | ✅ | test_fs.c |
| `l_truncate` | ✅ | test_fs.c |
| `l_ftruncate` | ✅ | test_fs.c |
| `l_file_size` | ✅ | test_fs.c |
| `l_read_all` | ✅ | test_fs.c |
| `l_write_all` | ✅ | test_fs.c |
| `l_opendir` | ✅ | test_fs.c |
| `l_readdir` | ✅ | test_fs.c |
| `l_closedir` | ✅ | test_fs.c |
| `l_mmap` | ✅ | test_fs.c, test_utils.c |
| `l_munmap` | ✅ | test_fs.c, test_utils.c |
| ``l_getrandom`` | — | |
| **Arena function declarations** | | |
| `l_arena_init` | ✅ | test_fs.c, test_utils.c |
| `l_arena_alloc` | ✅ | test_utils.c |
| `l_arena_reset` | ✅ | test_utils.c |
| `l_arena_free` | ✅ | test_fs.c, test_utils.c |
| **Buffer function declarations** | | |
| `l_buf_init` | ✅ | test_utils.c |
| `l_buf_push` | ✅ | test_utils.c |
| `l_buf_printf` | ✅ | test_utils.c |
| `l_buf_clear` | ✅ | test_utils.c |
| `l_buf_free` | ✅ | test_utils.c |
| **L_Str â€” fat string (pointer + length) function declarations** | | |
| `l_str` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
| `l_str_from` | ✅ | test_utils.c |
| `l_str_null` | ✅ | test_utils.c |
| `l_str_eq` | ✅ | test_utils.c |
| `l_str_cmp` | ✅ | test_utils.c |
| `l_str_startswith` | ✅ | test_utils.c |
| `l_str_endswith` | ✅ | test_utils.c |
| `l_str_contains` | ✅ | test_fs.c, test_utils.c |
| `l_str_sub` | ✅ | test_utils.c |
| `l_str_trim` | ✅ | test_utils.c |
| `l_str_ltrim` | ✅ | test_utils.c |
| `l_str_rtrim` | ✅ | test_utils.c |
| `l_str_chr` | ✅ | test_utils.c |
| `l_str_rchr` | ✅ | test_utils.c |
| `l_str_find` | ✅ | test_utils.c |
| `l_str_dup` | ✅ | test_utils.c |
| `l_str_cat` | ✅ | test_utils.c |
| `l_str_cstr` | ✅ | test_utils.c |
| `l_str_from_cstr` | ✅ | test_utils.c |
| `l_str_split` | ✅ | test_utils.c |
| `l_str_join` | ✅ | test_utils.c |
| `l_str_upper` | ✅ | test_utils.c |
| `l_str_lower` | ✅ | test_utils.c |
| `l_str_replace` | ✅ | test_utils.c |
| `l_buf_push_str` | ✅ | test_utils.c |
| `l_buf_push_cstr` | ✅ | test_utils.c |
| `l_buf_push_int` | ✅ | test_utils.c |
| `l_buf_as_str` | ✅ | test_utils.c |
| **I/O multiplexing** | | |
| `l_poll` | ✅ | test_fs.c, test_net.c |
| **Signal handling** | | |
| `l_signal` | ✅ | test.c |
| **Environment manipulation** | | |
| `l_setenv` | ✅ | test_fs.c, test_utils.c |
| `l_unsetenv` | ✅ | test_fs.c, test_utils.c |
| **Scatter-gather I/O** | | |
| `l_writev` | ✅ | test_fs.c |
| `l_readv` | ✅ | test_fs.c |
| **Terminal detection** | | |
| `l_isatty` | ✅ | test_fs.c |
| **Hash map (arena-backed, fixed capacity)** | | |
| `l_map_init` | ✅ | test_utils.c |
| `l_map_get` | ✅ | test_utils.c |
| `l_map_put` | ✅ | test_utils.c |
| `l_map_del` | ✅ | test_utils.c |
| **Time conversion** | | |
| `l_gmtime` | ✅ | test_utils.c |
| `l_localtime` | ✅ | test_utils.c |
| `l_mktime` | ✅ | test_utils.c |
| `l_strftime` | ✅ | test_utils.c |
| **Glob pattern matching** | | |
| `l_fnmatch` | ✅ | test_utils.c |
| `l_glob` | ✅ | test_fs.c |
| **SHA-256** | | |
| `l_sha256_init` | ✅ | test_utils.c |
| `l_sha256_update` | ✅ | test_utils.c |
| `l_sha256_final` | ✅ | test_utils.c |
| `l_sha256` | ✅ | test_utils.c |
| `l_base64_encode` | ✅ | test_utils.c |
| `l_base64_decode` | ✅ | test_utils.c |
| `l_getcwd` | ✅ | test_fs.c |
| `l_chdir` | ✅ | test_fs.c |
| `l_pipe` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_dup` | ✅ | test.c |
| `l_dup2` | ✅ | test.c |
| `l_getpid` | ✅ | test.c |
| `l_spawn_stdio` | ✅ | test.c |
| `l_spawn` | ✅ | test.c |
| `l_wait` | ✅ | test.c |
| `l_system` | ✅ | test.c |
| **Unix and WASI functions** | | |
| `l_lseek` | ✅ | test_fs.c |
| `l_mkdir` | ✅ | test_fs.c |
| `l_sched_yield` | ✅ | test_fs.c |
| `l_fork` | ✅ | test.c |
| ``l_execve`` | — | |
| `l_waitpid` | ✅ | test.c |
| `l_getppid` | ✅ | test.c |
| `l_kill` | ✅ | test.c |
| **Byte order helpers** | | |
| `l_htons` | ✅ | test_net.c |
| `l_htonl` | ✅ | test_net.c |
| `l_inet_addr` | ✅ | test_net.c |
| `l_resolve` | ✅ | test_net.c |
| **TCP socket functions** | | |
| `l_socket_tcp` | ✅ | test_net.c |
| `l_socket_connect` | ✅ | test_net.c |
| `l_socket_bind` | ✅ | test_net.c |
| `l_socket_listen` | ✅ | test_net.c |
| `l_socket_accept` | ✅ | test_net.c |
| `l_socket_send` | ✅ | test_net.c |
| `l_socket_recv` | ✅ | test_net.c |
| `l_socket_close` | ✅ | test_net.c |
| **UDP socket functions** | | |
| `l_socket_udp` | ✅ | test_net.c |
| `l_socket_sendto` | ✅ | test_net.c |
| `l_socket_recvfrom` | ✅ | test_net.c |
| **Generic address-based socket API (IPv4 and IPv6)** | | |
| `l_sockaddr_ipv4` | ✅ | test_net.c |
| `l_sockaddr_ipv6` | ✅ | test_net.c |
| `l_parse_ipv6` | ✅ | test_net.c |
| `l_format_ipv6` | ✅ | test_net.c |
| `l_socket_open` | ✅ | test_net.c |
| `l_socket_connect_addr` | ✅ | test_net.c |
| `l_socket_bind_addr` | ✅ | test_net.c |
| ``l_socket_sendto_addr`` | — | |
| ``l_socket_recvfrom_addr`` | — | |
| ``l_socket_unix_connect`` | — | |

**Coverage: 243 / 248 functions referenced in tests** (98%)

<!-- END COVERAGE MATRIX -->

## Example Programs

Every program in `examples/` compiles to a small, self-contained binary with no libc dependency.

### Utilities

| Program | Description | Source |
|---------|-------------|--------|
| **base64** | Base64 encoder/decoder (RFC 4648) | [base64.c](examples/base64.c) |
| **checksum** | SHA-256 file hash | [checksum.c](examples/checksum.c) |
| **config** | INI config file parser (L_Map + L_Str) | [config.c](examples/config.c) |
| **countlines** | Line counter | [countlines.c](examples/countlines.c) |
| **find** | Recursive file finder with glob matching | [find.c](examples/find.c) |
| **grep** | Substring or glob pattern filter (`-g` flag) | [grep.c](examples/grep.c) |
| **hexdump** | Hex + ASCII file dump | [hexdump.c](examples/hexdump.c) |
| **hello** | Hello world (README example) | [hello.c](examples/hello.c) |
| **http_get** | Simple HTTP GET over TCP sockets (IPv4 or hostname) | [http_get.c](examples/http_get.c) |
| **file_io** | File read/write (README example) | [file_io.c](examples/file_io.c) |
| **ls** | Directory listing (`-a`, `-l` with timestamps) | [ls.c](examples/ls.c) |
| **printenv** | Print environment variables | [printenv.c](examples/printenv.c) |
| **sort** | Line sort (`-r`, `-f`, `-n`, `-u`) | [sort.c](examples/sort.c) |
| **upper** | Uppercase filter | [upper.c](examples/upper.c) |
| **wc** | Line/word/byte counter | [wc.c](examples/wc.c) |

### Interactive Programs

| Program | Description | Source |
|---------|-------------|--------|
| **led** | Vim-style text editor (hjkl, insert/normal/command, :w/:q, search, SIGWINCH resize) | [led.c](examples/led.c) |
| **mandelbrot** | Fixed-point fractal renderer — pan, zoom, iteration control | [mandelbrot.c](examples/mandelbrot.c) |
| **sh** | Shell — builtins (cd, pwd, echo, export), L_Map lookup, PATH search, quotes, I/O redirection, pipes | [sh.c](examples/sh.c) |
| **snake** | Terminal Snake game with WASD controls | [snake.c](examples/snake.c) |

### Graphical Demos (`l_gfx.h`)

| Program | Description | Source |
|---------|-------------|--------|
| **gfx_demo** | Static drawing — rectangles, circles, text (README example) | [gfx_demo.c](examples/gfx_demo.c) |
| **bounce** | Bouncing ball animation — run with `-f` for fullscreen (README example) | [bounce.c](examples/bounce.c) |
| **life** | Conway's Game of Life — 80×60 grid, pause/randomize/clear | [life.c](examples/life.c) |
| **plasma** | Rainbow plasma — animated sine-wave color cycling | [plasma.c](examples/plasma.c) |
| **starfield** | 3D starfield — 200 stars with perspective projection | [starfield.c](examples/starfield.c) |
| **fire** | Doom-style fire — bottom-up heat propagation | [fire.c](examples/fire.c) |
| **clock** | Analog clock — hour/minute/second hands, ticking in real time | [clock.c](examples/clock.c) |
| **scaled_text** | Scaled text at 1×–6× plus stretch modes | [scaled_text.c](examples/scaled_text.c) |
| **blit_demo** | Opaque and alpha-blended sprite blitting | [blit_demo.c](examples/blit_demo.c) |
| **img_view** | Image viewer — load PNG/JPEG/BMP, aspect-ratio scaling | [img_view.c](examples/img_view.c) |
| **https_get** | HTTPS client — fetch a page over TLS (Windows + Linux) | [https_get.c](examples/https_get.c) |

### UI Demos (`l_ui.h`)

| Program | Description | Source |
|---------|-------------|--------|
| **ui_demo** | Full widget showcase — buttons, checkbox, slider, textbox, layout | [ui_demo.c](examples/ui_demo.c) |
| **ui_controls** | RGB color mixer — three sliders, hex input, live color preview | [ui_controls.c](examples/ui_controls.c) |

### Test Suite

| Program | Assertions | Source |
|---------|-----------|--------|
| **test** | Process/self-spawn regression shard | [test.c](tests/test.c) |
| **test_strings** | String, conversion, memory, and formatting shard | [test_strings.c](tests/test_strings.c) |
| **test_fs** | Filesystem, environment, and low-level I/O shard | [test_fs.c](tests/test_fs.c) |
| **test_utils** | Utility, data-structure, time, and math shard | [test_utils.c](tests/test_utils.c) |
| **test_img** | Image decoding tests (BMP, PNG, invalid data) | [test_img.c](tests/test_img.c) |
| **test_tls** | TLS init/cleanup, invalid handles, platform availability | [test_tls.c](tests/test_tls.c) |
| **test_net** | Manual socket/runtime shard (`l_poll` stays in default `test_fs`) | [test_net.c](tests/test_net.c) |
| **gfx_test** | 28 (in-memory pixel buffer tests) | [gfx_test.c](tests/gfx_test.c) |
| **ui_test** | UI widget logic tests (simulated canvas) | [ui_test.c](tests/ui_test.c) |

## Directory Structure

```
l_os.h          — Core runtime header (strings, I/O, processes, terminal)
l_gfx.h        — Pixel graphics header (drawing, font, canvas)
l_img.h        — Image decoding header (PNG, JPEG, BMP, GIF, TGA via stb_image)
l_tls.h        — TLS/HTTPS client header (SChannel on Windows, BearSSL on Linux)
l_ui.h         — Immediate-mode UI header (widgets, layout, theme)
stb_image.h    — Vendored image decoder (public domain, from nothings/stb)
bearssl_amalg.c — Vendored TLS library amalgamation (MIT, from BearSSL v0.6)
compat/         — Freestanding shims (string.h, stdlib.h) for stb_image on Linux
examples/       — Example programs and utilities (33 programs)
tests/          — Test suites (test.c, test_strings.c, test_fs.c, test_utils.c, test_img.c, test_net.c, gfx_test.c, ui_test.c)
tests/fixtures/ — Test data files (binaries, expected outputs)
tests/smoke/    — Smoke test scripts (showcase_smoke.sh, showcase_smoke.ps1)
bin/            — Compiled binaries (generated)
misc/           — Reference implementations (incl. multi-client echo_server via l_poll)
Taskfile        — Linux/macOS build automation (bash)
build.bat       — Windows build script
test_all.bat    — Windows build + default regression tests
runtime_checks.bat — Windows on-demand socket + showcase checks
ci.ps1          — Cross-platform CI (PowerShell)
```

## Building and Testing

### Linux/macOS
```sh
./Taskfile test              # build everything + run default regression tests
./Taskfile test_runtime      # run socket + showcase runtime checks on demand
./Taskfile build clang 2     # build with clang at -O2
```

### Windows
```bat
test_all.bat                 :: build everything + run default regression tests
runtime_checks.bat           :: run socket + showcase runtime checks on demand
```

### Full CI (all platforms from Windows via WSL)

```powershell
.\ci.ps1                                              # all targets, all compilers
.\ci.ps1 -Target windows -Action test                 # Windows only
.\ci.ps1 -Target linux -Compiler clang -OptLevel 2    # Linux clang -O2
.\ci.ps1 -Target arm -Action test                     # ARM32 + AArch64 via QEMU
.\ci.ps1 -Target riscv                                # RISC-V gcc + clang via QEMU
.\ci.ps1 -Target wasi                                 # WASI examples via wasmtime
```

`ci.ps1` builds, tests, and verifies across **10 configurations**: Windows, Linux gcc, Linux clang, ARM gcc, ARM clang, AArch64 gcc, AArch64 clang, RISC-V gcc, RISC-V clang, and WASI. Linux-family targets run in WSL from Windows; ARM/AArch64/RISC-V binaries execute via QEMU user-mode emulation, and WASI examples run under `wasmtime`.

**Requirements:** clang on PATH (Windows), WSL with `gcc`, `clang`, `arm-linux-gnueabihf-gcc`, `aarch64-linux-gnu-gcc`, `riscv64-linux-gnu-gcc`, `qemu-user`, plus `lld`, `libclang-rt-dev-wasm32`, `wasi-libc`, `wabt`, and `wasmtime` for the WASI target.

## Scope — Not Included (by Design)
- `printf`/`sprintf` — use `l_snprintf` or direct `l_write`
- `malloc`/`free` — no heap allocation
- Threads — no multithreading primitives

## License
MIT License
