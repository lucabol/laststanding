# Platform Compatibility

Which `l_os.h` functions work on which platform. Generated from code
annotations — run `.\gen-docs.ps1` from the repository root to update.

Legend:

- ✅ Implemented
- ⚠️ Stubbed — could be implemented (WASI preview 1 host call exists)
- ❌ Stubbed by design — platform fundamentally cannot support this
- — Not applicable (e.g., Unix-only function on Windows)

<!-- BEGIN COMPAT MATRIX -->

| Function | Linux | Windows | WASI |
|----------|-------|---------|------|
| **String functions** | | | |
| `l_wcslen` | ✅ | ✅ | ✅ |
| `l_strlen` | ✅ | ✅ | ✅ |
| `l_strcpy` | ✅ | ✅ | ✅ |
| `l_strncpy` | ✅ | ✅ | ✅ |
| `l_strcat` | ✅ | ✅ | ✅ |
| `l_strncat` | ✅ | ✅ | ✅ |
| `l_stpcpy` | ✅ | ✅ | ✅ |
| `l_stpncpy` | ✅ | ✅ | ✅ |
| `l_memccpy` | ✅ | ✅ | ✅ |
| `l_strchr` | ✅ | ✅ | ✅ |
| `l_strrchr` | ✅ | ✅ | ✅ |
| `l_strstr` | ✅ | ✅ | ✅ |
| `l_strcmp` | ✅ | ✅ | ✅ |
| `l_strncmp` | ✅ | ✅ | ✅ |
| `l_strcasecmp` | ✅ | ✅ | ✅ |
| `l_strncasecmp` | ✅ | ✅ | ✅ |
| `l_strspn` | ✅ | ✅ | ✅ |
| `l_strcspn` | ✅ | ✅ | ✅ |
| `l_strpbrk` | ✅ | ✅ | ✅ |
| `l_strtok_r` | ✅ | ✅ | ✅ |
| `l_strsep` | ✅ | ✅ | ✅ |
| `l_bin2hex` | ✅ | ✅ | ✅ |
| `l_hex2bin` | ✅ | ✅ | ✅ |
| `l_basename` | ✅ | ✅ | ✅ |
| `l_dirname` | ✅ | ✅ | ✅ |
| `l_path_join` | ✅ | ✅ | ✅ |
| `l_path_ext` | ✅ | ✅ | ✅ |
| `l_path_exists` | ✅ | ✅ | ✅ |
| `l_path_isdir` | ✅ | ✅ | ✅ |
| `l_reverse` | ✅ | ✅ | ✅ |
| **Conversion functions** | | | |
| `l_isspace` | ✅ | ✅ | ✅ |
| `l_isdigit` | ✅ | ✅ | ✅ |
| `l_isalpha` | ✅ | ✅ | ✅ |
| `l_isalnum` | ✅ | ✅ | ✅ |
| `l_isupper` | ✅ | ✅ | ✅ |
| `l_islower` | ✅ | ✅ | ✅ |
| `l_toupper` | ✅ | ✅ | ✅ |
| `l_tolower` | ✅ | ✅ | ✅ |
| `l_isprint` | ✅ | ✅ | ✅ |
| `l_isxdigit` | ✅ | ✅ | ✅ |
| `l_abs` | ✅ | ✅ | ✅ |
| `l_labs` | ✅ | ✅ | ✅ |
| `l_llabs` | ✅ | ✅ | ✅ |
| `l_atol` | ✅ | ✅ | ✅ |
| `l_atoi` | ✅ | ✅ | ✅ |
| `l_strtoul` | ✅ | ✅ | ✅ |
| `l_strtol` | ✅ | ✅ | ✅ |
| `l_strtoull` | ✅ | ✅ | ✅ |
| `l_strtoll` | ✅ | ✅ | ✅ |
| `l_strtod` | ✅ | ✅ | ✅ |
| `l_atof` | ✅ | ✅ | ✅ |
| `l_strtof` | ✅ | ✅ | ✅ |
| **Math functions** | | | |
| `l_fabs` | ✅ | ✅ | ✅ |
| `l_floor` | ✅ | ✅ | ✅ |
| `l_ceil` | ✅ | ✅ | ✅ |
| `l_fmod` | ✅ | ✅ | ✅ |
| `l_sqrt` | ✅ | ✅ | ✅ |
| `l_sin` | ✅ | ✅ | ✅ |
| `l_cos` | ✅ | ✅ | ✅ |
| `l_exp` | ✅ | ✅ | ✅ |
| `l_log` | ✅ | ✅ | ✅ |
| `l_pow` | ✅ | ✅ | ✅ |
| `l_atan2` | ✅ | ✅ | ✅ |
| `l_tan` | ✅ | ✅ | ✅ |
| `l_asin` | ✅ | ✅ | ✅ |
| `l_acos` | ✅ | ✅ | ✅ |
| `l_atan` | ✅ | ✅ | ✅ |
| `l_log10` | ✅ | ✅ | ✅ |
| `l_log2` | ✅ | ✅ | ✅ |
| `l_round` | ✅ | ✅ | ✅ |
| `l_trunc` | ✅ | ✅ | ✅ |
| `l_hypot` | ✅ | ✅ | ✅ |
| `l_itoa` | ✅ | ✅ | ✅ |
| **Memory functions** | | | |
| `l_memmove` | ✅ | ✅ | ✅ |
| `l_memset` | ✅ | ✅ | ✅ |
| `l_memcmp` | ✅ | ✅ | ✅ |
| `l_memcpy` | ✅ | ✅ | ✅ |
| `l_memchr` | ✅ | ✅ | ✅ |
| `l_memrchr` | ✅ | ✅ | ✅ |
| `l_strnlen` | ✅ | ✅ | ✅ |
| `l_memmem` | ✅ | ✅ | ✅ |
| **Random number generation (xorshift32, single-threaded)** | | | |
| `l_srand` | ✅ | ✅ | ✅ |
| `l_rand` | ✅ | ✅ | ✅ |
| `l_rand_ctx_init` | ✅ | ✅ | ✅ |
| `l_srand_ctx` | ✅ | ✅ | ✅ |
| `l_rand_ctx` | ✅ | ✅ | ✅ |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | | |
| `l_vsnprintf` | ✅ | ✅ | ✅ |
| `l_snprintf` | ✅ | ✅ | ✅ |
| `l_dprintf` | ✅ | ✅ | ✅ |
| `l_printf` | ✅ | ✅ | ✅ |
| `l_vfprintf` | ✅ | ✅ | ✅ |
| `l_vprintf` | ✅ | ✅ | ✅ |
| `l_fprintf` | ✅ | ✅ | ✅ |
| **System functions** | | | |
| `l_exit` | ✅ | ✅ | ✅ |
| `l_open` | ✅ | ✅ | ✅ |
| `l_close` | ✅ | ✅ | ✅ |
| `l_read` | ✅ | ✅ | ✅ |
| `l_write` | ✅ | ✅ | ✅ |
| `l_read_line` | ✅ | ✅ | ✅ |
| `l_linebuf_init` | ✅ | ✅ | ✅ |
| `l_linebuf_read` | ✅ | ✅ | ✅ |
| `l_time` | ✅ | ✅ | ✅ |
| `l_puts` | ✅ | ✅ | ✅ |
| `l_exitif` | ✅ | ✅ | ✅ |
| `l_getenv` | ✅ | ✅ | ✅ |
| `l_getenv_init` | ✅ | ✅ | ✅ |
| `l_env_start` | ✅ | ✅ | ✅ |
| `l_env_next` | ✅ | ✅ | ✅ |
| `l_env_end` | ✅ | ✅ | ✅ |
| `l_find_executable` | ✅ | ✅ | ✅ |
| **Option parsing (single-threaded; state in static variables)** | | | |
| `l_getopt` | ✅ | ✅ | ✅ |
| `l_getopt_ctx_init` | ✅ | ✅ | ✅ |
| `l_getopt_ctx` | ✅ | ✅ | ✅ |
| **Convenience file openers** | | | |
| `l_open_read` | ✅ | ✅ | ✅ |
| `l_open_write` | ✅ | ✅ | ✅ |
| `l_open_readwrite` | ✅ | ✅ | ✅ |
| `l_open_append` | ✅ | ✅ | ✅ |
| `l_open_trunc` | ✅ | ✅ | ✅ |
| **Error reporting** | | | |
| `l_errno` | ✅ | ✅ | ✅ |
| `l_strerror` | ✅ | ✅ | ✅ |
| **Terminal and timing functions (cross-platform)** | | | |
| `l_sleep_ms` | ✅ | ✅ | ✅ |
| `l_term_raw` | ✅ | ✅ | ✅ |
| `l_term_restore` | ✅ | ✅ | ✅ |
| `l_read_nonblock` | ✅ | ✅ | ✅ |
| `l_term_size` | ✅ | ✅ | ✅ |
| **ANSI terminal helpers** | | | |
| `l_ansi_move` | ✅ | ✅ | ✅ |
| `l_ansi_color` | ✅ | ✅ | ✅ |
| `l_ansi_color_rgb` | ✅ | ✅ | ✅ |
| **File system functions (cross-platform)** | | | |
| `l_unlink` | ✅ | ✅ | ✅ |
| `l_rmdir` | ✅ | ✅ | ✅ |
| `l_rename` | ✅ | ✅ | ✅ |
| `l_access` | ✅ | ✅ | ✅ |
| `l_chmod` | ✅ | ✅ | ❌ |
| `l_symlink` | ✅ | ✅ | ✅ |
| `l_readlink` | ✅ | ✅ | ✅ |
| `l_realpath` | ✅ | ✅ | ✅ |
| `l_stat` | ✅ | ✅ | ✅ |
| `l_fstat` | ✅ | ✅ | ✅ |
| `l_truncate` | ✅ | ✅ | ✅ |
| `l_ftruncate` | ✅ | ✅ | ✅ |
| `l_file_size` | ✅ | ✅ | ✅ |
| `l_read_all` | ✅ | ✅ | ✅ |
| `l_write_all` | ✅ | ✅ | ✅ |
| `l_opendir` | ✅ | ✅ | ✅ |
| `l_readdir` | ✅ | ✅ | ✅ |
| `l_closedir` | ✅ | ✅ | ✅ |
| `l_mmap` | ✅ | ✅ | ✅ |
| `l_munmap` | ✅ | ✅ | ✅ |
| `l_getrandom` | ✅ | ✅ | ✅ |
| **Arena function declarations** | | | |
| `l_arena_init` | ✅ | ✅ | ✅ |
| `l_arena_alloc` | ✅ | ✅ | ✅ |
| `l_arena_reset` | ✅ | ✅ | ✅ |
| `l_arena_free` | ✅ | ✅ | ✅ |
| **Buffer function declarations** | | | |
| `l_buf_init` | ✅ | ✅ | ✅ |
| `l_buf_push` | ✅ | ✅ | ✅ |
| `l_buf_printf` | ✅ | ✅ | ✅ |
| `l_buf_clear` | ✅ | ✅ | ✅ |
| `l_buf_free` | ✅ | ✅ | ✅ |
| **L_Str — fat string (pointer + length) function declarations** | | | |
| `l_str` | ✅ | ✅ | ✅ |
| `l_str_from` | ✅ | ✅ | ✅ |
| `l_str_null` | ✅ | ✅ | ✅ |
| `l_str_eq` | ✅ | ✅ | ✅ |
| `l_str_cmp` | ✅ | ✅ | ✅ |
| `l_str_startswith` | ✅ | ✅ | ✅ |
| `l_str_endswith` | ✅ | ✅ | ✅ |
| `l_str_contains` | ✅ | ✅ | ✅ |
| `l_str_sub` | ✅ | ✅ | ✅ |
| `l_str_trim` | ✅ | ✅ | ✅ |
| `l_str_ltrim` | ✅ | ✅ | ✅ |
| `l_str_rtrim` | ✅ | ✅ | ✅ |
| `l_str_chr` | ✅ | ✅ | ✅ |
| `l_str_rchr` | ✅ | ✅ | ✅ |
| `l_str_find` | ✅ | ✅ | ✅ |
| `l_str_dup` | ✅ | ✅ | ✅ |
| `l_str_cat` | ✅ | ✅ | ✅ |
| `l_str_cstr` | ✅ | ✅ | ✅ |
| `l_str_from_cstr` | ✅ | ✅ | ✅ |
| `l_str_split` | ✅ | ✅ | ✅ |
| `l_str_join` | ✅ | ✅ | ✅ |
| `l_str_upper` | ✅ | ✅ | ✅ |
| `l_str_lower` | ✅ | ✅ | ✅ |
| `l_str_replace` | ✅ | ✅ | ✅ |
| `l_buf_push_str` | ✅ | ✅ | ✅ |
| `l_buf_push_cstr` | ✅ | ✅ | ✅ |
| `l_buf_push_int` | ✅ | ✅ | ✅ |
| `l_buf_as_str` | ✅ | ✅ | ✅ |
| **I/O multiplexing** | | | |
| `l_poll` | ✅ | ✅ | ✅ |
| **Signal handling** | | | |
| `l_signal` | ✅ | ✅ | ✅ |
| **Environment manipulation** | | | |
| `l_setenv` | ✅ | ✅ | ✅ |
| `l_unsetenv` | ✅ | ✅ | ✅ |
| **Scatter-gather I/O** | | | |
| `l_writev` | ✅ | ✅ | ✅ |
| `l_readv` | ✅ | ✅ | ✅ |
| **Terminal detection** | | | |
| `l_isatty` | ✅ | ✅ | ✅ |
| **Hash map (arena-backed, fixed capacity)** | | | |
| `l_map_init` | ✅ | ✅ | ✅ |
| `l_map_get` | ✅ | ✅ | ✅ |
| `l_map_put` | ✅ | ✅ | ✅ |
| `l_map_del` | ✅ | ✅ | ✅ |
| **Time conversion** | | | |
| `l_gmtime` | ✅ | ✅ | ✅ |
| `l_localtime` | ✅ | ✅ | ✅ |
| `l_mktime` | ✅ | ✅ | ✅ |
| `l_strftime` | ✅ | ✅ | ✅ |
| **Glob pattern matching** | | | |
| `l_fnmatch` | ✅ | ✅ | ✅ |
| `l_glob` | ✅ | ✅ | ✅ |
| **SHA-256** | | | |
| `l_sha256_init` | ✅ | ✅ | ✅ |
| `l_sha256_update` | ✅ | ✅ | ✅ |
| `l_sha256_final` | ✅ | ✅ | ✅ |
| `l_sha256` | ✅ | ✅ | ✅ |
| `l_base64_encode` | ✅ | ✅ | ✅ |
| `l_base64_decode` | ✅ | ✅ | ✅ |
| `l_getcwd` | ✅ | ✅ | ✅ |
| `l_chdir` | ✅ | ✅ | ❌ |
| `l_pipe` | ✅ | ✅ | ❌ |
| `l_dup` | ✅ | ✅ | ❌ |
| `l_dup2` | ✅ | ✅ | ❌ |
| `l_getpid` | ✅ | ✅ | ✅ |
| `l_spawn_stdio` | ✅ | ✅ | ❌ |
| `l_spawn` | ✅ | ✅ | ❌ |
| `l_wait` | ✅ | ✅ | ❌ |
| `l_system` | ✅ | ✅ | ✅ |
| **Unix and WASI functions** | | | |
| `l_lseek` | ✅ | — | ✅ |
| `l_mkdir` | ✅ | — | ✅ |
| `l_sched_yield` | ✅ | — | ✅ |
| `l_fork` | ✅ | — | ❌ |
| `l_execve` | ✅ | — | ❌ |
| `l_waitpid` | ✅ | — | ❌ |
| `l_getppid` | ✅ | — | ✅ |
| `l_kill` | ✅ | — | ❌ |
| **Byte order helpers** | | | |
| `l_htons` | ✅ | ✅ | ✅ |
| `l_htonl` | ✅ | ✅ | ✅ |
| `l_inet_addr` | ✅ | ✅ | ✅ |
| `l_resolve` | ✅ | ✅ | ✅ |
| **TCP socket functions** | | | |
| `l_socket_tcp` | ✅ | ✅ | ✅ |
| `l_socket_connect` | ✅ | ✅ | ✅ |
| `l_socket_bind` | ✅ | ✅ | ✅ |
| `l_socket_listen` | ✅ | ✅ | ✅ |
| `l_socket_accept` | ✅ | ✅ | ✅ |
| `l_socket_send` | ✅ | ✅ | ✅ |
| `l_socket_recv` | ✅ | ✅ | ✅ |
| `l_socket_close` | ✅ | ✅ | ✅ |
| **UDP socket functions** | | | |
| `l_socket_udp` | ✅ | ✅ | ✅ |
| `l_socket_sendto` | ✅ | ✅ | ✅ |
| `l_socket_recvfrom` | ✅ | ✅ | ✅ |
| **Generic address-based socket API (IPv4 and IPv6)** | | | |
| `l_sockaddr_ipv4` | ✅ | ✅ | ✅ |
| `l_sockaddr_ipv6` | ✅ | ✅ | ✅ |
| `l_parse_ipv6` | ✅ | ✅ | ✅ |
| `l_format_ipv6` | ✅ | ✅ | ✅ |
| `l_socket_open` | ✅ | ✅ | ✅ |
| `l_socket_connect_addr` | ✅ | ✅ | ✅ |
| `l_socket_bind_addr` | ✅ | ✅ | ✅ |
| `l_socket_sendto_addr` | ✅ | ✅ | ✅ |
| `l_socket_recvfrom_addr` | ✅ | ✅ | ✅ |
| `l_socket_unix_connect` | ✅ | ✅ | ✅ |

<!-- END COMPAT MATRIX -->
