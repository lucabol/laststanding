# API Reference

Auto-generated from `///` doc-comments in the header files. Run `.\gen-docs.ps1`
from the repository root to regenerate.

- [`l_os.h`](#function-reference--l_osh) — core runtime (strings, I/O, processes, terminal, data structures)
- [`l_gfx.h`](#function-reference--l_gfxh) — pixel graphics (drawing, fonts, canvas)
- [`l_svg.h`](#function-reference--l_svgh) — SVG rasterization
- [`l_ui.h`](#function-reference--l_uih) — immediate-mode UI

For platform support see [COMPAT.md](COMPAT.md). For test coverage see
[COVERAGE.md](COVERAGE.md).

---

## Function Reference — `l_os.h`

Platform column: **All** = Linux + Windows + WASI; **Unix** = Linux-only (Windows
has no `-` column equivalent — consult [COMPAT.md](COMPAT.md) for WASI stubs).

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
| `l_ansi_color_rgb` | Writes 24-bit truecolor ANSI sequence into buf; is_bg=0 for foreground, 1 for background | All |
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
| `l_arena_reset` | Reset used to 0. Memory is NOT freed — arena can be reused. | All |
| `l_arena_free` | Free the backing memory. Sets base=NULL. | All |
| **Buffer function declarations** | | |
| `l_buf_init` | Zero-initialize a buffer. | All |
| `l_buf_push` | Append n bytes. Returns 0 on success, -1 on failure. | All |
| `l_buf_printf` | Formatted append using l_vsnprintf. Returns bytes written or -1. | All |
| `l_buf_clear` | Set len=0 (keep allocated memory). | All |
| `l_buf_free` | Free backing memory and zero the struct. | All |
| **L_Str — fat string (pointer + length) function declarations** | | |
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
| `l_hmac_sha256` | One-shot HMAC-SHA256. Authenticates `datalen` bytes using `keylen`-byte key; writes 32-byte tag. | All |
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

---

## Function Reference — `l_gfx.h`

Platform backends:

- **Linux:** tries X11 first when `$DISPLAY` is set, falls back to `/dev/fb0`
  (framebuffer console), then falls back to the terminal when both stdin and
  stdout are TTYs.
- **Windows:** opens a native GDI window (`user32.dll` + `gdi32.dll`) by
  default. Set `L_GFX_TERM=1` to force the terminal backend when stdin and
  stdout are TTYs.
- **Terminal backend:** renders the pixel buffer as Unicode half-block
  characters (`▀`, U+2580) with 24-bit ANSI truecolor; each terminal cell
  represents 1×2 pixels.

For framebuffer access on Linux, you may need to grant access first:
`sudo chmod 666 /dev/fb0`. All graphical demos use **integer-only math** (no
floats) for full ARM compatibility.

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
| `l_canvas_wheel` | Returns and clears the accumulated vertical mouse-wheel delta (positive=up, negative=down, |
| `l_canvas_resized` | Returns 1 if the window was resized since the last call, 0 otherwise. Clears the flag. |
| `l_canvas_set_icon` | Sets the window / taskbar icon from an ARGB pixel array (0xAARRGGBB). |
| `l_clipboard_set` | Copy text to clipboard. Returns 0 on success, -1 on failure. |
| `l_clipboard_get` | Get text from clipboard. Returns bytes read (excluding NUL), 0 if empty, -1 on failure. |
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
| **Result of l_font_lookup: bitmap pointer (NULL if missing) and pixel advance.** | |
| `l_utf8_next` | Reads one UTF-8 codepoint starting at *p and advances *p past it. |
| `l_font_lookup_raw` | Looks up a codepoint in the font. Returns the bitmap pointer and pixel |
| `l_font_lookup` | Looks up a codepoint, falling back to fallback_cp once if missing. |
| `l_draw_glyph_f` | Draws one codepoint at (x,y) in the given font. Returns its pixel advance. |
| `l_draw_text_f` | Draws a UTF-8 string at (x,y) using font f. Returns the total pixel width drawn. |
| `l_draw_glyph_scaled_f` | Draws a single codepoint scaled by (sx,sy). Returns scaled advance. |
| `l_draw_text_scaled_f` | Draws a UTF-8 string scaled by (sx,sy). Returns total pixel width drawn. |
| `l_text_width_f` | Returns the pixel width that would be drawn for a UTF-8 string in font f. |
| **Pixel blitting** | |
| `l_blit` | Blit a rectangle of ARGB pixels onto the canvas at (dx, dy). |
| `l_blit_alpha` | Blit with alpha blending (source-over). Assumes pre-multiplied alpha in the A channel. |
| **Shared terminal flush (used by both Windows and Linux backend=2)** | |
| `l_term_flush_pixels` | Renders the pixel buffer as half-block characters with ANSI truecolor. |
| `l_term_canvas_init` | Opens a terminal canvas: enters raw mode, gets terminal size, allocates buffers. |
| `l_term_canvas_cleanup` | Closes a terminal canvas: shows cursor, resets colors, restores terminal. |

<!-- END GFX REFERENCE -->

---

## Function Reference — `l_svg.h`

Freestanding SVG rasterization powered by a vendored NanoSVG-derived fork.
Parses SVG from memory buffers and rasterizes to ARGB pixels. Subset renderer
optimized for icons, diagrams, and controlled vector assets. Uses a 256 MB
demand-paged bump allocator (same as `l_img.h`).

**Supported elements:** `<svg>`, `<g>` (groups), `<path>`, `<rect>`,
`<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`, `<defs>`,
`<linearGradient>` (basic), `<radialGradient>` (basic).

**Unsupported (intentional subset):** `<text>`, `<image>`, `<symbol>`,
`<use>`, `<clipPath>`, `<mask>`, `<filter>`, CSS stylesheets, advanced color
spaces (ICC profiles), complex gradients with pattern fills.

**Sizing behavior:**
- If `width` and `height` are both 0 in `L_SvgOptions`, uses the SVG's
  intrinsic dimensions from `viewBox` or `width`/`height` attributes. If none
  exist, returns NULL.
- If one dimension is 0 and the other is nonzero, scales preserving aspect
  ratio.
- If both dimensions are nonzero, rasterizes at that size (may distort).

<!-- BEGIN SVG REFERENCE -->

| Function | Description |
|----------|-------------|
| **-- Public API ---------------------------------------------------------------** | |
| `l_svg_load_mem` | Rasterize SVG from a memory buffer. Returns ARGB pixel data or NULL. |
| `l_svg_free_pixels` | Free pixel data returned by l_svg_load_mem(). w and h must match the decode. |

<!-- END SVG REFERENCE -->

**Type:** `L_SvgOptions`

```c
typedef struct {
    int width;        // requested raster width (0 = use intrinsic/viewBox)
    int height;       // requested raster height (0 = use intrinsic/viewBox)
    float dpi;        // default 96.0f
} L_SvgOptions;
```

**Usage notes:**
- Pixels are 32-bit ARGB, compatible with `l_blit` in `l_gfx.h`.
- Read SVG file into memory first, then pass the buffer to `l_svg_load_mem`.
- The internal allocator reserves 256 MB virtual memory (demand-paged).
- `#include "l_svg.h"` automatically pulls in `l_os.h`.
- Default DPI is 96; multiply by scale factor (e.g., 144 for 1.5× resolution) as needed.
- Transforms (`translate`, `scale`, `rotate`, `skewX`, `skewY`, `matrix`) are
  supported; nested transforms accumulate.
- CSS color names are supported; `currentColor` uses fallback rules; RGB/RGBA
  hex notation is standard.
- Opacity and `stroke`/`fill` attributes follow SVG spec (default fill:
  black, no stroke).
- SVG backgrounds are transparent. When displaying with `l_gfx.h`, use
  `l_blit_alpha` (not `l_blit`) and clear to `L_WHITE` so transparent areas
  render correctly. See `examples/svg_view.c`.

---

## Function Reference — `l_ui.h`

Immediate-mode UI library built on `l_gfx.h`. No heap allocation, no widget
tree — declare widgets every frame between `l_ui_begin`/`l_ui_end`. Widget
functions return action state (e.g. `l_ui_button` returns 1 if clicked).

<!-- BEGIN UI REFERENCE -->

| Function | Description |
|----------|-------------|
| **Frame functions** | |
| `l_ui_begin` | Begins a UI frame. Call once per frame before declaring widgets. |
| `l_ui_end` | Ends a UI frame. Handles releasing active widget when mouse released. |
| `l_ui_init` | Initializes a UI context with the default dark theme. font_scale is left |
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
