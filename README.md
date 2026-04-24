# laststanding

A freestanding C runtime — zero dependencies, direct syscalls, tiny binaries.
**Seven** header files give you everything from `strlen` to pixel graphics to
image decoding to SVG rasterization to TLS/HTTPS to interactive UI widgets,
across **Linux** (x86_64, ARM, AArch64, RISC-V), **Windows**, and **WASI
(WebAssembly)** — with no libc at all.

Binaries are statically linked, stripped, and typically **2–10 KB**. The
project ships a suite of Unix-style utilities, interactive programs,
graphics demos, an image viewer, an HTTPS client, and UI demos — all built
without a single line of libc.

---

## Table of contents

- [Why laststanding?](#why-laststanding)
- [Headers at a glance](#headers-at-a-glance)
- [Platform support](#platform-support)
- [Hello, world](#hello-world)
- [Quick-start examples](#quick-start-examples)
  - [Core runtime (`l_os.h`)](#core-runtime-l_osh)
  - [Pixel graphics (`l_gfx.h`)](#pixel-graphics-l_gfxh)
  - [Image decoding (`l_img.h`)](#image-decoding-l_imgh)
  - [SVG rasterization (`l_svg.h`)](#svg-rasterization-l_svgh)
  - [TrueType text (`l_tt.h`)](#truetype-text-l_tth)
  - [HTTPS client (`l_tls.h`)](#https-client-l_tlsh)
  - [Immediate-mode UI (`l_ui.h`)](#immediate-mode-ui-l_uih)
- [Compile-time flags](#compile-time-flags)
- [Key types](#key-types)
- [Binary sizes](#binary-sizes)
- [Example programs](#example-programs)
- [Project layout](#project-layout)
- [Building and testing](#building-and-testing)
- [Contributing](#contributing)
- [Scope — not included (by design)](#scope--not-included-by-design)
- [License](#license)

## Why laststanding?

- **No libc, no surprises.** Every syscall is a direct `syscall(2)` or Win32
  API call. No hidden allocations, no locale, no `errno` TLS shuffle, no
  thousand-line `printf` pulling in floats. You can read the entire runtime
  in an afternoon.
- **Tiny binaries.** A `hello` program is under 3 KB stripped. `grep`, a
  full vi-style editor, a fractal renderer, an HTTPS client — all under
  16 KB. See [Binary sizes](#binary-sizes).
- **One header per concern.** Drop `l_os.h` into any C project and get
  familiar APIs (`strlen`, `memcpy`, `puts`, `open`, `read`, …) backed by
  syscalls. Add more headers only when you need them.
- **Portable.** The same source compiles cleanly under `-Wall -Wextra
  -Wpedantic -ffreestanding -nostdlib` on 10 configurations: Linux
  x86_64/ARM/AArch64/RISC-V (gcc + clang), Windows (clang), and WASI
  (wasmtime).
- **Batteries included.** Pixel graphics, TrueType and SVG rendering, TLS
  over BearSSL/SChannel, an immediate-mode UI toolkit, arenas, hash maps,
  SHA-256, base64 — all in under 400 KB of headers.

## Headers at a glance

| Header    | What it provides |
|-----------|------------------|
| `l_os.h`  | String/memory, file I/O, processes, pipes, terminal, environment, sockets, arenas, hash maps, SHA-256, glob, time |
| `l_gfx.h` | Pixel graphics — drawing primitives, scaled bitmap font, pixel blitting, alpha blending, keyboard/mouse, clipboard, window/framebuffer/terminal backends |
| `l_img.h` | Image decoding — PNG/JPEG/BMP/GIF/TGA from memory via vendored stb_image (freestanding, no libc) |
| `l_svg.h` | SVG rasterization — paths, shapes, gradients. ARGB output compatible with `l_gfx.h`. No text/images/clipping/masks |
| `l_tt.h`  | TrueType/OpenType text — parses `.ttf`/`.otf` from memory via vendored stb_truetype. `l_tt_draw_text` does subpixel positioning + 2× supersampling |
| `l_tls.h` | TLS/HTTPS client — SChannel on Windows, BearSSL on Linux. Up to 8 simultaneous connections |
| `l_ui.h`  | Immediate-mode UI — buttons, checkboxes, sliders, text inputs, layout helpers (built on `l_gfx.h`) |

## Platform support

| Target  | Status | Notes |
|---------|--------|-------|
| Linux x86_64 (gcc + clang)     | ✅ first-class | everything |
| Linux ARM / AArch64 / RISC-V   | ✅ first-class | everything, tested under QEMU |
| Windows x86_64 (clang)         | ✅ first-class | GDI + SChannel |
| WASI (wasmtime)                | ⚠️ experimental | see note below |

> **WASI is experimental.** Core I/O (`l_read`, `l_write`, `l_open`,
> `l_close`, `l_lseek`) and environment access work. Process creation
> (`l_fork`, `l_spawn`), pipes, sockets, signals, and terminal control are
> stubbed and return `-1`. Memory mapping for anonymous allocations
> (`L_Arena`) works via `memory.grow`. Run with `wasmtime --dir . app.wasm`.
> Full matrix in [docs/COMPAT.md](docs/COMPAT.md).

## Hello, world

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    puts("Hello from laststanding!\n");  // note: l_puts does NOT append newline
    return 0;
}
```

```sh
# Linux
gcc -I. -Oz -ffreestanding -nostdlib -static \
    -Wall -Wextra -Wpedantic -o hello hello.c

# Windows
clang -I. -Oz -ffreestanding -nostdlib -lkernel32 \
      -Wall -Wextra -Wpedantic -o hello.exe hello.c
```

> ⚠️ **`l_puts` does not append a newline.** The standard C `puts` appends
> `\n`; the `puts` macro here is aliased to `l_puts`, which writes the
> string verbatim. Always include `\n` explicitly, or define
> `L_DONTOVERRIDE` and link against a different `puts`.

Define `L_MAINFILE` in **exactly one** translation unit to pull in the
function bodies and the platform-specific `_start` (Linux) or
`mainCRTStartup` (Windows) entry point.

## Quick-start examples

Compile every example with the same canonical command shown in
[Hello, world](#hello-world). For a full table of runnable programs see
[Example programs](#example-programs).

### Core runtime (`l_os.h`)

<details>
<summary>File I/O</summary>

```c
#define L_MAINFILE
#include "l_os.h"

int main(void) {
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
</details>

<details>
<summary>Spawn a child process</summary>

```c
#define L_MAINFILE
#include "l_os.h"

int main(void) {
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
</details>

<details>
<summary>Arena, growable buffer, fat strings</summary>

```c
L_Arena arena = l_arena_init(1024 * 1024);     // 1 MB arena
char *name    = l_arena_alloc(&arena, 256);    // bump-allocate
l_arena_reset(&arena);                         // reuse
l_arena_free(&arena);                          // release to OS

L_Buf buf;
l_buf_init(&buf);
l_buf_push(&buf, "hello", 5);
l_buf_printf(&buf, " world %d", 42);           // requires L_WITHSNPRINTF
l_buf_free(&buf);

// Fat string: pointer + length, 16 bytes, passed by value
L_Str s       = l_str("  hello world  ");
L_Str trimmed = l_str_trim(s);                 // zero-copy
L_Str upper   = l_str_upper(&arena, trimmed);  // arena-backed
```

`l_buf_printf` is **opt-in**: compile with `-DL_WITHSNPRINTF` to enable it
and the rest of the `l_*printf` family. Without this flag only `l_itoa`
and direct `l_write` are available, which keeps binaries tiny.
</details>

<details>
<summary>Hash map, poll, signals, SHA-256, env, glob</summary>

```c
// Hash map (arena-backed, FNV-1a, fixed capacity)
L_Map map = l_map_init(&arena, 64);
l_map_put(&map, "name", "Alice");
const char *v = l_map_get(&map, "name");

// Poll multiple fds
L_PollFd fds[2] = { { sock1, L_POLLIN, 0 }, { sock2, L_POLLIN, 0 } };
l_poll(fds, 2, 1000);

// Signals (Windows: SIGINT and SIGTERM only)
l_signal(L_SIGINT, my_handler);

// SHA-256 one-shot
unsigned char hash[32];
l_sha256("hello", 5, hash);

// Environment
l_setenv("MY_VAR", "hello");
const char *val = l_getenv("MY_VAR");

// Glob expansion (single level, no `**`)
L_Str *paths; int n;
l_glob("src/*.c", &paths, &n, &arena);
```
</details>

See [docs/API.md](docs/API.md) for the full function reference
(~250 functions, auto-generated).

### Pixel graphics (`l_gfx.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"              // pulls in l_os.h

int main(void) {
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

<details>
<summary>Animation loop — bouncing ball</summary>

```c
#define L_MAINFILE
#include "l_gfx.h"

int main(void) {
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
</details>

<details>
<summary>Extended fonts and UTF-8</summary>

`l_draw_text` uses the built-in 8×8 ASCII table (codepoints 32–126). For
UTF-8 text, proportional widths, or extra glyphs, use the `L_Font` API.
Extended glyph tables are **opt-in** via compile-time macros so programs
that don't use them pay zero bytes.

| Macro | Adds |
|-------|------|
| *(none)*                   | `l_font8x8_default` — fixed-width ASCII (always available) |
| `L_FONT_PROPORTIONAL`      | `l_font8x8_proportional` — per-glyph widths for ASCII |
| `L_FONT_LATIN1_SUPPLEMENT` | `l_font8x8_latin1` — U+00A0..U+00FF (accents, symbols) |
| `L_FONT_BOX_DRAWING`       | `l_font8x8_box` — box-drawing + arrows subset |

```c
#define L_MAINFILE
#define L_FONT_PROPORTIONAL
#define L_FONT_LATIN1_SUPPLEMENT
#include "l_gfx.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    L_Canvas c; l_canvas_open(&c, 480, 120, "Fonts");
    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, L_BLACK);
        l_draw_text_f(&c, &l_font8x8_latin1,       10, 10,
                      "Caf\xc3\xa9 r\xc3\xa9sum\xc3\xa9 \xc2\xa9 2025", L_WHITE);
        l_draw_text_f(&c, &l_font8x8_proportional, 10, 30,
                      "proportional widths: iii vs MMM", L_CYAN);
        l_draw_text_scaled_f(&c, &l_font8x8_proportional, 10, 50,
                             "scaled text", L_YELLOW, 3, 3);
        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }
    l_canvas_close(&c); return 0;
}
```

Key entry points: `l_utf8_next` (decoder), `l_font_lookup` (with fallback),
`l_draw_glyph_f` / `l_draw_text_f` / `l_draw_text_scaled_f`,
`l_text_width_f`. `l_ui.h` can also route its widgets through a custom
font — define `L_UI_WITH_CUSTOM_FONT` before including it and set
`ui.font = &l_font8x8_latin1`.

See [`examples/font_demo.c`](examples/font_demo.c) for a full visual tour.
</details>

### Image decoding (`l_img.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"
#include "l_img.h"              // pulls in stb_image (freestanding)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_FD fd    = l_open("photo.jpg", 0, 0);
    long long sz = l_lseek(fd, 0, 2); l_lseek(fd, 0, 0);
    unsigned char *buf = l_mmap(0, sz, L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);

    int w, h;
    uint32_t *pixels = l_img_load_mem(buf, (int)sz, &w, &h);  // ARGB
    l_munmap(buf, sz);
    if (!pixels) return 1;

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

> ⚠️ **Security.** `stb_image` performs only minimal validation on its
> inputs. Do **not** decode untrusted image files from the network or
> unsanitized uploads — a malicious image can trigger out-of-bounds reads
> or excessive allocations. For untrusted input, pre-validate with a
> hardened decoder or run the process inside a sandbox.

### SVG rasterization (`l_svg.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"
#include "l_svg.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    if (argc < 2) { l_puts("Usage: svg_view <file> [w] [h]\n"); return 0; }

    L_FD fd = l_open(argv[1], 0, 0);
    long long sz = l_lseek(fd, 0, 2); l_lseek(fd, 0, 0);
    unsigned char *buf = l_mmap(0, sz, L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);

    int w = argc > 2 ? l_atoi(argv[2]) : 512;
    int h = argc > 3 ? l_atoi(argv[3]) : 512;
    L_SvgOptions opt = { w, h, 96.0f };
    uint32_t *pixels = l_svg_load_mem(buf, (int)sz, &opt, &w, &h);
    l_munmap(buf, sz);
    if (!pixels) return 1;

    L_Canvas c; l_canvas_open(&c, w, h, argv[1]);
    l_blit(&c, 0, 0, w, h, pixels, w * 4);
    l_canvas_flush(&c);
    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27) l_sleep_ms(16);
    l_canvas_close(&c);
    l_svg_free_pixels(pixels, w, h);
    return 0;
}
```

Supported subset and sizing rules are documented in
[docs/API.md](docs/API.md#function-reference--l_svgh).

### TrueType text (`l_tt.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"
#include "l_tt.h"

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);
    L_FD fd = l_open("C:\\Windows\\Fonts\\segoeui.ttf", 0, 0);
    long long sz = l_lseek(fd, 0, 2); l_lseek(fd, 0, 0);
    unsigned char *font = l_mmap(0, sz, L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);

    stbtt_fontinfo info;
    l_tt_init_font(font, (int)sz, &info);

    L_Canvas c; l_canvas_open(&c, 640, 240, "Hello, text!");
    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27) {
        l_canvas_clear(&c, 0xFF101018);
        l_tt_draw_text(c.pixels, c.width, c.height, c.stride,
                       &info, 16.0f, 80.0f, 48.0f,
                       "The quick brown fox", 0xFFFFFFFF);
        l_canvas_flush(&c); l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
```

**Notes.**

- **DPI awareness.** On Windows, `l_canvas_open` marks the process
  Per-Monitor-V2 DPI-aware automatically (via `GetProcAddress`, so pre-1703
  builds still work).
- **Security.** `stb_truetype` does no range checking on font offsets. Do
  **not** pass untrusted font files — a malicious font can read arbitrary
  process memory.
- **Hinting.** No TrueType hinting. Very small body text (≤ 12 px) won't
  quite match native ClearType — prefer bold / semibold fonts at those
  sizes.

### HTTPS client (`l_tls.h`)

```c
#define L_MAINFILE
#include "l_tls.h"

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

On Linux, run `git submodule update --init` once after cloning to fetch
BearSSL. On Windows, `build_parallel.ps1` links `secur32`/`crypt32`/
`ws2_32` automatically. Certificate verification is currently in **no-verify
mode** — for production use, load CAs from
`/etc/ssl/certs/ca-certificates.crt`.

### Immediate-mode UI (`l_ui.h`)

```c
#define L_MAINFILE
#include "l_ui.h"               // pulls in l_gfx.h and l_os.h

int main(void) {
    L_Canvas canvas;
    if (l_canvas_open(&canvas, 400, 300, "My App") != 0) return 1;

    L_UI ui; l_ui_init(&ui);
    int clicked = 0, checked = 0, volume = 50;
    char name[256] = "";

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(30, 30, 34));
        if (ui.key == 27) break;

        l_ui_panel(&ui, 10, 10, 380, 280);
        l_ui_column_begin(&ui, 20, 20, 8);

        int y = l_ui_next(&ui, 10); l_ui_label(&ui, 20, y, "Settings");
        y = l_ui_next(&ui, 24); if (l_ui_button(&ui, 20, y, 120, 24, "Click Me")) clicked++;
        y = l_ui_next(&ui, 16); l_ui_checkbox(&ui, 20, y, "Dark mode", &checked);
        y = l_ui_next(&ui, 8);  l_ui_label(&ui, 20, y, "Volume:");
        y = l_ui_next(&ui, 16); l_ui_slider(&ui, 20, y, 200, &volume, 0, 100);
        y = l_ui_next(&ui, 8);  l_ui_label(&ui, 20, y, "Name:");
        y = l_ui_next(&ui, 20); l_ui_textbox(&ui, 20, y, 200, name, 256);

        l_ui_layout_end(&ui);
        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }
    l_canvas_close(&canvas);
    return 0;
}
```

For a full widget tour, see
[`examples/ui_demo.c`](examples/ui_demo.c). For the RGB color-mixer variant,
see [`examples/ui_controls.c`](examples/ui_controls.c).

## Compile-time flags

| Define                     | Purpose |
|----------------------------|---------|
| `L_MAINFILE`               | Activates function definitions **and** platform startup code. Exactly one translation unit must define this. |
| `L_DONTOVERRIDE`           | Prevents `#define strlen l_strlen` aliases — use when mixing with standard headers. |
| `L_WITHSNPRINTF`           | Enables `l_snprintf`/`l_vsnprintf` and the `l_*printf` family (opt-in, keeps binaries small). |
| `L_FONT_PROPORTIONAL`      | Adds `l_font8x8_proportional` (per-glyph ASCII widths) to `l_gfx.h`. |
| `L_FONT_LATIN1_SUPPLEMENT` | Adds `l_font8x8_latin1` — U+00A0..U+00FF glyph table (~1 KB). |
| `L_FONT_BOX_DRAWING`       | Adds `l_font8x8_box` — box-drawing and arrow subset. |
| `L_UI_WITH_CUSTOM_FONT`    | Adds a `const L_Font *font` field to `L_UI` so widgets route through the `_f` drawing path when set. |

By default, `l_os.h` aliases standard names (`strlen`, `memcpy`, `exit`,
`puts`, …) to their `l_` equivalents so you can write familiar C. Define
`L_DONTOVERRIDE` to disable the aliases.

In multi-file projects, only one `.c` file defines `L_MAINFILE`:

```c
// utils.c — gets type definitions and declarations only
#include "l_os.h"

// main.c — the one file with startup code and function bodies
#define L_MAINFILE
#include "l_os.h"
```

**Canonical compile commands:**

| Platform            | Command |
|---------------------|---------|
| Linux (gcc/clang)   | `gcc -I. -Oz -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o app app.c` |
| Windows (clang)     | `clang -I. -Oz -ffreestanding -nostdlib -lkernel32 -Wall -Wextra -Wpedantic -o app.exe app.c` |
| ARM32 cross         | `arm-linux-gnueabihf-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| AArch64 cross       | `aarch64-linux-gnu-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| RISC-V cross        | `riscv64-linux-gnu-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| WASI (wasm32)       | `clang --target=wasm32-wasi --sysroot=/path/to/wasi-sdk/share/wasi-sysroot -I. -O2 -ffreestanding -nostdlib -o app.wasm app.c` |

## Key types

| Type                                | Purpose |
|-------------------------------------|---------|
| `L_FD`                              | File descriptor (`ptrdiff_t`). On Windows, a library-managed slot — not a raw `HANDLE`. |
| `L_STDIN`, `L_STDOUT`, `L_STDERR`   | Standard descriptor constants (0, 1, 2). |
| `L_Str`                             | Fat string — pointer+length pair (16 bytes, by value). Zero-copy views or arena-backed. |
| `L_Arena`                           | Bump allocator. Init with `l_arena_init(size)`, allocate with `l_arena_alloc`. |
| `L_Buf`                             | Growable byte buffer (heap-backed via `realloc`). |
| `L_Canvas`                          | Pixel graphics context (X11 / framebuffer / terminal on Linux, GDI window or terminal on Windows). |
| `L_PollFd`                          | Poll descriptor — `fd`, `events`, `revents` for `l_poll`. |
| `L_IoVec`                           | I/O vector — base pointer + length for scatter-gather I/O. |
| `L_Map`                             | Arena-backed hash map — FNV-1a, open addressing, linear probing. |
| `L_Tm`                              | Broken-down time — year, month, day, hour, min, sec, weekday, yearday. |
| `L_Sha256`                          | SHA-256 hash context for incremental hashing. |
| `L_UI`                              | Immediate-mode UI context (theme, input state, layout cursor). |
| `L_UI_Theme`                        | UI color theme (background, hover, active, text, border, accent, input). |

## Binary sizes

Typical stripped sizes for a release build at `-Oz`. Measure your own with
`ls -l bin/` or `Get-Item bin\*.exe | Format-Table Name,Length` — the
numbers below are illustrative and will drift as the library grows.

| Example       | Linux x86_64 | Windows x64 | Notes |
|---------------|-------------:|------------:|-------|
| `hello`       |   ~2–3 KB    |   ~4–5 KB   | `puts` only |
| `wc`          |   ~4 KB      |   ~6 KB     | line/word/byte counter |
| `grep`        |   ~5 KB      |   ~7 KB     | substring + glob filter |
| `led` (editor)|   ~12 KB     |  ~14 KB     | vim-ish text editor |
| `mandelbrot`  |   ~8 KB      |  ~10 KB     | fixed-point fractal |
| `ui_demo`     |   ~14 KB     |  ~18 KB     | full widget showcase (`l_gfx.h` + `l_ui.h`) |
| `https_get`   |   ~80 KB*    |   ~10 KB    | *BearSSL linked statically on Linux; SChannel is an OS DLL on Windows |

Sockets and the `l_*printf` family are opt-in, so programs that don't use
them pay zero bytes. A program that needs only `l_write` and `l_strlen`
compiles down to roughly **1.5 KB** on Linux.

## Example programs

Every program in `examples/` compiles to a small, self-contained binary
with no libc dependency.

**To build a single example:**

```sh
# Linux / macOS
./Taskfile bin/hello            # builds bin/hello (default gcc -Oz)
./Taskfile bin/mandelbrot clang 2

# Windows
build.bat examples\hello.c      # emits hello.exe next to the source
```

Or just invoke the compiler directly using the commands in
[Compile-time flags](#compile-time-flags).

### Utilities

| Program | Description | Source |
|---------|-------------|--------|
| `base64`     | Base64 encoder/decoder (RFC 4648) | [base64.c](examples/base64.c) |
| `checksum`   | SHA-256 file hash | [checksum.c](examples/checksum.c) |
| `config`     | INI config file parser (`L_Map` + `L_Str`) | [config.c](examples/config.c) |
| `countlines` | Line counter | [countlines.c](examples/countlines.c) |
| `find`       | Recursive file finder with glob matching | [find.c](examples/find.c) |
| `grep`       | Substring or glob pattern filter (`-g` flag) | [grep.c](examples/grep.c) |
| `hexdump`    | Hex + ASCII file dump | [hexdump.c](examples/hexdump.c) |
| `hello`      | Hello world (README example) | [hello.c](examples/hello.c) |
| `http_get`   | Simple HTTP GET over TCP sockets | [http_get.c](examples/http_get.c) |
| `file_io`    | File read/write (README example) | [file_io.c](examples/file_io.c) |
| `ls`         | Directory listing (`-a`, `-l` with timestamps) | [ls.c](examples/ls.c) |
| `printenv`   | Print environment variables | [printenv.c](examples/printenv.c) |
| `sort`       | Line sort (`-r`, `-f`, `-n`, `-u`) | [sort.c](examples/sort.c) |
| `upper`      | Uppercase filter | [upper.c](examples/upper.c) |
| `wc`         | Line/word/byte counter | [wc.c](examples/wc.c) |

### Interactive programs

| Program | Description | Source |
|---------|-------------|--------|
| `led`        | Vim-style text editor (hjkl, insert/normal/command, `:w`/`:q`, search, SIGWINCH) | [led.c](examples/led.c) |
| `mandelbrot` | Fixed-point fractal — pan, zoom, iteration control | [mandelbrot.c](examples/mandelbrot.c) |
| `sh`         | Shell — builtins (`cd`, `pwd`, `echo`, `export`), PATH search, quotes, redirection, pipes | [sh.c](examples/sh.c) |
| `snake`      | Terminal Snake game with WASD controls | [snake.c](examples/snake.c) |

### Graphics + terminal demos (`l_gfx.h`)

| Program | Description | Source |
|---------|-------------|--------|
| `gfx_demo`    | Static drawing — rectangles, circles, text (README example) | [gfx_demo.c](examples/gfx_demo.c) |
| `term_demo`   | Terminal pixel graphics via ANSI truecolor half-block rendering (`L_GFX_TERM=1` on Windows) | [term_demo.c](examples/term_demo.c) |
| `bounce`      | Bouncing ball animation — `-f` for fullscreen | [bounce.c](examples/bounce.c) |
| `life`        | Conway's Game of Life — 80×60 grid, pause/randomize/clear | [life.c](examples/life.c) |
| `plasma`      | Rainbow plasma — animated sine-wave color cycling | [plasma.c](examples/plasma.c) |
| `starfield`   | 3D starfield — 200 stars with perspective projection | [starfield.c](examples/starfield.c) |
| `fire`        | Doom-style fire — bottom-up heat propagation | [fire.c](examples/fire.c) |
| `clock`       | Analog clock — hour/minute/second hands, ticking in real time | [clock.c](examples/clock.c) |
| `scaled_text` | Scaled text at 1×–6× plus stretch modes | [scaled_text.c](examples/scaled_text.c) |
| `font_demo`   | Extended fonts — proportional ASCII, Latin-1 accents, box drawing, UTF-8 | [font_demo.c](examples/font_demo.c) |
| `blit_demo`   | Opaque and alpha-blended sprite blitting | [blit_demo.c](examples/blit_demo.c) |
| `img_view`    | Image viewer — load PNG/JPEG/BMP, aspect-ratio scaling | [img_view.c](examples/img_view.c) |
| `svg_view`    | SVG viewer — load and rasterize | [svg_view.c](examples/svg_view.c) |
| `ttf_view`    | TrueType font viewer — renders a pangram at several sizes | [ttf_view.c](examples/ttf_view.c) |
| `https_get`   | HTTPS client — fetch a page over TLS | [https_get.c](examples/https_get.c) |

### UI demos (`l_ui.h`)

| Program | Description | Source |
|---------|-------------|--------|
| `ui_demo`     | Full widget showcase — buttons, checkbox, slider, textbox, layout | [ui_demo.c](examples/ui_demo.c) |
| `ui_controls` | RGB color mixer — three sliders, hex input, live color preview | [ui_controls.c](examples/ui_controls.c) |

### Test suite

See `tests/`. The shards are `test`, `test_strings`, `test_fs`,
`test_utils`, `test_img`, `test_tls`, `test_net`, `test_clipboard`,
`test_term_gfx`, `gfx_test`, and `ui_test`. Per-function coverage is
tracked in [docs/COVERAGE.md](docs/COVERAGE.md).

## Project layout

```
l_os.h              Core runtime (strings, I/O, processes, terminal, data structures)
l_gfx.h             Pixel graphics (drawing, font, window/framebuffer/terminal canvas)
l_img.h             Image decoding (PNG/JPEG/BMP/GIF/TGA via stb_image)
l_svg.h             SVG rasterization
l_tt.h              TrueType text rendering (via stb_truetype)
l_tls.h             TLS/HTTPS client (SChannel on Windows, BearSSL on Linux)
l_ui.h              Immediate-mode UI (widgets, layout, theme)
stb_image.h         Vendored image decoder (public domain, from nothings/stb)
stb_truetype.h      Vendored TrueType rasterizer (public domain, from nothings/stb)
bearssl/            BearSSL TLS library (git submodule, MIT)
compat/             Freestanding shims (string.h, stdlib.h) for stb_image on Linux
examples/           Example programs and utilities
tests/              Test suites (+ tests/fixtures/, tests/smoke/)
misc/               Reference implementations (incl. multi-client echo_server via l_poll)
bin/                Compiled binaries (generated, gitignored)
docs/               Generated API reference, compat matrix, coverage matrix
Taskfile            Linux/macOS build automation (bash)
build.bat           Windows single-file build
build_parallel.ps1  Windows parallel build
test_all.bat        Windows build + default regression tests
runtime_checks.bat  Windows on-demand socket + showcase checks
ci.ps1              Cross-platform CI (PowerShell, runs all 10 configurations)
gen-docs.ps1        Regenerates docs/API.md, docs/COMPAT.md, docs/COVERAGE.md
```

## Building and testing

### Linux / macOS

```sh
./Taskfile test              # build everything + run default regression tests
./Taskfile test_runtime      # run socket + showcase runtime checks on demand
./Taskfile build clang 2     # build with clang at -O2
./Taskfile test_arm          # ARM32 via QEMU
./Taskfile test_aarch64      # AArch64 via QEMU
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

`ci.ps1` builds, tests, and verifies across **10 configurations**: Windows,
Linux gcc, Linux clang, ARM gcc, ARM clang, AArch64 gcc, AArch64 clang,
RISC-V gcc, RISC-V clang, and WASI. Linux-family targets run in WSL from
Windows; ARM/AArch64/RISC-V binaries execute via QEMU user-mode emulation,
and WASI examples run under `wasmtime`.

**Requirements:** clang on PATH (Windows); WSL with `gcc`, `clang`,
`arm-linux-gnueabihf-gcc`, `aarch64-linux-gnu-gcc`,
`riscv64-linux-gnu-gcc`, `qemu-user`, plus `lld`, `libclang-rt-dev-wasm32`,
`wasi-libc`, `wabt`, and `wasmtime` for the WASI target.

## Contributing

1. Make your change.
2. Run `powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1` — the
   full cross-platform CI. Never commit without running it first; ARM and
   AArch64 targets in particular catch real bugs.
3. If you added or changed a doc-commented function, regenerate the docs:
   `powershell -NoProfile -ExecutionPolicy Bypass -File gen-docs.ps1`.
4. Commit and push. Compiler warnings are treated as errors — code must
   build cleanly with `-Wall -Wextra -Wpedantic` on all 10 configurations.

See `.github/copilot-instructions.md` for more detail on conventions
(syscall layering, `L_` naming, ARM32 constraints, header ordering).

## Scope — not included (by design)

- `printf` / `sprintf` as the default — available opt-in via
  `L_WITHSNPRINTF`.
- `malloc` / `free` — use `L_Arena` (bump) or `L_Buf` (heap-backed
  growable). No general-purpose heap allocator ships with the library.
- Threads — no multithreading primitives. `l_rand`, `l_getopt`, and a few
  other functions keep static state and are therefore single-threaded;
  reentrant variants (`l_rand_ctx`, `l_getopt_ctx`) are available.

## License

MIT — see [LICENSE](LICENSE).
