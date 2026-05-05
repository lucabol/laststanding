# Test Coverage

Which `l_os.h` functions are referenced by the test suite in `tests/`.
Generated — run `.\gen-docs.ps1` from the repository root to update.

<!-- BEGIN COVERAGE MATRIX -->

| Function | Tested | Test File |
|----------|--------|-----------|
| **String functions** | | |
| `l_wcslen` | ✅ | test_strings.c |
| `l_strlen` | ✅ | test.c, test_clipboard.c, test_fs.c, test_net.c, test_strings.c, test_term_gfx.c, test_utils.c |
| `l_strcpy` | ✅ | test.c, test_strings.c |
| `l_strncpy` | ✅ | test_strings.c |
| `l_strcat` | ✅ | test.c, test_strings.c |
| `l_strncat` | ✅ | test_strings.c |
| `l_strchr` | ✅ | test_fs.c, test_strings.c |
| `l_strrchr` | ✅ | test_strings.c |
| `l_strstr` | ✅ | test.c, test_fs.c, test_strings.c |
| `l_strcmp` | ✅ | test.c, test_fs.c, test_net.c, test_strings.c, test_term_gfx.c, test_utils.c |
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
| `l_memset` | ✅ | font_test.c, test.c, test_fs.c, test_net.c, test_strings.c, test_term_gfx.c, test_tt.c, test_utils.c |
| `l_memcmp` | ✅ | test.c, test_clipboard.c, test_fs.c, test_net.c, test_strings.c, test_utils.c |
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
| `l_getenv` | ✅ | gfx_test.c, test.c, test_clipboard.c, test_fs.c, test_img.c, test_net.c, test_strings.c, test_svg.c, test_term_gfx.c, test_tls.c, test_tls_live.c, test_utils.c |
| `l_getenv_init` | ✅ | gfx_test.c, test.c, test_clipboard.c, test_fs.c, test_img.c, test_net.c, test_strings.c, test_svg.c, test_term_gfx.c, test_tls.c, test_tls_live.c, test_utils.c |
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
| `l_ansi_color` | ✅ | test_term_gfx.c, test_utils.c |
| `l_ansi_color_rgb` | ✅ | test_term_gfx.c |
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
| `l_getrandom` | — | |
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
| `l_str` | ✅ | test.c, test_clipboard.c, test_fs.c, test_net.c, test_strings.c, test_term_gfx.c, test_utils.c |
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
| `l_execve` | — | |
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
| `l_socket_sendto_addr` | — | |
| `l_socket_recvfrom_addr` | — | |
| `l_socket_unix_connect` | — | |

**Coverage: 244 / 249 functions referenced in tests** (98%)

<!-- END COVERAGE MATRIX -->
