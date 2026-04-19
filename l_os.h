// Includes for freestanding
#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// This need to be before stdnoreturn because it defines 'noreturn' differently
#ifdef _WIN32
#define VC_EXTRALEAN
// See http://utf8everywhere.org/ for the general idea of managing text as utf-8 on windows
#define UNICODE
#define _UNICODE
#ifdef L_WITHSOCKETS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <windows.h>
#include <shellapi.h>
#endif

#include <stdnoreturn.h>

// Portable type definitions (freestanding — no sys/types.h dependency)
#ifndef _WIN32
typedef long          ssize_t;
typedef unsigned int  mode_t;
#endif

#ifdef _WIN32
typedef long long     off_t;
#else
typedef long          off_t;
#endif

#ifdef _WIN32
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
typedef DWORD mode_t;
#endif

#if defined(_MSC_VER)
#pragma comment(linker, "/subsystem:console")
#pragma comment(compiler, "/GS-")
#pragma comment(lib, "kernel32.lib")
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Definitions and .start function must be defined in a single translation unit
#ifdef L_MAINFILE
#define L_WITHSTART
#define L_WITHDEFS
#endif

typedef   ptrdiff_t       L_FD;
typedef   long long       L_PID;  // pid_t on Linux, process HANDLE on Windows
#define   L_STDIN         0
#define   L_STDOUT        1
#define   L_STDERR        2
#define   L_SPAWN_INHERIT ((L_FD)-2)

// WASI Preview 1 types and function imports (shared by startup and implementation)
#ifdef __wasi__
#ifndef L_WASI_TYPES_DEFINED
#define L_WASI_TYPES_DEFINED

typedef uint16_t  __wasi_errno_t;
typedef uint32_t  __wasi_fd_t;
typedef uint64_t  __wasi_filesize_t;
typedef int64_t   __wasi_filedelta_t;
typedef uint8_t   __wasi_whence_t;
typedef uint64_t  __wasi_timestamp_t;
typedef uint32_t  __wasi_clockid_t;
typedef uint16_t  __wasi_oflags_t;
typedef uint64_t  __wasi_rights_t;
typedef uint16_t  __wasi_fdflags_t;
typedef uint32_t  __wasi_lookupflags_t;
typedef uint32_t  __wasi_exitcode_t;
typedef uint32_t  __wasi_size_t_w;
typedef uint8_t   __wasi_filetype_t;
typedef uint64_t  __wasi_device_t;
typedef uint64_t  __wasi_inode_t;
typedef uint64_t  __wasi_linkcount_t;
typedef uint64_t  __wasi_dircookie_t;
typedef uint32_t  __wasi_dirnamlen_t;

typedef struct { uint8_t *buf; uint32_t buf_len; } __wasi_iovec_t;
typedef struct { const uint8_t *buf; uint32_t buf_len; } __wasi_ciovec_t;

// WASI filestat — must match ABI layout exactly (64 bytes, 8-byte aligned)
typedef struct {
    __wasi_device_t dev;
    __wasi_inode_t ino;
    __wasi_filetype_t filetype;
    uint8_t __pad0[7];
    __wasi_linkcount_t nlink;
    __wasi_filesize_t size;
    __wasi_timestamp_t atim;
    __wasi_timestamp_t mtim;
    __wasi_timestamp_t ctim;
} __wasi_filestat_t;

// WASI dirent — must match ABI layout exactly (24 bytes)
typedef struct {
    __wasi_dircookie_t d_next;
    __wasi_inode_t d_ino;
    __wasi_dirnamlen_t d_namlen;
    __wasi_filetype_t d_type;
    uint8_t __pad0[3];
} __wasi_dirent_t;

#define __WASI_FILETYPE_UNKNOWN        0
#define __WASI_FILETYPE_DIRECTORY      3
#define __WASI_FILETYPE_REGULAR_FILE   4
#define __WASI_FILETYPE_SYMBOLIC_LINK  7

// Preopened directory fd (convention: fd 3 = cwd with --dir .)
#ifndef L_WASI_PREOPEN_FD
#define L_WASI_PREOPEN_FD 3
#endif

#define __WASI_WHENCE_SET 0
#define __WASI_WHENCE_CUR 1
#define __WASI_WHENCE_END 2

#define __WASI_OFLAGS_CREAT     ((__wasi_oflags_t)1)
#define __WASI_OFLAGS_DIRECTORY ((__wasi_oflags_t)2)
#define __WASI_OFLAGS_EXCL      ((__wasi_oflags_t)4)
#define __WASI_OFLAGS_TRUNC     ((__wasi_oflags_t)8)

#define __WASI_FDFLAGS_APPEND   ((__wasi_fdflags_t)1)
#define __WASI_FDFLAGS_DSYNC    ((__wasi_fdflags_t)2)
#define __WASI_FDFLAGS_NONBLOCK ((__wasi_fdflags_t)4)
#define __WASI_FDFLAGS_RSYNC    ((__wasi_fdflags_t)8)
#define __WASI_FDFLAGS_SYNC     ((__wasi_fdflags_t)16)

#define __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW 1

#define __WASI_CLOCKID_REALTIME  0
#define __WASI_CLOCKID_MONOTONIC 1

#define __WASI_RIGHTS_FD_DATASYNC    (1ULL << 0)
#define __WASI_RIGHTS_FD_READ        (1ULL << 1)
#define __WASI_RIGHTS_FD_SEEK        (1ULL << 2)
#define __WASI_RIGHTS_FD_FDSTAT_SET_FLAGS (1ULL << 3)
#define __WASI_RIGHTS_FD_SYNC        (1ULL << 4)
#define __WASI_RIGHTS_FD_TELL        (1ULL << 5)
#define __WASI_RIGHTS_FD_WRITE       (1ULL << 6)
#define __WASI_RIGHTS_FD_ADVISE      (1ULL << 7)
#define __WASI_RIGHTS_FD_ALLOCATE    (1ULL << 8)
#define __WASI_RIGHTS_PATH_CREATE_DIRECTORY (1ULL << 9)
#define __WASI_RIGHTS_PATH_CREATE_FILE  (1ULL << 10)
#define __WASI_RIGHTS_PATH_OPEN      (1ULL << 13)
#define __WASI_RIGHTS_FD_READDIR     (1ULL << 14)
#define __WASI_RIGHTS_PATH_FILESTAT_GET  (1ULL << 18)
#define __WASI_RIGHTS_FD_FILESTAT_GET (1ULL << 21)
#define __WASI_RIGHTS_FD_FILESTAT_SET_SIZE (1ULL << 22)
#define __WASI_RIGHTS_PATH_UNLINK_FILE (1ULL << 26)
#define __WASI_RIGHTS_PATH_REMOVE_DIRECTORY (1ULL << 25)

// WASI Preview 1 function imports
__wasi_errno_t __wasi_fd_read(__wasi_fd_t fd, const __wasi_iovec_t *iovs, size_t iovs_len, __wasi_size_t_w *nread)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_read")));
__wasi_errno_t __wasi_fd_write(__wasi_fd_t fd, const __wasi_ciovec_t *iovs, size_t iovs_len, __wasi_size_t_w *nwritten)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_write")));
__wasi_errno_t __wasi_fd_close(__wasi_fd_t fd)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_close")));
__wasi_errno_t __wasi_fd_seek(__wasi_fd_t fd, __wasi_filedelta_t offset, __wasi_whence_t whence, __wasi_filesize_t *newoffset)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_seek")));
__wasi_errno_t __wasi_path_open(__wasi_fd_t fd, __wasi_lookupflags_t dirflags,
    const char *path, size_t path_len, __wasi_oflags_t oflags,
    __wasi_rights_t fs_rights_base, __wasi_rights_t fs_rights_inheriting,
    __wasi_fdflags_t fdflags, __wasi_fd_t *opened_fd)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_open")));
_Noreturn void __wasi_proc_exit(__wasi_exitcode_t code)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("proc_exit")));
__wasi_errno_t __wasi_args_sizes_get(__wasi_size_t_w *argc, __wasi_size_t_w *argv_buf_size)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("args_sizes_get")));
__wasi_errno_t __wasi_args_get(uint8_t **argv, uint8_t *argv_buf)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("args_get")));
__wasi_errno_t __wasi_environ_sizes_get(__wasi_size_t_w *environ_count, __wasi_size_t_w *environ_buf_size)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("environ_sizes_get")));
__wasi_errno_t __wasi_environ_get(uint8_t **environ, uint8_t *environ_buf)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("environ_get")));
__wasi_errno_t __wasi_clock_time_get(__wasi_clockid_t id, __wasi_timestamp_t precision, __wasi_timestamp_t *time)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("clock_time_get")));
__wasi_errno_t __wasi_random_get(uint8_t *buf, __wasi_size_t_w buf_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("random_get")));
__wasi_errno_t __wasi_path_create_directory(__wasi_fd_t fd, const char *path, size_t path_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_create_directory")));
__wasi_errno_t __wasi_path_unlink_file(__wasi_fd_t fd, const char *path, size_t path_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_unlink_file")));
__wasi_errno_t __wasi_path_remove_directory(__wasi_fd_t fd, const char *path, size_t path_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_remove_directory")));
__wasi_errno_t __wasi_path_rename(__wasi_fd_t old_fd, const char *old_path, size_t old_path_len,
    __wasi_fd_t new_fd, const char *new_path, size_t new_path_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_rename")));
__wasi_errno_t __wasi_path_symlink(const char *old_path, size_t old_path_len,
    __wasi_fd_t fd, const char *new_path, size_t new_path_len)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_symlink")));
__wasi_errno_t __wasi_path_readlink(__wasi_fd_t fd, const char *path, size_t path_len,
    uint8_t *buf, __wasi_size_t_w buf_len, __wasi_size_t_w *bufused)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_readlink")));
__wasi_errno_t __wasi_path_filestat_get(__wasi_fd_t fd, __wasi_lookupflags_t flags,
    const char *path, size_t path_len, __wasi_filestat_t *filestat)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("path_filestat_get")));
__wasi_errno_t __wasi_fd_filestat_get(__wasi_fd_t fd, __wasi_filestat_t *filestat)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_filestat_get")));
__wasi_errno_t __wasi_fd_filestat_set_size(__wasi_fd_t fd, __wasi_filesize_t size)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_filestat_set_size")));
__wasi_errno_t __wasi_fd_readdir(__wasi_fd_t fd, uint8_t *buf, __wasi_size_t_w buf_len,
    __wasi_dircookie_t cookie, __wasi_size_t_w *bufused)
    __attribute__((__import_module__("wasi_snapshot_preview1"), __import_name__("fd_readdir")));

#endif // L_WASI_TYPES_DEFINED
#endif // __wasi__
#ifndef L_STAT_TYPES_DEFINED
#define L_STAT_TYPES_DEFINED
typedef struct {
    long long st_size;    // file size in bytes
    int       st_mode;    // file type and permissions
    long long st_mtime;   // modification time (Unix timestamp)
} L_Stat;

// Directory entry
typedef struct {
    char d_name[256];   // filename (null-terminated)
    int  d_type;        // L_DT_REG, L_DT_DIR, L_DT_UNKNOWN
} L_DirEntry;

// Directory handle (platform-specific)
#ifdef _WIN32
typedef struct {
    void *handle;         // HANDLE from FindFirstFileW
    L_DirEntry current;   // current entry
    int first;            // 1 = first call returns current, then FindNextFileW
    int done;             // 1 = no more entries
} L_Dir;
#elif defined(__wasi__)
typedef struct {
    L_FD fd;
    unsigned char buf[1024]; // fd_readdir buffer
    int  pos;           // current position in buffer
    int  len;           // bytes used in buffer
    unsigned long long cookie; // __wasi_dircookie_t for continuation
    int  done;          // 1 = no more entries
} L_Dir;
#else
typedef struct {
    L_FD fd;
    char buf[1024];     // getdents buffer
    int  pos;           // current position in buffer
    int  len;           // bytes read into buffer
} L_Dir;
#endif
#endif // L_STAT_TYPES_DEFINED

// Mode flag constants
#define L_S_IFDIR  0040000
#define L_S_IFREG  0100000
#define L_S_ISDIR(m)  (((m) & 0170000) == L_S_IFDIR)
#define L_S_IFLNK  0120000
#define L_S_ISREG(m)  (((m) & 0170000) == L_S_IFREG)
#define L_S_ISLNK(m)  (((m) & 0170000) == L_S_IFLNK)

#define L_DT_LNK     10
#define L_DT_UNKNOWN 0
#define L_DT_REG     8
#define L_DT_DIR     4

// Access mode flags for l_access
#define L_F_OK 0  // test existence
#define L_R_OK 4  // test read permission
#define L_W_OK 2  // test write permission
#define L_X_OK 1  // test execute permission

// Memory protection flags for l_mmap
#define L_PROT_READ   1
#define L_PROT_WRITE  2
#define L_PROT_EXEC   4

// Mapping flags for l_mmap
#define L_MAP_SHARED    1
#define L_MAP_PRIVATE   2
#define L_MAP_ANONYMOUS 0x20

#define L_MAP_FAILED ((void *)-1)

// Maximum path length
#define L_PATH_MAX 4096

// Common utility macros
#define L_MIN(a, b) ((a) < (b) ? (a) : (b))
#define L_MAX(a, b) ((a) > (b) ? (a) : (b))
#define L_CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : (v) > (hi) ? (hi) : (v))

// Cross-platform error codes (POSIX values on all platforms)
/// No such file or directory
#define L_ENOENT      2
/// Permission denied
#define L_EACCES     13
/// Bad file descriptor
#define L_EBADF       9
/// File exists
#define L_EEXIST     17
/// Invalid argument
#define L_EINVAL     22
/// Cannot allocate memory
#define L_ENOMEM     12
/// Resource temporarily unavailable
#define L_EAGAIN     11
/// Broken pipe
#define L_EPIPE      32
/// No space left on device
#define L_ENOSPC     28
/// Not a directory
#define L_ENOTDIR    20
/// Is a directory
#define L_EISDIR     21
/// Directory not empty
#define L_ENOTEMPTY  39

// Signal numbers (POSIX values; Windows does not support these)
#ifndef _WIN32
#define L_SIGHUP    1
#define L_SIGINT    2
#define L_SIGQUIT   3
#define L_SIGKILL   9
#define L_SIGPIPE  13
#define L_SIGTERM  15
#define L_SIGUSR1  10
#define L_SIGUSR2  12
#define L_SIGCHLD  17
#endif

// CLang warns for 'asm'
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

// Arena (bump allocator) — backed by l_mmap/l_munmap
#ifndef L_ARENA_TYPES_DEFINED
#define L_ARENA_TYPES_DEFINED
typedef struct {
    unsigned char *base;
    size_t used;
    size_t cap;
} L_Arena;

// Growable byte buffer — backed by l_mmap/l_munmap
typedef struct {
    unsigned char *data;
    size_t len;
    size_t cap;
} L_Buf;

// Fat string — pointer + length, no null terminator required
typedef struct {
    const char *data;
    size_t len;
} L_Str;
#endif

#ifndef L_NEWTYPES_DEFINED
#define L_NEWTYPES_DEFINED

// I/O multiplexing
typedef struct { L_FD fd; short events; short revents; } L_PollFd;
#define L_POLLIN  0x0001
#define L_POLLOUT 0x0004
#define L_POLLERR 0x0008
#define L_POLLHUP 0x0010

// Signal handling
typedef void (*L_SigHandler)(int);
#define L_SIG_DFL ((L_SigHandler)0)
#define L_SIG_IGN ((L_SigHandler)1)

// Address families and socket types
#define L_AF_UNIX   1
#define L_AF_INET   2
#define L_AF_INET6 10
#ifdef _WIN32
#undef  L_AF_INET6
#define L_AF_INET6 23
#endif
#define L_SOCK_STREAM 1
#define L_SOCK_DGRAM  2

// Generic socket address — library-owned, stable layout
// Holds either IPv4 or IPv6. Convert to OS structs internally.
typedef struct {
    int family;             // L_AF_INET or L_AF_INET6
    unsigned short port;    // host byte order
    unsigned char addr[16]; // IPv4 in first 4 bytes, IPv6 in all 16
} L_SockAddr;

#define L_INET6_ADDRSTRLEN 46

// Scatter-gather I/O
typedef struct { void *base; size_t len; } L_IoVec;

// Buffered line reader — wraps a file descriptor with a 4096-byte read buffer
#define L_LINEBUF_CAP 4096
typedef struct {
    L_FD fd;
    char buf[L_LINEBUF_CAP];
    int  pos;  /* next unread byte in buf */
    int  end;  /* one past last valid byte in buf */
    int  eof;  /* non-zero once fd returns EOF */
} L_LineBuf;

// Arena-backed hash map
typedef struct {
    const char *key;
    size_t keylen;
    void *value;
    unsigned int hash;
    int occupied;  // 0=empty, 1=occupied, 2=tombstone
} L_MapSlot;

typedef struct {
    L_Arena *arena;
    size_t cap;
    size_t len;
    L_MapSlot *slots;
} L_Map;

// Broken-down time
typedef struct {
    int sec;    // 0-59
    int min;    // 0-59
    int hour;   // 0-23
    int mday;   // 1-31
    int mon;    // 0-11
    int year;   // years since 1900
    int wday;   // 0-6 (Sunday=0)
    int yday;   // 0-365
} L_Tm;

// SHA-256
typedef struct {
    unsigned int state[8];
    unsigned long long count;
    unsigned char buf[64];
} L_Sha256;

// Context types for isolated subsystem state (Option B: explicit context variants)
// Use these when you need independent RNG streams or nested option parsing.
// The default global-state functions (l_rand, l_getopt) remain unchanged.

/// Independent random number generator context (xorshift32)
typedef struct {
    unsigned int state;
} L_RandCtx;

/// Independent option parser context (reentrant getopt)
typedef struct {
    char *arg;      // argument of the current option (like l_optarg)
    int   ind;      // index of the next argv element (starts at 1, like l_optind)
    int   opt;      // unknown option character when '?' is returned (like l_optopt)
    int   _pos;     // position within grouped short-option cluster
} L_GetoptCtx;

#endif // L_NEWTYPES_DEFINED

#ifdef L_WITHSOCKETS
#ifndef L_SOCKET_TYPES_DEFINED
#define L_SOCKET_TYPES_DEFINED
typedef ptrdiff_t L_SOCKET;

typedef struct {
    unsigned short sin_family;
    unsigned short sin_port;
    unsigned int   sin_addr;
    char           sin_zero[8];
} L_SockAddrIn;

typedef struct {
    unsigned short sin6_family;
    unsigned short sin6_port;
    unsigned int   sin6_flowinfo;
    unsigned char  sin6_addr[16];
    unsigned int   sin6_scope_id;
} L_SockAddrIn6;
#endif
#endif

// Math constants (always available, not guarded by L_WITHDEFS)
#ifndef L_PI
#define L_PI    3.14159265358979323846
#define L_PI_2  1.57079632679489661923
#define L_E     2.71828182845904523536
#define L_LN2   0.69314718055994530942
#define L_SQRT2 1.41421356237309504880
#endif

#ifdef L_WITHDEFS

// String functions
/// Returns the length of a wide character string
static inline size_t l_wcslen(const wchar_t *s);
/// Returns the length of a null-terminated string
static inline size_t l_strlen(const char *str);
/// Copies src string to dst, returns dst
static inline char *l_strcpy(char *dst, const char *src);
/// Copies up to n characters from src to dst, padding with nulls
static inline char *l_strncpy(char *dst, const char *src, size_t n);
/// Appends src string to dst, returns dst
static inline char *l_strcat(char *dst, const char *src);
/// Appends at most n characters of src to dst, always null-terminates, returns dst
static inline char *l_strncat(char *dst, const char *src, size_t n);
/// Returns pointer to first occurrence of c in s, or NULL
static inline char *l_strchr(const char *s, int c);
/// Returns pointer to last occurrence of c in s, or NULL
static inline char *l_strrchr(const char *s, int c);
/// Returns pointer to first occurrence of s2 in s1, or NULL
static inline char *l_strstr(const char *s1, const char *s2);
/// Compares two strings, returns <0, 0, or >0
static inline int l_strcmp(const char *s1, const char *s2);
/// Compares up to n characters of two strings
static inline int l_strncmp(const char *s1, const char *s2, size_t n);
/// Case-insensitive string comparison
static inline int l_strcasecmp(const char *s1, const char *s2);
/// Case-insensitive comparison of up to n characters
static inline int l_strncasecmp(const char *s1, const char *s2, size_t n);
/// Returns length of initial segment of s consisting entirely of bytes in accept
static inline size_t l_strspn(const char *s, const char *accept);
/// Returns length of initial segment of s consisting entirely of bytes NOT in reject
static inline size_t l_strcspn(const char *s, const char *reject);
/// Returns pointer to first occurrence in s of any character in accept, or NULL
static inline char *l_strpbrk(const char *s, const char *accept);
/// Splits str into tokens delimited by any char in delim; saves state in *saveptr (reentrant)
static inline char *l_strtok_r(char *str, const char *delim, char **saveptr);
/// Extracts token from *stringp delimited by any char in delim (BSD strsep); advances *stringp past delimiter
static inline char *l_strsep(char **stringp, const char *delim);
/// Converts binary data to lowercase hex string. NUL-terminates dst. Returns 2*len.
static inline int l_bin2hex(char *dst, const void *src, size_t len);
/// Converts hex string to binary data. Returns bytes written, or -1 on invalid input.
static inline int l_hex2bin(void *dst, const char *src, size_t len);
/// Returns pointer to the filename component of path (after last '/' or '\')
static inline const char *l_basename(const char *path);
/// Writes the directory component of path into buf (up to bufsize), returns buf
static inline char *l_dirname(const char *path, char *buf, size_t bufsize);
/// Joins directory and filename with separator, returns buf
static inline char *l_path_join(char *buf, size_t bufsz, const char *dir, const char *file);
/// Returns pointer to extension including dot (".txt"), or "" if none
static inline const char *l_path_ext(const char *path);
/// Returns 1 if path exists, 0 if not
static inline int l_path_exists(const char *path);
/// Returns 1 if path is a directory, 0 if not
static inline int l_path_isdir(const char *path);
/// Reverses a string in place
static inline void l_reverse(char str[], int length);

// Conversion functions
/// Returns non-zero if c is a whitespace character (space, tab, newline, etc.)
static inline int l_isspace(int c);
/// Returns non-zero if c is a digit ('0'-'9')
static inline int l_isdigit(int c);
/// Returns non-zero if c is an alphabetic character ('A'-'Z' or 'a'-'z')
static inline int l_isalpha(int c);
/// Returns non-zero if c is alphanumeric (l_isalpha or l_isdigit)
static inline int l_isalnum(int c);
/// Returns non-zero if c is an uppercase letter ('A'-'Z')
static inline int l_isupper(int c);
/// Returns non-zero if c is a lowercase letter ('a'-'z')
static inline int l_islower(int c);
/// Converts c to uppercase; returns c unchanged if not a lowercase letter
static inline int l_toupper(int c);
/// Converts c to lowercase; returns c unchanged if not an uppercase letter
static inline int l_tolower(int c);
/// Returns non-zero if c is a printable ASCII character (0x20-0x7e)
static inline int l_isprint(int c);
/// Returns non-zero if c is a hexadecimal digit (0-9, a-f, A-F)
static inline int l_isxdigit(int c);
/// Returns the absolute value of an integer
static inline int l_abs(int x);
/// Returns the absolute value of a long
static inline long l_labs(long x);
/// Returns the absolute value of a long long
static inline long long l_llabs(long long x);
/// Converts a string to a long integer, skipping leading whitespace
static inline long l_atol(const char *s);
/// Converts a string to an integer
static inline int l_atoi(const char *s);
/// Converts a string to an unsigned long, auto-detecting base when base==0 (0x=hex, 0=octal, else decimal); sets *endptr past last digit
static inline unsigned long l_strtoul(const char *nptr, char **endptr, int base);
/// Converts a string to a long, auto-detecting base when base==0; handles leading sign; sets *endptr past last digit
static inline long l_strtol(const char *nptr, char **endptr, int base);
/// Converts a string to an unsigned long long (64-bit); auto-detects base when base==0; sets *endptr past last digit
static inline unsigned long long l_strtoull(const char *nptr, char **endptr, int base);
/// Converts a string to a long long (64-bit); auto-detects base when base==0; handles leading sign; sets *endptr past last digit
static inline long long l_strtoll(const char *nptr, char **endptr, int base);
/// Converts a string to a double; skips leading whitespace; handles sign, decimal point, and e/E exponent; sets *endptr past last digit
static inline double l_strtod(const char *nptr, char **endptr);
/// Converts a string to a double (convenience wrapper around l_strtod)
static inline double l_atof(const char *s);
/// Converts a string to a float; skips leading whitespace; handles sign, decimal point, and e/E exponent; sets *endptr past last digit
static inline float l_strtof(const char *nptr, char **endptr);

// Math functions
/// Returns the absolute value of a double
static inline double l_fabs(double x);
/// Rounds toward negative infinity
static inline double l_floor(double x);
/// Rounds toward positive infinity
static inline double l_ceil(double x);
/// Floating-point remainder of x/y
static inline double l_fmod(double x, double y);
/// Square root via Newton-Raphson with IEEE bit hack seed
static inline double l_sqrt(double x);
/// Sine via range reduction and Taylor series
static inline double l_sin(double x);
/// Cosine: l_sin(x + pi/2)
static inline double l_cos(double x);
/// Exponential function via range reduction and Taylor series
static inline double l_exp(double x);
/// Natural logarithm via mantissa/exponent decomposition
static inline double l_log(double x);
/// Power: base^exp via exp(exp * log(base))
static inline double l_pow(double base, double exponent);
/// Two-argument arctangent with quadrant handling
static inline double l_atan2(double y, double x);
/// Tangent: sin(x)/cos(x)
static inline double l_tan(double x);
/// Inverse sine via Newton's method, valid for [-1,1]
static inline double l_asin(double x);
/// Inverse cosine: pi/2 - asin(x)
static inline double l_acos(double x);
/// Inverse tangent: asin(x/sqrt(1+x*x))
static inline double l_atan(double x);
/// Base-10 logarithm: log(x)/log(10)
static inline double l_log10(double x);
/// Base-2 logarithm: log(x)/log(2)
static inline double l_log2(double x);
/// Round to nearest integer (halfway rounds away from zero)
static inline double l_round(double x);
/// Truncate toward zero
static inline double l_trunc(double x);
/// Euclidean distance, overflow-safe: sqrt(x*x+y*y)
static inline double l_hypot(double x, double y);

/// Converts an integer to a string in the given radix (2-36)
static inline char *l_itoa(int in, char* buffer, int radix);

// Memory functions
/// Copies len bytes from src to dst, handling overlapping regions
static inline void *l_memmove(void *dst, const void *src, size_t len);
/// Fills len bytes of dst with byte value b
static inline void *l_memset(void *dst, int b, size_t len);
/// Compares n bytes of s1 and s2, returns <0, 0, or >0
static inline int l_memcmp(const void *s1, const void *s2, size_t n);
/// Copies len bytes from src to dst
static inline void *l_memcpy(void *dst, const void *src, size_t len);
/// Finds first occurrence of byte c in the first n bytes of s, or NULL
static inline void *l_memchr(const void *s, int c, size_t n);
/// Finds last occurrence of byte c in the first n bytes of s, or NULL
static inline void *l_memrchr(const void *s, int c, size_t n);
/// Returns the length of s, but at most maxlen (does not scan past maxlen bytes)
static inline size_t l_strnlen(const char *s, size_t maxlen);
/// Finds first occurrence of needle (needlelen bytes) in haystack (haystacklen bytes), or NULL
static inline void *l_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);

// Random number generation (xorshift32, single-threaded)
/// Seeds the pseudo-random number generator
static inline void l_srand(unsigned int seed);
/// Returns a pseudo-random unsigned int (xorshift32)
static inline unsigned int l_rand(void);
/// Initialize an independent RNG context
static inline void l_rand_ctx_init(L_RandCtx *ctx, unsigned int seed);
/// Seed an independent RNG context
static inline void l_srand_ctx(L_RandCtx *ctx, unsigned int seed);
/// Returns a pseudo-random unsigned int from an independent context
static inline unsigned int l_rand_ctx(L_RandCtx *ctx);

// Sorting and searching
/// Sorts an array in-place using Shell sort (no malloc, no recursion)
static inline void l_qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));
/// Binary search in a sorted array. Returns pointer to matching element, or NULL.
static inline void *l_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *));

// Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)
#ifdef L_WITHSNPRINTF
/// Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written
static int l_vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);
/// Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written
static int l_snprintf(char *buf, size_t n, const char *fmt, ...);
/// Writes formatted output to file descriptor fd. Returns number of bytes written.
static inline int l_dprintf(L_FD fd, const char *fmt, ...);
/// Writes formatted output to stdout. Returns number of bytes written.
static inline int l_printf(const char *fmt, ...);
/// Writes formatted output to file descriptor fd via va_list. Returns number of bytes written.
static inline int l_vfprintf(L_FD fd, const char *fmt, va_list ap);
/// Writes formatted output to stdout via va_list. Returns number of bytes written.
static inline int l_vprintf(const char *fmt, va_list ap);
/// Writes formatted output to file descriptor fd. Returns number of bytes written.
static inline int l_fprintf(L_FD fd, const char *fmt, ...);
#endif

// System functions
/// Terminates the process with the given status code
static inline noreturn void l_exit(int status);
/// Opens a file with the given flags and mode, returns file descriptor
static inline L_FD l_open(const char *path, int flags, mode_t mode);
/// Closes a file descriptor
static inline int l_close(L_FD fd);
/// Reads up to count bytes from fd into buf
static inline ssize_t l_read(L_FD fd, void *buf, size_t count);
/// Writes up to count bytes from buf to fd
static inline ssize_t l_write(L_FD fd, const void *buf, size_t count);
/// Reads one line from fd into buf (up to bufsz-1 bytes). Strips the newline.
/// Returns number of bytes read (excluding newline), or -1 on error/EOF with no data.
static inline ptrdiff_t l_read_line(L_FD fd, char *buf, size_t bufsz);
/// Initialise a buffered line reader wrapping fd.
static inline void l_linebuf_init(L_LineBuf *lb, L_FD fd);
/// Read one line into out (up to outsz-1 bytes). Strips newline. Buffers reads in 4096-byte chunks.
/// Returns number of bytes written (excluding '\0'), or -1 on EOF/error with no data.
static inline ptrdiff_t l_linebuf_read(L_LineBuf *lb, char *out, size_t outsz);
/// Returns current Unix timestamp (seconds since 1970-01-01). Also writes to *t if non-NULL.
static inline long long l_time(long long *t);
/// Writes a string to stdout
static inline void l_puts(const char* s);
/// Exits with code and message if condition is true
static inline void l_exitif(bool condition, int code, char *message);
/// Returns value of environment variable, or NULL if not found
static char *l_getenv(const char *name);
/// Initializes environment variable access (call from main)
static void l_getenv_init(int argc, char *argv[]);
/// Begin iterating environment variables. Returns opaque handle (pass to l_env_end).
static void *l_env_start(void);
/// Get next "KEY=VALUE" string. buf/bufsz provide conversion space (Windows).
/// Returns NULL when done. Caller must not free the returned pointer.
static const char *l_env_next(void **iter, char *buf, size_t bufsz);
/// End iteration and free resources.
static void l_env_end(void *handle);

/// Finds an executable by name, searching PATH if needed.
/// On Windows: tries .exe/.bat/.com extensions. Uses ';' PATH separator.
/// On Unix: uses ':' PATH separator, no extension magic.
/// Returns 1 if found (writes full path to out), 0 if not found.
static int l_find_executable(const char *cmd, char *out, size_t outsz);

// Option parsing (single-threaded; state in static variables)
/// Parses command-line options. optstring lists valid option chars; trailing ':' means the option
/// takes an argument. Returns the option char on match, '?' for unknown options, -1 when done.
/// Sets l_optarg to the argument string (or NULL), l_optind to the next argv index.
static inline int l_getopt(int argc, char *const argv[], const char *optstring);
/// Initialize an independent option parser context
static inline void l_getopt_ctx_init(L_GetoptCtx *ctx);
/// Reentrant getopt using an independent context. Same semantics as l_getopt.
static inline int l_getopt_ctx(L_GetoptCtx *ctx, int argc, char *const argv[], const char *optstring);

// Convenience file openers
/// Opens a file for reading
static inline L_FD l_open_read(const char* file);
/// Opens or creates a file for writing
static inline L_FD l_open_write(const char* file);
/// Opens or creates a file for reading and writing
static inline L_FD l_open_readwrite(const char* file);
/// Opens or creates a file for appending
static inline L_FD l_open_append(const char* file);
/// Opens or creates a file, truncating to zero length
static inline L_FD l_open_trunc(const char* file);

// Error reporting
/// Returns the error code from the most recent failed syscall (0 if last call succeeded)
static inline int l_errno(void);
/// Returns a human-readable string for the given error code
static inline const char *l_strerror(int errnum);

// Terminal and timing functions (cross-platform)
/// Sleeps for the given number of milliseconds
static inline void l_sleep_ms(unsigned int ms);
/// Sets stdin to raw mode (no echo, no line buffering), returns old mode
static unsigned long l_term_raw(void);
/// Restores terminal mode from value returned by l_term_raw
static void l_term_restore(unsigned long old_mode);
/// Reads from fd without blocking, returns 0 if no data available
static inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count);
/// Gets terminal size in rows and columns
static inline void l_term_size(int *rows, int *cols);

// ANSI terminal helpers
#define L_ANSI_CLEAR    "\033[2J"
#define L_ANSI_HOME     "\033[H"
#define L_ANSI_SHOW_CUR "\033[?25h"
#define L_ANSI_HIDE_CUR "\033[?25l"
#define L_ANSI_RESET    "\033[0m"
/// Writes cursor-move sequence into buf, returns bytes written
static inline int l_ansi_move(char *buf, size_t bufsz, int row, int col);
/// Writes color sequence into buf; fg/bg are 0-7 ANSI colors, -1 for default
static inline int l_ansi_color(char *buf, size_t bufsz, int fg, int bg);
/// Writes 24-bit truecolor ANSI sequence into buf; is_bg=0 for foreground, 1 for background
static inline int l_ansi_color_rgb(char *buf, size_t bufsz, int r, int g, int b, int is_bg);

// File system functions (cross-platform)
/// Deletes a file, returns 0 on success, -1 on error
static inline int l_unlink(const char *path);
/// Removes an empty directory, returns 0 on success, -1 on error
static inline int l_rmdir(const char *path);
/// Renames (or moves) a file or directory. Returns 0 on success, -1 on error.
static inline int l_rename(const char *oldpath, const char *newpath);
/// Checks access to a file. mode: L_F_OK (exists), L_R_OK, L_W_OK, L_X_OK. Returns 0 if ok, -1 on error.
static inline int l_access(const char *path, int mode);
/// Changes permission bits of a file. Returns 0 on success, -1 on error.
static inline int l_chmod(const char *path, mode_t mode);
/// Creates a symbolic link at linkpath pointing to target. Returns 0 on success, -1 on error.
static inline int l_symlink(const char *target, const char *linkpath);
/// Reads the target of a symbolic link into buf (up to bufsiz bytes). Returns number of bytes read, or -1 on error.
static inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz);
/// Resolves path to its canonical absolute form into resolved (at least L_PATH_MAX bytes). Returns resolved on success, NULL on error.
static inline char *l_realpath(const char *path, char *resolved);
/// Gets file metadata by path. Returns 0 on success, -1 on error.
static inline int l_stat(const char *path, L_Stat *st);
/// Gets file metadata by open file descriptor. Returns 0 on success, -1 on error.
static inline int l_fstat(L_FD fd, L_Stat *st);
/// Truncates a file at the given path to the specified size. Returns 0 on success, -1 on error.
static inline int l_truncate(const char *path, long long size);
/// Truncates an open file descriptor to the specified size. Returns 0 on success, -1 on error.
static inline int l_ftruncate(L_FD fd, long long size);
/// Returns the size of a file in bytes, or -1 on error.
static inline long long l_file_size(const char *path);
/// Reads exactly count bytes, retrying on short reads. Returns total bytes read, 0 on EOF, or negative on error.
static inline ptrdiff_t l_read_all(L_FD fd, void *buf, size_t count);
/// Writes exactly count bytes, retrying on short writes. Returns total bytes written, or negative on error.
static inline ptrdiff_t l_write_all(L_FD fd, const void *buf, size_t count);
/// Opens a directory for reading. Returns 0 on success, -1 on error.
static inline int l_opendir(const char *path, L_Dir *dir);
/// Reads the next directory entry. Returns pointer to L_DirEntry or NULL when done.
static L_DirEntry *l_readdir(L_Dir *dir);
/// Closes a directory handle.
static inline void l_closedir(L_Dir *dir);

/// Maps a file or anonymous memory into the process address space
static inline void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset);
/// Unmaps a previously mapped region
static inline int l_munmap(void *addr, size_t length);

/// Fill buf with len bytes of cryptographic-quality random data (getrandom(2) on Linux, BCryptGenRandom on Windows). Returns 0 on success, -1 on error.
static inline int l_getrandom(void *buf, size_t len);

// Arena function declarations
/// Allocate an arena of `size` bytes via mmap. On failure, base=NULL.
static inline L_Arena l_arena_init(size_t size);
/// Bump-allocate n bytes (8-byte aligned). Returns NULL if arena is full.
static inline void *l_arena_alloc(L_Arena *a, size_t n);
/// Reset used to 0. Memory is NOT freed — arena can be reused.
static inline void l_arena_reset(L_Arena *a);
/// Free the backing memory. Sets base=NULL.
static inline void l_arena_free(L_Arena *a);

// Buffer function declarations
/// Zero-initialize a buffer.
static inline void l_buf_init(L_Buf *b);
/// Append n bytes. Returns 0 on success, -1 on failure.
static inline int l_buf_push(L_Buf *b, const void *src, size_t n);
/// Formatted append using l_vsnprintf. Returns bytes written or -1.
#ifdef L_WITHSNPRINTF
static inline int l_buf_printf(L_Buf *b, const char *fmt, ...);
#endif
/// Set len=0 (keep allocated memory).
static inline void l_buf_clear(L_Buf *b);
/// Free backing memory and zero the struct.
static inline void l_buf_free(L_Buf *b);

// L_Str — fat string (pointer + length) function declarations

/// Wrap a C string (computes strlen).
static inline L_Str l_str(const char *cstr);
/// Wrap pointer+length.
static inline L_Str l_str_from(const char *data, size_t len);
/// Return null string {NULL, 0}.
static inline L_Str l_str_null(void);

/// 1 if equal, 0 otherwise.
static inline int l_str_eq(L_Str a, L_Str b);
/// Lexicographic compare (like strcmp).
static inline int l_str_cmp(L_Str a, L_Str b);
/// 1 if s starts with prefix.
static inline int l_str_startswith(L_Str s, L_Str prefix);
/// 1 if s ends with suffix.
static inline int l_str_endswith(L_Str s, L_Str suffix);
/// 1 if s contains needle.
static inline int l_str_contains(L_Str s, L_Str needle);

/// Substring (zero-copy).
static inline L_Str l_str_sub(L_Str s, size_t start, size_t len);
/// Trim leading+trailing whitespace (zero-copy).
static inline L_Str l_str_trim(L_Str s);
/// Trim leading whitespace (zero-copy).
static inline L_Str l_str_ltrim(L_Str s);
/// Trim trailing whitespace (zero-copy).
static inline L_Str l_str_rtrim(L_Str s);
/// Find char in string, -1 if not found.
static inline ptrdiff_t l_str_chr(L_Str s, char c);
/// Find last occurrence of char, -1 if not found.
static inline ptrdiff_t l_str_rchr(L_Str s, char c);
/// Find substring, -1 if not found.
static inline ptrdiff_t l_str_find(L_Str haystack, L_Str needle);

/// Copy string into arena.
static inline L_Str l_str_dup(L_Arena *a, L_Str s);
/// Concatenate two strings into arena.
static inline L_Str l_str_cat(L_Arena *a, L_Str x, L_Str y);
/// Null-terminated C string copy in arena.
static inline char *l_str_cstr(L_Arena *a, L_Str s);
/// strdup into arena as L_Str.
static inline L_Str l_str_from_cstr(L_Arena *a, const char *cstr);

/// Split string by delimiter. Returns count; *out is arena-allocated array.
static inline int l_str_split(L_Arena *a, L_Str s, L_Str delim, L_Str **out);
/// Join strings with separator.
static inline L_Str l_str_join(L_Arena *a, const L_Str *parts, int count, L_Str sep);

/// Uppercase copy in arena (ASCII).
static inline L_Str l_str_upper(L_Arena *a, L_Str s);
/// Lowercase copy in arena (ASCII).
static inline L_Str l_str_lower(L_Arena *a, L_Str s);
/// Replace all occurrences of find with repl in s. Result is arena-allocated.
static inline L_Str l_str_replace(L_Arena *a, L_Str s, L_Str find, L_Str repl);

/// Append L_Str to buf. Returns 0 on success, -1 on failure.
static inline int l_buf_push_str(L_Buf *b, L_Str s);
/// Append C string to buf. Returns 0 on success, -1 on failure.
static inline int l_buf_push_cstr(L_Buf *b, const char *s);
/// Append decimal int to buf. Returns 0 on success, -1 on failure.
static inline int l_buf_push_int(L_Buf *b, int value);
/// Return L_Str view of buf contents.
static inline L_Str l_buf_as_str(const L_Buf *b);

// I/O multiplexing
/// Poll file descriptors for events. Returns number ready, 0 on timeout, -1 on error.
static inline int l_poll(L_PollFd *fds, int nfds, int timeout_ms);

// Signal handling
/// Set signal handler. Returns previous handler or L_SIG_DFL on error.
static inline L_SigHandler l_signal(int sig, L_SigHandler handler);

// Environment manipulation
/// Set environment variable. Returns 0 on success, -1 on error.
static int l_setenv(const char *name, const char *value);
/// Unset environment variable. Returns 0 on success, -1 on error.
static int l_unsetenv(const char *name);

// Scatter-gather I/O
/// Write from multiple buffers. Returns bytes written or -1 on error.
static inline ptrdiff_t l_writev(L_FD fd, const L_IoVec *iov, int iovcnt);
/// Read into multiple buffers. Returns bytes read or -1 on error.
static inline ptrdiff_t l_readv(L_FD fd, L_IoVec *iov, int iovcnt);

// Terminal detection
/// Returns 1 if fd is a terminal, 0 otherwise.
static inline int l_isatty(L_FD fd);

// Hash map (arena-backed, fixed capacity)
/// Initialize a map with given capacity (rounded to power of 2).
static inline L_Map l_map_init(L_Arena *a, size_t capacity);
/// Get value by key. Returns value pointer or NULL if not found.
static inline void *l_map_get(L_Map *m, const char *key, size_t keylen);
/// Put key-value pair. Returns 0 on success, -1 if full (>75% load).
static inline int l_map_put(L_Map *m, const char *key, size_t keylen, void *value);
/// Delete key. Returns 0 on success, -1 if not found.
static inline int l_map_del(L_Map *m, const char *key, size_t keylen);

// Time conversion
/// Convert Unix timestamp to UTC broken-down time.
static inline L_Tm l_gmtime(long long timestamp);
/// Convert Unix timestamp to local broken-down time.
static inline L_Tm l_localtime(long long timestamp);
/// Convert UTC broken-down time to Unix timestamp (seconds since 1970-01-01 00:00:00 UTC).
static inline long long l_mktime(L_Tm *tm);
/// Format time into buffer. Returns bytes written (excluding NUL).
static inline int l_strftime(char *buf, size_t max, const char *fmt, const L_Tm *tm);

// Glob pattern matching
/// Match pattern against string. Returns 0 if matches, -1 if no match.
static inline int l_fnmatch(const char *pattern, const char *string);
/// Expand a glob pattern into matching paths. Single-level only (no recursive **).
/// Returns number of matches; *out_paths and *out_count are set in the arena.
static inline int l_glob(const char *pattern, L_Str **out_paths, int *out_count, L_Arena *a);

// SHA-256
/// Initialize SHA-256 context.
static inline void l_sha256_init(L_Sha256 *ctx);
/// Feed data into SHA-256.
static inline void l_sha256_update(L_Sha256 *ctx, const void *data, size_t len);
/// Finalize and produce 32-byte hash.
static inline void l_sha256_final(L_Sha256 *ctx, unsigned char hash[32]);
/// One-shot SHA-256.
static inline void l_sha256(const void *data, size_t len, unsigned char hash[32]);

/// Encode `len` bytes from `data` into standard Base64. Writes at most `outsz` bytes (including NUL)
/// to `out`. Returns the number of characters written (excluding NUL), or -1 if `outsz` is too small.
static inline ptrdiff_t l_base64_encode(const void *data, size_t len, char *out, size_t outsz);
/// Decode Base64 text of length `inlen` into `out`. Returns decoded byte count, or -1 on invalid input
/// or insufficient `outsz`. Ignores whitespace; accepts both standard (+/) and URL-safe (-_) alphabets.
static inline ptrdiff_t l_base64_decode(const char *in, size_t inlen, void *out, size_t outsz);

/// Gets the current working directory into buf (up to size bytes). Returns buf on success, NULL on error.
static inline char *l_getcwd(char *buf, size_t size);
/// Changes the current working directory
static inline int l_chdir(const char *path);

/// Creates a pipe. fds[0] is the read end, fds[1] is the write end. Returns 0 on success, -1 on error.
static inline int l_pipe(L_FD fds[2]);

/// Duplicates fd, returning a new descriptor on success or -1 on error.
static inline int l_dup(L_FD fd);

/// Duplicates oldfd onto newfd. Returns newfd on success, -1 on error.
static inline int l_dup2(L_FD oldfd, L_FD newfd);

/// Returns the current process ID.
static inline L_PID l_getpid(void);

/// Spawns a new process with explicit stdio. Use L_SPAWN_INHERIT to keep the parent's stream.
static L_PID l_spawn_stdio(const char *path, char *const argv[], char *const envp[],
                    L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd);

/// Spawns a new process, inheriting the current stdio descriptors.
/// Returns process ID/handle on success, -1 on error.
/// path: executable path. argv: NULL-terminated argument array. envp: NULL-terminated environment (NULL = inherit).
static inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[]);

/// Waits for a spawned process to finish. Returns 0 on success, -1 on error.
/// exitcode receives the process exit code.
static inline int l_wait(L_PID pid, int *exitcode);

/// Executes a shell command string. Returns the exit code, or -1 on spawn failure.
/// Uses /bin/sh -c on Unix, cmd.exe /c on Windows.
static inline int l_system(const char *cmd);

#if defined(__unix__) || defined(__wasi__)
// Unix and WASI functions
/// Repositions the file offset of fd
static inline off_t l_lseek(L_FD fd, off_t offset, int whence);
/// Creates a directory with the given permissions
static inline int l_mkdir(const char *path, mode_t mode);
/// Yields the processor to other threads
static inline int l_sched_yield(void);
/// Fork the current process. Returns child pid to parent, 0 to child, -1 on error.
static inline L_PID l_fork(void);
/// Replace the current process image. Does not return on success.
static inline int l_execve(const char *path, char *const argv[], char *const envp[]);
/// Wait for a child process. Returns child pid on success, -1 on error.
static inline L_PID l_waitpid(L_PID pid, int *status, int options);
/// Returns the parent process ID.
static inline L_PID l_getppid(void);
/// Sends signal sig to process pid. Returns 0 on success, -1 on error.
static inline int l_kill(L_PID pid, int sig);
#endif

#ifdef L_WITHSOCKETS
// Byte order helpers
/// Convert 16-bit value from host to network byte order
static inline unsigned short l_htons(unsigned short h);
/// Convert 32-bit value from host to network byte order
static inline unsigned int l_htonl(unsigned int h);
/// Parse dotted-quad IP string to network-order u32. Returns 0 on error.
static inline unsigned int l_inet_addr(const char *ip);
/// Resolve hostname to IPv4 dotted-quad string. ip_out must be at least 16 bytes. If hostname is already IPv4 text, copies it unchanged. Returns 0 on success, -1 on error.
static inline int l_resolve(const char *hostname, char *ip_out);

// TCP socket functions
/// Create a TCP socket. Returns socket fd or -1 on error.
static inline L_SOCKET l_socket_tcp(void);
/// Connect to addr:port. Returns 0 on success, -1 on error.
static inline int l_socket_connect(L_SOCKET sock, const char *addr, int port);
/// Bind socket to port on all interfaces. Returns 0/-1.
static inline int l_socket_bind(L_SOCKET sock, int port);
/// Listen for connections. Returns 0/-1.
static inline int l_socket_listen(L_SOCKET sock, int backlog);
/// Accept connection. Returns new socket or -1.
static inline L_SOCKET l_socket_accept(L_SOCKET sock);
/// Send data. Returns bytes sent or -1.
static inline ptrdiff_t l_socket_send(L_SOCKET sock, const void *data, size_t len);
/// Receive data. Returns bytes received, 0 on close, -1 on error.
static inline ptrdiff_t l_socket_recv(L_SOCKET sock, void *buf, size_t len);
/// Close socket.
static inline void l_socket_close(L_SOCKET sock);

// UDP socket functions
/// Create a UDP socket. Returns socket fd or -1 on error.
static inline L_SOCKET l_socket_udp(void);
/// Send data to addr:port via UDP. Returns bytes sent or -1.
static inline ptrdiff_t l_socket_sendto(L_SOCKET s, const void *data, size_t len, const char *addr, int port);
/// Receive data via UDP. addr_out (>=16 bytes) and port_out receive sender info. Returns bytes received or -1.
static inline ptrdiff_t l_socket_recvfrom(L_SOCKET s, void *buf, size_t len, char *addr_out, int *port_out);

// Generic address-based socket API (IPv4 and IPv6)
/// Build an IPv4 L_SockAddr from dotted-quad string and port. Returns 0 on success, -1 on error.
static inline int l_sockaddr_ipv4(L_SockAddr *sa, const char *ip, int port);
/// Build an IPv6 L_SockAddr from IPv6 text and port. Returns 0 on success, -1 on error.
static inline int l_sockaddr_ipv6(L_SockAddr *sa, const char *ip, int port);
/// Parse IPv6 text representation into 16-byte binary. Returns 1 on success, 0 on error.
static inline int l_parse_ipv6(const char *ip, unsigned char out[16]);
/// Format 16-byte IPv6 binary to text. buf must be at least L_INET6_ADDRSTRLEN bytes. Returns buf.
static inline char *l_format_ipv6(const unsigned char addr[16], char *buf, size_t bufsz);
/// Create a socket of the given family (L_AF_INET or L_AF_INET6) and type (L_SOCK_STREAM or L_SOCK_DGRAM). Returns socket fd or -1.
static inline L_SOCKET l_socket_open(int family, int type);
/// Connect socket to an L_SockAddr. Returns 0 on success, -1 on error.
static inline int l_socket_connect_addr(L_SOCKET sock, const L_SockAddr *addr);
/// Bind socket to an L_SockAddr. Returns 0 on success, -1 on error.
static inline int l_socket_bind_addr(L_SOCKET sock, const L_SockAddr *addr);
/// Send data to an L_SockAddr via UDP. Returns bytes sent or -1.
static inline ptrdiff_t l_socket_sendto_addr(L_SOCKET s, const void *data, size_t len, const L_SockAddr *dest);
/// Receive data via UDP. src receives sender address. Returns bytes received or -1.
static inline ptrdiff_t l_socket_recvfrom_addr(L_SOCKET s, void *buf, size_t len, L_SockAddr *src);
#ifndef _WIN32
/// Create a Unix domain socket and connect to the given path. Returns socket fd or -1.
static inline L_SOCKET l_socket_unix_connect(const char *path);
#endif
#endif // L_WITHSOCKETS

#endif // L_WITHDEFS

#ifdef L_WITHSTART
#ifndef L_WITHSTART_DONE
#define L_WITHSTART_DONE

#ifdef __unix__

#if defined(__x86_64__)
/* startup code for x86_64 only */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "pop %rdi\n"                // argc   (first arg, %rdi)
    "mov %rsp, %rsi\n"          // argv[] (second arg, %rsi)
    "lea 8(%rsi,%rdi,8),%rdx\n" // then a NULL then envp (third arg, %rdx)
    "and $-16, %rsp\n"          // x86 ABI : stack must be 16-byte aligned before CALL
    "call main\n"               // main() returns the status code, we'll exit with it.
    "movzb %al, %rdi\n"         // retrieve exit code from 8 lower bits
    "mov $60, %rax\n"           // NR_exit == 60
    "syscall\n"                 // really exit
    "hlt\n"                     // ensure it does not return
    "");
#elif defined(__arm__)
/* startup code for ARM */
asm(".section .text\n"
    ".weak _start\n"
    "_start:\n"
#if defined(__THUMBEB__) || defined(__THUMBEL__)
    /* We enter here in 32-bit mode but if some previous functions were in
     * 16-bit mode, the assembler cannot know, so we need to tell it we're in
     * 32-bit now, then switch to 16-bit (is there a better way to do it than
     * by hand ?) and tell the asm we're now in 16-bit mode so that
     * it generates correct instructions. Note that we do not support thumb1.
     */
    ".code 32\n"
    "add     r0, pc, #1\n"
    "bx      r0\n"
    ".code 16\n"
#endif
    "pop {r0}\n"                   // argc was in the stack
    "mov r1, sp\n"                 // argv = sp
    "add r2, r1, r0, lsl #2\n"    // envp = argv + 4*argc ...
    "add r2, r2, #4\n"            //        ... + 4
    "and r3, r1, #-8\n"           // AAPCS : sp must be 8-byte aligned in the
    "mov sp, r3\n"                //         callee, an bl doesn't push (lr=pc)
    "bl main\n"                   // main() returns the status code, we'll exit with it.
    "movs r7, #1\n"               // NR_exit == 1
    "svc #0\n"
    );
#elif defined(__aarch64__)
/* startup code for AArch64 */
asm(".section .text\n"
    ".weak _start\n"
    "_start:\n"
    "ldr x0, [sp]\n"              // argc (x0) was in the stack
    "add x1, sp, 8\n"             // argv (x1) = sp
    "lsl x2, x0, 3\n"             // envp (x2) = 8*argc ...
    "add x2, x2, 8\n"             //           + 8 (skip null)
    "add x2, x2, x1\n"            //           + argv
    "and sp, x1, -16\n"           // sp must be 16-byte aligned in the callee
    "bl main\n"                   // main() returns the status code, we'll exit with it.
    "mov x8, 93\n"                // NR_exit == 93
    "svc #0\n"
    );
#elif defined(__riscv)
/* startup code for RISC-V (rv64gc) */
asm(".section .text\n"
    ".weak _start\n"
    "_start:\n"
    ".option push\n"
    ".option norelax\n"
    "lla gp, __global_pointer$\n" // init global pointer (must precede any gp-relative access)
    ".option pop\n"
    "ld a0, 0(sp)\n"              // argc
    "addi a1, sp, 8\n"            // argv
    "slli a2, a0, 3\n"            // envp = 8*argc ...
    "add a2, a2, a1\n"            //       + argv
    "addi a2, a2, 8\n"            //       + 8 (skip null)
    "andi sp, sp, -16\n"          // 16-byte stack alignment
    "call main\n"                 // main() returns status code
    "li a7, 93\n"                 // NR_exit == 93
    "ecall\n"
    );
#endif

#elif defined(__wasi__)
// WASI: _start is a normal C function exported to the runtime.
// Types and imports are defined at file scope above.

static char **l_envp;

int main(int argc, char *argv[]);

void _start(void) {
    // Fetch command-line arguments
    __wasi_size_t_w argc = 0, argv_buf_size = 0;
    __wasi_args_sizes_get(&argc, &argv_buf_size);
    static uint8_t *argv_ptrs[256];
    static uint8_t argv_buf[16384];
    if (argc > 255) argc = 255;
    if (argv_buf_size > sizeof(argv_buf)) argv_buf_size = sizeof(argv_buf);
    __wasi_args_get(argv_ptrs, argv_buf);
    argv_ptrs[argc] = 0;

    // Fetch environment variables
    __wasi_size_t_w env_count = 0, env_buf_size = 0;
    __wasi_environ_sizes_get(&env_count, &env_buf_size);
    static uint8_t *env_ptrs[512];
    static uint8_t env_buf[32768];
    if (env_count > 511) env_count = 511;
    if (env_buf_size > sizeof(env_buf)) env_buf_size = sizeof(env_buf);
    __wasi_environ_get(env_ptrs, env_buf);
    env_ptrs[env_count] = 0;
    l_envp = (char **)env_ptrs;

    int ret = main((int)argc, (char **)argv_ptrs);
    __wasi_proc_exit((unsigned)ret);
}

#else // windows

// MINGW complains I need to define this. It gets called inside main apparently
void __main() {}
int main(int argc, char* argv[]);

// It also complain about this not being defined if compiled with -fwhole-program
__attribute__((__used__)) 
extern int atexit(void (*f)(void)){ (void) f; return 0;}

// Some routines liberally taken from various versions of minicrt, mincrt and tinylib. Look them up.

// CommandLineToArgvW is not in kernel32.dll
// Under Windows we take utf16 at the boundaries, but represents string internally as utf-8 (aka char).
static char** parseCommandLine(char* szCmdLine, int * argc)
{
    int arg_count = 0;
    int char_count = 0;
    char break_char = ' ';
    char *c;
    char **ret;
    char *ret_str;

    //
    //  Consume all spaces.  After this, we're either at
    //  the end of string, or we have an arg, and it
    //  might start with a quote
    //

    c = szCmdLine;
    while (*c == ' ') c++;
    if (*c == '"') {
        break_char = '"';
        c++;
    }

    while (*c != '\0') {
        if (*c == break_char) {
            break_char = ' ';
            c++;
            while (*c == break_char) c++;
            if (*c == '"') {
                break_char = '"';
                c++;
            }
            arg_count++;
        } else {
            char_count++;

            //
            //  If we hit a break char, we count the argument then.
            //  If we hit end of string, count it here; note we're
            //  only counting it if we counted a character before it
            //  (ie., trailing whitespace is not an arg.)
            //

            c++;

            if (*c == '\0') {
                arg_count++;
            }
        }
    }

    *argc = arg_count;

    ret = LocalAlloc( LMEM_FIXED, (arg_count * sizeof(char*)) + (char_count + arg_count) * sizeof(char));

    ret_str = (char*)(ret + arg_count);

    arg_count = 0;
    ret[arg_count] = ret_str;

    //
    //  Consume all spaces.  After this, we're either at
    //  the end of string, or we have an arg, and it
    //  might start with a quote
    //

    c = szCmdLine;
    while (*c == ' ') c++;
    if (*c == '"') {
        break_char = '"';
        c++;
    }

    while (*c != '\0') {
        if (*c == break_char) {
            *ret_str = '\0';
            ret_str++;

            break_char = ' ';
            c++;
            while (*c == break_char) c++;
            if (*c == '"') {
                break_char = '"';
                c++;
            }
            if (*c != '\0') {
                arg_count++;
                ret[arg_count] = ret_str;
            }
        } else {
            *ret_str = *c;
            ret_str++;

            //
            //  If we hit a break char, we count the argument then.
            //  If we hit end of string, count it here; note we're
            //  only counting it if we counted a character before it
            //  (ie., trailing whitespace is not an arg.)
            //

            c++;

            if (*c == '\0') {
                *ret_str = '\0';
            }
        }
    }


    return ret;
}

#ifdef __GNUC__
__attribute((externally_visible))
#endif
#ifdef __i686__
__attribute((force_align_arg_pointer))
#endif
int WINAPI mainCRTStartup(void)
{
    char **szArglist;
    int nArgs;
    int i;

    // TODO: Can these fail??
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    wchar_t *u16cmdline = GetCommandLineW();

    // Max bytes needed when converting from utf-16 to utf-8.
    // https://stackoverflow.com/questions/55056322/maximum-utf-8-string-size-given-utf-16-size
    size_t required_size = l_wcslen(u16cmdline) * 4;

    // Cannot blow the stack because of limited lenght of windows cmd line :
    // https://stackoverflow.com/questions/3205027/maximum-length-of-command-line-string
    char buffer[required_size];

    int size = WideCharToMultiByte(CP_UTF8, 0, u16cmdline, -1, buffer,required_size, NULL, NULL);
    l_exitif(!size, GetLastError(), "Error converting command line to utf-8. Are you using some ungodly character?");

    szArglist = parseCommandLine(buffer, &nArgs);

    if( NULL == szArglist )
    {
        return 0;
    }
    else {
        i = main(nArgs, szArglist);
    }

    LocalFree(szArglist);

    // https://nullprogram.com/blog/2023/02/15/ recommends the below for non console apps ...
    // TerminateProcess(GetCurrentProcess(), i);
    return(i);
}
#endif // __unix__
#endif // L_WITHSTART_DONE
#endif // L_WITHSTART

#ifndef L_OSH
#define L_OSH

#ifndef L_DONTOVERRIDE

#  define wcslen l_wcslen
#  define strlen l_strlen
#  define strcpy l_strcpy
#  define strchr l_strchr
#  define strrchr l_strrchr
#  define strstr l_strstr
#  define strcmp l_strcmp
#  define strncmp l_strncmp
#  define strncpy l_strncpy
#  define strcat l_strcat
#  define strncat l_strncat
#  define strcasecmp l_strcasecmp
#  define strncasecmp l_strncasecmp
#  define strspn l_strspn
#  define strcspn l_strcspn
#  define strpbrk l_strpbrk
#  define strtok_r l_strtok_r
#  define strsep l_strsep
#  define bin2hex l_bin2hex
#  define hex2bin l_hex2bin
#  define basename l_basename
#  define dirname l_dirname

#  define isdigit l_isdigit
#  define isspace l_isspace
#  define isalpha l_isalpha
#  define isalnum l_isalnum
#  define isupper l_isupper
#  define islower l_islower
#  define toupper l_toupper
#  define tolower l_tolower
#  define isprint l_isprint
#  define isxdigit l_isxdigit
#  define abs l_abs
#  define labs l_labs
#  define llabs l_llabs
#  define atol l_atol
#  define atoi l_atoi
#  define strtoul l_strtoul
#  define strtol l_strtol
#  define strtoull l_strtoull
#  define strtoll l_strtoll
#  define strtod  l_strtod
#  define strtof  l_strtof
#  define atof    l_atof
#  define itoa l_itoa

#  define fabs  l_fabs
#  define floor l_floor
#  define ceil  l_ceil
#  define fmod  l_fmod
#  define sqrt  l_sqrt
#  define sin   l_sin
#  define cos   l_cos
#  define exp   l_exp
#  define log   l_log
#  define pow   l_pow
#  define atan2 l_atan2
#  define tan   l_tan
#  define asin  l_asin
#  define acos  l_acos
#  define atan  l_atan
#  define log10 l_log10
#  define log2  l_log2
#  define round l_round
#  define trunc l_trunc
#  define hypot l_hypot

#  define memmove l_memmove
#  define memset l_memset
#  define memcmp l_memcmp
#  define memcpy l_memcpy
#  define memchr l_memchr
#  define memrchr l_memrchr
#  define strnlen l_strnlen
#  define memmem  l_memmem

#  define rand l_rand
#  define srand l_srand
#  define qsort l_qsort
#  define bsearch l_bsearch

#  define vsnprintf l_vsnprintf
#  define snprintf  l_snprintf
#  define dprintf   l_dprintf
#  define printf    l_printf
#  define fprintf   l_fprintf
#  define vprintf   l_vprintf
#  define vfprintf  l_vfprintf

#  define exit l_exit
#  define close l_close
#  define read l_read
#  define write l_write
#  define puts l_puts
#  define lseek l_lseek
#  define truncate l_truncate
#  define ftruncate l_ftruncate
#  define dup l_dup
#  define mkdir l_mkdir
#  define chdir l_chdir
#  define getcwd l_getcwd
#  define pipe l_pipe
#  define dup2 l_dup2
#  define sched_yield l_sched_yield
#  define getpid l_getpid
#  ifndef _WIN32
#    define getppid l_getppid
#    define kill l_kill
#  endif
#  define unlink l_unlink
#  define rmdir l_rmdir
#  define rename l_rename
#  define access l_access
#  define chmod l_chmod
#  define symlink l_symlink
#  define readlink l_readlink
#  define realpath l_realpath
#  ifndef PATH_MAX
#    define PATH_MAX L_PATH_MAX
#  endif
#  define F_OK L_F_OK
#  define R_OK L_R_OK
#  define W_OK L_W_OK
#  define X_OK L_X_OK
#  define stat l_stat
#  define fstat l_fstat
#  define opendir l_opendir
#  define readdir l_readdir
#  define closedir l_closedir
#  define mmap l_mmap
#  define munmap l_munmap

#  define exitif l_exitif
#  define getenv l_getenv
#  define getopt l_getopt
#  define optarg l_optarg
#  define optind l_optind
#  define opterr l_opterr
#  define optopt l_optopt
#  define setenv l_setenv
#  define unsetenv l_unsetenv
#  define isatty l_isatty
#  define poll l_poll
#  define signal l_signal
#  define writev l_writev
#  define readv l_readv
#  define fnmatch l_fnmatch
#  define system l_system
#  define gmtime l_gmtime
#  define mktime l_mktime
#  define strftime l_strftime
#  define open_read l_open_read
#  define open_write l_open_write
#  define open_readwrite l_open_readwrite
#  define open_append l_open_append
#  define open_trunc l_open_trunc

#define STDIN    L_STDIN
#define STDOUT   L_STDOUT
#define STDERR   L_STDERR

#  undef strerror
#  define strerror l_strerror
#  undef errno
#  define errno    l_errno()

#  undef ENOENT
#  define ENOENT   L_ENOENT
#  undef EACCES
#  define EACCES   L_EACCES
#  undef EBADF
#  define EBADF    L_EBADF
#  undef EEXIST
#  define EEXIST   L_EEXIST
#  undef EINVAL
#  define EINVAL   L_EINVAL
#  undef ENOMEM
#  define ENOMEM   L_ENOMEM
#  undef EAGAIN
#  define EAGAIN   L_EAGAIN
#  undef EPIPE
#  define EPIPE    L_EPIPE
#  undef ENOSPC
#  define ENOSPC   L_ENOSPC
#  undef ENOTDIR
#  define ENOTDIR  L_ENOTDIR
#  undef EISDIR
#  define EISDIR   L_EISDIR
#  undef ENOTEMPTY
#  define ENOTEMPTY L_ENOTEMPTY

#endif // L_DONT_OVERRIDE

// Error state (single-threaded; no TLS in freestanding)
static int l_last_errno;

static inline int l_errno(void) {
    return l_last_errno;
}

static inline void l_set_errno(int err) {
    l_last_errno = err;
}

static inline void l_set_errno_from_ret(long ret) {
    l_last_errno = (ret < 0) ? (int)(-ret) : 0;
}

// Character classification (platform-independent)
static inline int l_isprint(int c) { return c >= 0x20 && c <= 0x7e; }
static inline int l_isxdigit(int c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

// Absolute value (platform-independent)
static inline int l_abs(int x) { return x < 0 ? -x : x; }
static inline long l_labs(long x) { return x < 0 ? -x : x; }
static inline long long l_llabs(long long x) { return x < 0 ? -x : x; }

// Random number generation (xorshift32, single-threaded)
static unsigned int l_rand_state = 1;

static inline void l_srand(unsigned int seed) {
    l_rand_state = seed;
}

static inline unsigned int l_rand(void) {
    l_rand_state ^= l_rand_state << 13;
    l_rand_state ^= l_rand_state >> 17;
    l_rand_state ^= l_rand_state << 5;
    return l_rand_state;
}

// Context-based RNG (independent streams)
static inline void l_rand_ctx_init(L_RandCtx *ctx, unsigned int seed) {
    ctx->state = seed ? seed : 1;
}

static inline void l_srand_ctx(L_RandCtx *ctx, unsigned int seed) {
    ctx->state = seed ? seed : 1;
}

static inline unsigned int l_rand_ctx(L_RandCtx *ctx) {
    ctx->state ^= ctx->state << 13;
    ctx->state ^= ctx->state >> 17;
    ctx->state ^= ctx->state << 5;
    return ctx->state;
}

// Option parser state (single-threaded; no TLS in freestanding)
/// Points to the argument of the current option (set by l_getopt)
static char *l_optarg;
/// Index of the next argv element to process (starts at 1)
static int   l_optind = 1;
/// Reserved for POSIX compat; l_getopt itself does no I/O
static int   l_opterr __attribute__((unused)) = 1;
/// Set to the unknown option character when l_getopt returns '?'
static int   l_optopt;
static int   l__optpos;  /* position within a grouped short-option cluster */

/// Parses command-line options. optstring lists valid option chars; trailing ':' means the option
/// takes an argument. Returns the option char on match, '?' for unknown options, -1 when done.
static inline int l_getopt(int argc, char *const argv[], const char *optstring) {
    l_optarg = (char *)0;

    if (l_optind >= argc)
        return -1;

    char *arg = argv[l_optind];

    /* Not an option, or bare "-", or "--" → stop */
    if (arg[0] != '-' || arg[1] == '\0') return -1;
    if (arg[1] == '-' && arg[2] == '\0') { l_optind++; return -1; }

    int pos = l__optpos ? l__optpos : 1;
    char c = arg[pos];
    l_optopt = c;

    /* Search optstring */
    const char *p = optstring;
    while (*p) {
        if (*p == c) break;
        p++;
        if (*p == ':') p++;   /* skip colon */
    }

    if (*p == '\0') {
        /* Unknown option */
        if (!arg[pos + 1]) { l_optind++; l__optpos = 0; }
        else                { l__optpos = pos + 1; }
        return '?';
    }

    /* Known option — does it need an argument? */
    if (*(p + 1) == ':') {
        /* Argument required */
        if (arg[pos + 1]) {
            /* Argument glued to option: -oFILE */
            l_optarg = &arg[pos + 1];
        } else if (l_optind + 1 < argc) {
            /* Argument is next argv element: -o FILE */
            l_optind++;
            l_optarg = argv[l_optind];
        } else {
            /* Missing argument */
            l_optind++;
            l__optpos = 0;
            return '?';
        }
        l_optind++;
        l__optpos = 0;
    } else {
        /* No argument — advance within cluster or to next argv */
        if (arg[pos + 1]) {
            l__optpos = pos + 1;
        } else {
            l_optind++;
            l__optpos = 0;
        }
    }

    return c;
}

// Context-based getopt (reentrant, for nested option parsing)
static inline void l_getopt_ctx_init(L_GetoptCtx *ctx) {
    ctx->arg = (char *)0;
    ctx->ind = 1;
    ctx->opt = 0;
    ctx->_pos = 0;
}

static inline int l_getopt_ctx(L_GetoptCtx *ctx, int argc, char *const argv[], const char *optstring) {
    ctx->arg = (char *)0;

    if (ctx->ind >= argc)
        return -1;

    char *a = argv[ctx->ind];

    if (a[0] != '-' || a[1] == '\0') return -1;
    if (a[1] == '-' && a[2] == '\0') { ctx->ind++; return -1; }

    int pos = ctx->_pos ? ctx->_pos : 1;
    char c = a[pos];
    ctx->opt = c;

    const char *p = optstring;
    while (*p) {
        if (*p == c) break;
        p++;
        if (*p == ':') p++;
    }

    if (*p == '\0') {
        if (!a[pos + 1]) { ctx->ind++; ctx->_pos = 0; }
        else             { ctx->_pos = pos + 1; }
        return '?';
    }

    if (*(p + 1) == ':') {
        if (a[pos + 1]) {
            ctx->arg = &a[pos + 1];
        } else if (ctx->ind + 1 < argc) {
            ctx->ind++;
            ctx->arg = argv[ctx->ind];
        } else {
            ctx->ind++;
            ctx->_pos = 0;
            return '?';
        }
        ctx->ind++;
        ctx->_pos = 0;
    } else {
        if (a[pos + 1]) {
            ctx->_pos = pos + 1;
        } else {
            ctx->ind++;
            ctx->_pos = 0;
        }
    }

    return c;
}

static inline const char *l_strerror(int errnum) {
    switch (errnum) {
        case 0:            return "Success";
        case L_ENOENT:     return "No such file or directory";
        case L_EACCES:     return "Permission denied";
        case L_EBADF:      return "Bad file descriptor";
        case L_EEXIST:     return "File exists";
        case L_EINVAL:     return "Invalid argument";
        case L_ENOMEM:     return "Cannot allocate memory";
        case L_EAGAIN:     return "Resource temporarily unavailable";
        case L_EPIPE:      return "Broken pipe";
        case L_ENOSPC:     return "No space left on device";
        case L_ENOTDIR:    return "Not a directory";
        case L_EISDIR:     return "Is a directory";
        case L_ENOTEMPTY:  return "Directory not empty";
        default:           return "Unknown error";
    }
}

static inline size_t l_wcslen(const wchar_t *s) {
    size_t len = 0;
    while(s[len] != L'\0') ++len;
    return len;
}

static inline size_t l_strlen(const char *str)
{
    const char *p = str;

    /* Align to word boundary one byte at a time. */
    while ((uintptr_t)p & (sizeof(uintptr_t) - 1)) {
        if (!*p)
            return (size_t)(p - str);
        p++;
    }

    /* Word-at-a-time: Hacker's Delight has-zero-byte trick.
     * LONES = 0x0101...01, HIGHS = 0x8080...80.
     * (w - LONES) & ~w & HIGHS is non-zero iff w contains a zero byte. */
    typedef uintptr_t word_alias __attribute__((may_alias));
    const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;  /* 0x01010101... */
    const uintptr_t highs = lones << 7;                 /* 0x80808080... */
    const word_alias *wp = (const word_alias *)(const void *)p;
    while (1) {
        uintptr_t w = *wp;
        if ((w - lones) & ~w & highs)
            break;
        wp++;
    }

    /* Byte scan to find the exact zero byte. */
    p = (const char *)wp;
    while (*p)
        p++;

    return (size_t)(p - str);
}

static inline void *l_memmove(void *dst, const void *src, size_t len)
{
    char *d = (char *)dst;
    const char *s = (const char *)src;

    if (d <= s) {
        /* Forward copy — same word-at-a-time algorithm as l_memcpy.
         * Safe because d <= s means no read-after-write aliasing. */
        while (len && ((uintptr_t)d & (sizeof(uintptr_t) - 1U))) {
            *d++ = *s++;
            len--;
        }
        if (len >= sizeof(uintptr_t) && !((uintptr_t)s & (sizeof(uintptr_t) - 1U))) {
            typedef uintptr_t __attribute__((may_alias)) uptr_alias;
            uptr_alias       *dp = (uptr_alias *)(void *)d;
            const uptr_alias *sp = (const uptr_alias *)(const void *)s;
            size_t nw = len / sizeof(uptr_alias);
            while (nw--)
                *dp++ = *sp++;
            d   = (char *)(void *)dp;
            s   = (const char *)(const void *)sp;
            len &= sizeof(uptr_alias) - 1U;
        }
        while (len--)
            *d++ = *s++;
    } else {
        d += len;
        s += len;
        while (len--)
            *--d = *--s;
    }
    return dst;
}

static inline void *l_memset(void *dst, int b, size_t len)
{
    unsigned char *p = (unsigned char *)dst;
    unsigned char  c = (unsigned char)b;

    /* Byte-fill until naturally word-aligned */
    while (len && ((uintptr_t)p & (sizeof(uintptr_t) - 1U))) {
        *p++ = c;
        len--;
    }

    /* Word-at-a-time fill for the bulk of the buffer.
     * The may_alias attribute tells the compiler this type may alias char,
     * matching how standard memset implementations work in freestanding runtimes. */
    if (len >= sizeof(uintptr_t)) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        uptr_alias  word = c;
        size_t      nw;

        word |= word <<  8;
        word |= word << 16;
#if UINTPTR_MAX > 0xFFFFFFFFU
        word |= word << 32;
#endif
        nw = len / sizeof(uptr_alias);
        uptr_alias *wp = (uptr_alias *)(void *)p;
        while (nw--)
            *wp++ = word;
        p   = (unsigned char *)(void *)wp;
        len &= sizeof(uptr_alias) - 1U;
    }

    /* Remaining bytes */
    while (len--)
        *p++ = c;

    return dst;
}

static inline int l_memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;

    /* Align a to word boundary one byte at a time. */
    while (n && ((uintptr_t)a & (sizeof(uintptr_t) - 1U))) {
        if (*a != *b)
            return *a - *b;
        a++; b++; n--;
    }

    /* Word-at-a-time comparison when b is also word-aligned.
     * Compare one machine word per iteration; bail out on mismatch. */
    if (n >= sizeof(uintptr_t) && !((uintptr_t)b & (sizeof(uintptr_t) - 1U))) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        const uptr_alias *wa = (const uptr_alias *)(const void *)a;
        const uptr_alias *wb = (const uptr_alias *)(const void *)b;
        while (n >= sizeof(uintptr_t)) {
            if (*wa != *wb)
                break;
            wa++; wb++;
            n -= sizeof(uintptr_t);
        }
        a = (const unsigned char *)(const void *)wa;
        b = (const unsigned char *)(const void *)wb;
    }

    /* Tail bytes (or bail-out from word loop). */
    while (n--) {
        if (*a != *b)
            return *a - *b;
        a++; b++;
    }
    return 0;
}

static inline char *l_strcpy(char *dst, const char *src)
{
    char *ret = dst;

    /* Byte-copy until dst is word-aligned. */
    while ((uintptr_t)dst & (sizeof(uintptr_t) - 1U)) {
        if (!(*dst++ = *src++))
            return ret;
    }

    /* Word-at-a-time copy when src is also word-aligned.
     * Copy a whole word, then check for a zero byte using the
     * Hacker's Delight has-zero-byte trick.  When a zero is found we
     * return immediately — the null byte was already written as part of
     * the word copy, so no byte-level tail work is needed. */
    if (!((uintptr_t)src & (sizeof(uintptr_t) - 1U))) {
        typedef uintptr_t __attribute__((may_alias)) word_alias;
        const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;
        const uintptr_t highs = lones << 7;
        word_alias       *dp = (word_alias *)(void *)dst;
        const word_alias *sp = (const word_alias *)(const void *)src;
        for (;;) {
            uintptr_t w = *sp++;
            *dp++ = w;
            if ((w - lones) & ~w & highs)
                return ret; /* null was inside this word — done */
        }
    }

    /* Tail: byte-by-byte for unaligned src. */
    while ((*dst++ = *src++));
    return ret;
}

static inline char *l_strcat(char *dst, const char *src)
{
    char *ret = dst;

    dst += l_strlen(dst); /* word-at-a-time scan to end */
    while ((*dst++ = *src++));
    return ret;
}

static inline char *l_strncpy(char *dst, const char *src, size_t n)
{
    char *ret = dst;
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = '\0';
    return ret;
}

static inline char *l_strncat(char *dst, const char *src, size_t n)
{
    char *ret = dst;

    while (*dst)
        dst++;
    while (n-- && *src)
        *dst++ = *src++;
    *dst = '\0';
    return ret;
}

static inline char *l_strchr(const char *s, int c)
{
    unsigned char uc = (unsigned char)c;

    /* Searching for NUL: advance to end of string using the fast l_strlen path. */
    if (uc == 0)
        return (char *)s + l_strlen(s);

    /* Align to word boundary byte-at-a-time. */
    while ((uintptr_t)s & (sizeof(uintptr_t) - 1U)) {
        if ((unsigned char)*s == uc)
            return (char *)s;
        if (!*s)
            return NULL;
        s++;
    }

    /* Word-at-a-time: detect NUL or target byte in one pass.
     * Replicate the target byte across a machine word.  XOR-ing a word w
     * with this mask turns bytes that match the target to 0; the Hacker's
     * Delight has-zero-byte test then fires on any match.  The same test
     * on the raw word fires on NUL, so one iteration detects both events. */
    typedef uintptr_t __attribute__((may_alias)) uptr_alias;
    const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;   /* 0x0101...01 */
    const uintptr_t highs = lones << 7;                  /* 0x8080...80 */
    uintptr_t       target = uc;
    target |= target <<  8;
    target |= target << 16;
#if UINTPTR_MAX > 0xFFFFFFFFU
    target |= target << 32;
#endif
    const uptr_alias *wp = (const uptr_alias *)(const void *)s;
    for (;;) {
        uintptr_t w = *wp;
        uintptr_t x = w ^ target;
        if (((w - lones) & ~w & highs) | ((x - lones) & ~x & highs))
            break;
        wp++;
    }

    /* Byte scan to locate the exact position. */
    s = (const char *)wp;
    while ((unsigned char)*s != uc && *s)
        s++;
    return (unsigned char)*s == uc ? (char *)s : NULL;
}

/* l_memrchr is defined later in L_OSH; this declaration lets l_strrchr call it
 * on the first-include path before L_WITHDEFS forward declarations are active. */
static inline void *l_memrchr(const void *s, int c, size_t n);

static inline char *l_strrchr(const char *s, int c)
{
    size_t len = l_strlen(s);
    if ((char)c == '\0')
        return (char *)s + len;  /* pointer to the null terminator */
    return (char *)l_memrchr(s, c, len);
}

static inline int l_strncmp(const char *s1, const char *s2, size_t n) {
    /* Byte-at-a-time until s1 is word-aligned (or n exhausted). */
    while (n && ((uintptr_t)s1 & (sizeof(uintptr_t) - 1U))) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) return (int)u1 - (int)u2;
        if (u1 == '\0') return 0;
        n--;
    }
    /* Word-at-a-time when s2 is also word-aligned and >= one word remains.
     * Uses the has-zero-byte trick: (w - 0x0101..01) & ~w & 0x8080..80.
     * Break when s1's word contains a NUL or the two words differ. */
    if (n >= sizeof(uintptr_t) && !((uintptr_t)s2 & (sizeof(uintptr_t) - 1U))) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;  /* 0x0101...01 */
        const uintptr_t highs = lones << 7;                 /* 0x8080...80 */
        const uptr_alias *w1 = (const uptr_alias *)(const void *)s1;
        const uptr_alias *w2 = (const uptr_alias *)(const void *)s2;
        while (n >= sizeof(uintptr_t)) {
            uintptr_t a = *w1, b = *w2;
            if (((a - lones) & ~a & highs) | (a ^ b))
                break;
            w1++; w2++;
            n -= sizeof(uintptr_t);
        }
        s1 = (const char *)w1;
        s2 = (const char *)w2;
    }
    /* Tail: byte-at-a-time for remaining n bytes. */
    while (n-- > 0) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) return (int)u1 - (int)u2;
        if (u1 == '\0') return 0;
    }
    return 0;
}

static inline int l_strcmp(const char *s1, const char *s2) {
    /* Byte-at-a-time until s1 is word-aligned. */
    while ((uintptr_t)s1 & (sizeof(uintptr_t) - 1U)) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) return (int)u1 - (int)u2;
        if (u1 == '\0') return 0;
    }
    /* Word-at-a-time when s2 is also word-aligned.
     * Break when s1's word contains a NUL or the words differ. */
    if (!((uintptr_t)s2 & (sizeof(uintptr_t) - 1U))) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;  /* 0x0101...01 */
        const uintptr_t highs = lones << 7;                 /* 0x8080...80 */
        const uptr_alias *w1 = (const uptr_alias *)(const void *)s1;
        const uptr_alias *w2 = (const uptr_alias *)(const void *)s2;
        for (;;) {
            uintptr_t a = *w1, b = *w2;
            if (((a - lones) & ~a & highs) | (a ^ b))
                break;
            w1++; w2++;
        }
        s1 = (const char *)w1;
        s2 = (const char *)w2;
    }
    /* Byte scan to locate the exact mismatch or NUL. */
    for (;;) {
        unsigned char u1 = (unsigned char)*s1++;
        unsigned char u2 = (unsigned char)*s2++;
        if (u1 != u2) return (int)u1 - (int)u2;
        if (u1 == '\0') return 0;
    }
}

/* Forward declaration needed: l_strstr calls l_memchr which is defined later. */
static inline void *l_memchr(const void *s, int c, size_t n);

static inline char *l_strstr(const char *s1, const char *s2) {
    const size_t len = l_strlen(s2);
    if (len == 0) return (char *)s1;
    const size_t slen = l_strlen(s1);
    if (len > slen) return NULL;
    const char *end = s1 + (slen - len);
    const char first = s2[0];
    /* Use word-at-a-time l_memchr to skip ahead to the next candidate byte
     * instead of advancing one byte at a time. */
    while (s1 <= end) {
        s1 = (const char *)l_memchr(s1, first, (size_t)(end - s1 + 1));
        if (!s1) return NULL;
        if (!l_memcmp(s1, s2, len))
            return (char *)s1;
        s1++;
    }
    return NULL;
}

static inline int l_isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static inline int l_isdigit(int c)
{
    return (unsigned int)(c - '0') <= 9;
}

static inline int l_isalpha(int c)
{
    return (unsigned int)((c | 0x20) - 'a') <= ('z' - 'a');
}

static inline int l_isalnum(int c)
{
    return l_isalpha(c) || l_isdigit(c);
}

static inline int l_isupper(int c)
{
    return (unsigned int)(c - 'A') <= ('Z' - 'A');
}

static inline int l_islower(int c)
{
    return (unsigned int)(c - 'a') <= ('z' - 'a');
}

static inline int l_toupper(int c)
{
    return l_islower(c) ? c - ('a' - 'A') : c;
}

static inline int l_tolower(int c)
{
    return l_isupper(c) ? c + ('a' - 'A') : c;
}

static inline long l_atol(const char *s)
{
    unsigned long ret = 0;
    unsigned long d;
    int neg = 0;

    while (l_isspace((unsigned char)*s))
        s++;

    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (1) {
        d = (*s++) - '0';
        if (d > 9)
            break;
        ret *= 10;
        ret += d;
    }

    return neg ? -(long)ret : (long)ret;
}

static inline int l_atoi(const char *s)
{
    return (int)l_atol(s);
}

static inline unsigned long l_strtoul(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long acc = 0;
    int overflow = 0;
    int any = 0;
    int neg = 0;

    while (l_isspace((unsigned char)*s))
        s++;

    /* C99 §7.20.1.4: an optional sign is accepted; a minus sign negates
     * the converted value in the return type (unsigned wrap-around). */
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    /* Auto-detect base or consume 0x/0X prefix for hex */
    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    } else if (base == 0) {
        base = (s[0] == '0') ? 8 : 10;
    }

    if (base < 2 || base > 36) {
        if (endptr)
            *endptr = (char *)nptr;
        return 0;
    }

    for (;;) {
        unsigned char c = (unsigned char)*s;
        int digit;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'z')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            digit = c - 'A' + 10;
        else
            break;
        if (digit >= base)
            break;
        any = 1;
        if (!overflow) {
            if (acc > (ULONG_MAX - (unsigned long)digit) / (unsigned long)base) {
                overflow = 1;
                acc = ULONG_MAX;
            } else {
                acc = acc * (unsigned long)base + (unsigned long)digit;
            }
        }
        s++;
    }

    if (endptr)
        *endptr = (char *)(any ? s : nptr);
    return neg ? (unsigned long)(-(long)acc) : acc;
}

static inline long l_strtol(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    int neg = 0;
    unsigned long uval;
    char *ep;

    while (l_isspace((unsigned char)*s))
        s++;

    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    uval = l_strtoul(s, &ep, base);

    /* If no digits were parsed, reset endptr to nptr */
    if (ep == s)
        ep = (char *)nptr;
    if (endptr)
        *endptr = ep;

    if (neg) {
        if (uval >= (unsigned long)LONG_MAX + 1UL)
            return LONG_MIN;
        return -(long)uval;
    }
    if (uval > (unsigned long)LONG_MAX)
        return LONG_MAX;
    return (long)uval;
}

static inline unsigned long long l_strtoull(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long long acc = 0;
    int overflow = 0;
    int any = 0;
    int neg = 0;

    while (l_isspace((unsigned char)*s))
        s++;

    /* C99 §7.20.1.4: optional sign; minus negates in the return type. */
    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if ((base == 0 || base == 16) && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
        base = 16;
    } else if (base == 0) {
        base = (s[0] == '0') ? 8 : 10;
    }

    if (base < 2 || base > 36) {
        if (endptr)
            *endptr = (char *)nptr;
        return 0;
    }

    for (;;) {
        unsigned char c = (unsigned char)*s;
        int digit;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'z')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            digit = c - 'A' + 10;
        else
            break;
        if (digit >= base)
            break;
        any = 1;
        if (!overflow) {
            if (acc > (ULLONG_MAX - (unsigned long long)digit) / (unsigned long long)base) {
                overflow = 1;
                acc = ULLONG_MAX;
            } else {
                acc = acc * (unsigned long long)base + (unsigned long long)digit;
            }
        }
        s++;
    }

    if (endptr)
        *endptr = (char *)(any ? s : nptr);
    return neg ? (unsigned long long)(-(long long)acc) : acc;
}

static inline long long l_strtoll(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    int neg = 0;
    unsigned long long uval;
    char *ep;

    while (l_isspace((unsigned char)*s))
        s++;

    if (*s == '-') {
        neg = 1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    uval = l_strtoull(s, &ep, base);

    if (ep == s)
        ep = (char *)nptr;
    if (endptr)
        *endptr = ep;

    if (neg) {
        if (uval >= (unsigned long long)LLONG_MAX + 1ULL)
            return LLONG_MIN;
        return -(long long)uval;
    }
    if (uval > (unsigned long long)LLONG_MAX)
        return LLONG_MAX;
    return (long long)uval;
}

static inline double l_strtod(const char *nptr, char **endptr)
{
    const char *s = nptr;
    while (l_isspace((unsigned char)*s)) s++;

    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') { s++; }

    /* infinity */
    if ((s[0] == 'i' || s[0] == 'I') &&
        (s[1] == 'n' || s[1] == 'N') &&
        (s[2] == 'f' || s[2] == 'F')) {
        const char *end = s + 3;
        /* consume optional "inity" to match "infinity" (case-insensitive) */
        if ((end[0] == 'i' || end[0] == 'I') &&
            (end[1] == 'n' || end[1] == 'N') &&
            (end[2] == 'i' || end[2] == 'I') &&
            (end[3] == 't' || end[3] == 'T') &&
            (end[4] == 'y' || end[4] == 'Y'))
            end += 5;
        if (endptr) *endptr = (char *)end;
        return neg ? -__builtin_inf() : __builtin_inf();
    }
    /* NaN */
    if ((s[0] == 'n' || s[0] == 'N') &&
        (s[1] == 'a' || s[1] == 'A') &&
        (s[2] == 'n' || s[2] == 'N')) {
        if (endptr) *endptr = (char *)(s + 3);
        return __builtin_nan("");
    }

    double val = 0.0;
    int has_digits = 0;

    /* integer part */
    while (*s >= '0' && *s <= '9') {
        val = val * 10.0 + (double)(*s++ - '0');
        has_digits = 1;
    }

    /* fractional part — a lone "." with no adjacent digits is not a valid number */
    if (*s == '.') {
        s++;
        double frac = 0.1;
        while (*s >= '0' && *s <= '9') {
            val += (double)(*s++ - '0') * frac;
            frac *= 0.1;
            has_digits = 1;
        }
    }

    /* exponent — only valid when at least one digit was consumed */
    if (has_digits && (*s == 'e' || *s == 'E')) {
        const char *es = s + 1;
        int eneg = 0;
        if (*es == '-') { eneg = 1; es++; }
        else if (*es == '+') { es++; }
        if (*es >= '0' && *es <= '9') {
            s = es;
            int exp = 0;
            while (*s >= '0' && *s <= '9')
                exp = exp * 10 + (*s++ - '0');
            if (exp > 308) exp = 308; /* avoid infinite-loop for huge exponents */
            double epow = 1.0;
            for (int i = 0; i < exp; i++) epow *= 10.0;
            if (eneg) val /= epow;
            else      val *= epow;
        }
    }

    if (!has_digits) { if (endptr) *endptr = (char *)nptr; return 0.0; }
    if (endptr) *endptr = (char *)s;
    return neg ? -val : val;
}

static inline double l_atof(const char *s) { return l_strtod(s, (char **)0); }

static inline float l_strtof(const char *nptr, char **endptr)
{
    const char *s = nptr;
    while (l_isspace((unsigned char)*s)) s++;

    int neg = 0;
    if (*s == '-') { neg = 1; s++; }
    else if (*s == '+') { s++; }

    /* infinity */
    if ((s[0] == 'i' || s[0] == 'I') &&
        (s[1] == 'n' || s[1] == 'N') &&
        (s[2] == 'f' || s[2] == 'F')) {
        const char *end = s + 3;
        /* consume optional "inity" to match "infinity" (case-insensitive) */
        if ((end[0] == 'i' || end[0] == 'I') &&
            (end[1] == 'n' || end[1] == 'N') &&
            (end[2] == 'i' || end[2] == 'I') &&
            (end[3] == 't' || end[3] == 'T') &&
            (end[4] == 'y' || end[4] == 'Y'))
            end += 5;
        if (endptr) *endptr = (char *)end;
        return neg ? -__builtin_inff() : __builtin_inff();
    }
    /* NaN */
    if ((s[0] == 'n' || s[0] == 'N') &&
        (s[1] == 'a' || s[1] == 'A') &&
        (s[2] == 'n' || s[2] == 'N')) {
        if (endptr) *endptr = (char *)(s + 3);
        return __builtin_nanf("");
    }

    float val = 0.0f;
    int has_digits = 0;

    /* integer part */
    while (*s >= '0' && *s <= '9') {
        val = val * 10.0f + (float)(*s++ - '0');
        has_digits = 1;
    }

    /* fractional part — a lone "." with no adjacent digits is not a valid number */
    if (*s == '.') {
        s++;
        float frac = 0.1f;
        while (*s >= '0' && *s <= '9') {
            val += (float)(*s++ - '0') * frac;
            frac *= 0.1f;
            has_digits = 1;
        }
    }

    /* exponent — only valid when at least one digit was consumed */
    if (has_digits && (*s == 'e' || *s == 'E')) {
        const char *es = s + 1;
        int eneg = 0;
        if (*es == '-') { eneg = 1; es++; }
        else if (*es == '+') { es++; }
        if (*es >= '0' && *es <= '9') {
            s = es;
            int exp = 0;
            while (*s >= '0' && *s <= '9')
                exp = exp * 10 + (*s++ - '0');
            if (exp > 38) exp = 38; /* clamp to float range */
            float epow = 1.0f;
            for (int i = 0; i < exp; i++) epow *= 10.0f;
            if (eneg) val /= epow;
            else      val *= epow;
        }
    }

    if (!has_digits) { if (endptr) *endptr = (char *)nptr; return 0.0f; }
    if (endptr) *endptr = (char *)s;
    return neg ? -val : val;
}

// ─── Math functions ──────────────────────────────────────────────────────────

static inline double l_fabs(double x) {
    union { double d; unsigned char b[8]; } u;
    u.d = x;
    u.b[7] &= 0x7F;  // clear sign bit (IEEE 754 big-endian byte 7)
    return u.d;
}

static inline double l_floor(double x) {
    // Handle special cases: zero and very large values are already integral
    if (x == 0.0 || x != x) return x; // 0 or NaN
    union { double d; unsigned char b[8]; } u;
    u.d = x;
    int exponent = (int)(((((unsigned)u.b[7] & 0x7F) << 4) | (((unsigned)u.b[6] >> 4) & 0xF)) - 1023);
    if (exponent >= 52) return x; // already integral or inf
    if (exponent < 0) return (x < 0.0) ? -1.0 : 0.0;
    // Truncate toward zero
    double t = (double)(long long)x;
    if (t == x) return x;
    if (x < 0.0) return t - 1.0;
    return t;
}

static inline double l_ceil(double x) {
    double f = l_floor(x);
    if (f == x) return x;
    return f + 1.0;
}

static inline double l_fmod(double x, double y) {
    if (y == 0.0) return 0.0 / 0.0; // NaN
    double q = x / y;
    double t = (double)(long long)q; // truncate toward zero
    return x - t * y;
}

static inline double l_sqrt(double x) {
    if (x < 0.0) return 0.0 / 0.0; // NaN
    if (x == 0.0 || x != x) return x;

    // IEEE 754 bit hack for initial guess
    union { double d; unsigned char b[8]; } u;
    u.d = x;
    // Halve the exponent bits for a rough sqrt estimate
    unsigned hi = ((unsigned)u.b[7] << 24) | ((unsigned)u.b[6] << 16) |
                  ((unsigned)u.b[5] << 8) | (unsigned)u.b[4];
    hi = (hi >> 1) + 0x1FF80000u;
    u.b[7] = (unsigned char)(hi >> 24);
    u.b[6] = (unsigned char)(hi >> 16);
    u.b[5] = (unsigned char)(hi >> 8);
    u.b[4] = (unsigned char)hi;
    double guess = u.d;

    // Newton-Raphson iterations
    for (int i = 0; i < 8; i++)
        guess = 0.5 * (guess + x / guess);
    return guess;
}

static inline double l_sin(double x) {
    // Range reduce to [-pi, pi]
    double pi2 = 2.0 * L_PI;
    x = l_fmod(x, pi2);
    if (x > L_PI) x -= pi2;
    if (x < -L_PI) x += pi2;

    // Further reduce to [-pi/2, pi/2] using sin(x) = sin(pi - x)
    if (x > L_PI_2)       x = L_PI - x;
    else if (x < -L_PI_2) x = -L_PI - x;

    // Taylor series: sin(x) = x - x^3/3! + x^5/5! - ...
    // Converges fast for |x| <= pi/2
    double x2 = x * x;
    double term = x;
    double sum = x;
    for (int i = 1; i <= 12; i++) {
        term *= -x2 / (double)(2 * i * (2 * i + 1));
        sum += term;
    }
    return sum;
}

static inline double l_cos(double x) {
    return l_sin(x + L_PI_2);
}

static inline double l_exp(double x) {
    if (x == 0.0) return 1.0;
    // Clamp to avoid overflow/underflow
    if (x > 709.0) return 1.0 / 0.0;   // +inf
    if (x < -709.0) return 0.0;

    // Range reduction: exp(x) = 2^k * exp(r), where r = x - k*ln2
    int k = (int)(x / L_LN2 + (x >= 0.0 ? 0.5 : -0.5));
    double r = x - (double)k * L_LN2;

    // Taylor series for exp(r) where |r| <= ln2/2
    double sum = 1.0;
    double term = 1.0;
    for (int i = 1; i <= 20; i++) {
        term *= r / (double)i;
        sum += term;
        if (l_fabs(term) < 1e-15 * l_fabs(sum)) break;
    }

    // Multiply by 2^k using bit manipulation
    union { double d; unsigned char b[8]; } u;
    u.d = 1.0;
    unsigned hi = ((unsigned)u.b[7] << 24) | ((unsigned)u.b[6] << 16) |
                  ((unsigned)u.b[5] << 8) | (unsigned)u.b[4];
    hi += (unsigned)k << 20;
    u.b[7] = (unsigned char)(hi >> 24);
    u.b[6] = (unsigned char)(hi >> 16);
    u.b[5] = (unsigned char)(hi >> 8);
    u.b[4] = (unsigned char)hi;
    return sum * u.d;
}

static inline double l_log(double x) {
    if (x <= 0.0) {
        if (x == 0.0) return -1.0 / 0.0; // -inf
        return 0.0 / 0.0;                 // NaN
    }
    // Decompose: x = m * 2^e where 1 <= m < 2
    union { double d; unsigned char b[8]; } u;
    u.d = x;
    unsigned hi = ((unsigned)u.b[7] << 24) | ((unsigned)u.b[6] << 16) |
                  ((unsigned)u.b[5] << 8) | (unsigned)u.b[4];
    int e = (int)((hi >> 20) & 0x7FF) - 1023;
    // Set exponent to 0 so m is in [1, 2)
    hi = (hi & 0x800FFFFFu) | 0x3FF00000u;
    u.b[7] = (unsigned char)(hi >> 24);
    u.b[6] = (unsigned char)(hi >> 16);
    u.b[5] = (unsigned char)(hi >> 8);
    u.b[4] = (unsigned char)hi;
    double m = u.d;

    // log(m * 2^e) = log(m) + e * ln2
    // Use series: log(m) = 2 * atanh((m-1)/(m+1))
    double f = (m - 1.0) / (m + 1.0);
    double f2 = f * f;
    double sum = f;
    double term = f;
    for (int i = 1; i <= 20; i++) {
        term *= f2;
        sum += term / (double)(2 * i + 1);
    }
    return 2.0 * sum + (double)e * L_LN2;
}

static inline double l_pow(double base, double exponent) {
    if (exponent == 0.0) return 1.0;
    if (base == 0.0) return 0.0;
    if (base == 1.0) return 1.0;

    // Integer exponent fast path
    if (exponent == (double)(long long)exponent && l_fabs(exponent) < 1000.0) {
        long long n = (long long)exponent;
        int neg = 0;
        if (n < 0) { neg = 1; n = -n; }
        double result = 1.0;
        double b = base;
        while (n > 0) {
            if (n & 1) result *= b;
            b *= b;
            n >>= 1;
        }
        return neg ? 1.0 / result : result;
    }

    if (base < 0.0) return 0.0 / 0.0; // NaN for non-integer exp with negative base
    return l_exp(exponent * l_log(base));
}

static inline double l_atan2(double y, double x) {
    if (x == 0.0 && y == 0.0) return 0.0;
    if (x == 0.0) return (y > 0.0) ? L_PI_2 : -L_PI_2;

    double ax = l_fabs(x), ay = l_fabs(y);

    // Compute atan(ay/ax), result in [0, pi/2]
    double s;
    int region; // 0: s<=tan(pi/8), 1: s<=1, 2: s>1
    if (ay <= ax) {
        s = ay / ax;
        if (s > 0.4142135623730950) {
            // atan(s) = pi/4 + atan((s-1)/(s+1))
            s = (s - 1.0) / (s + 1.0);
            region = 1;
        } else {
            region = 0;
        }
    } else {
        // atan(ay/ax) = pi/2 - atan(ax/ay)
        s = ax / ay;
        if (s > 0.4142135623730950) {
            s = (s - 1.0) / (s + 1.0);
            region = 1;
        } else {
            region = 0;
        }
    }

    // Taylor series for atan(s), |s| < 0.42, converges fast
    double s2 = s * s;
    double r = s;
    double term = s;
    for (int i = 1; i <= 12; i++) {
        term *= -s2;
        r += term / (double)(2 * i + 1);
    }

    // Undo region reduction
    if (ay <= ax) {
        if (region == 1) r += L_PI / 4.0;
        // region 0: r is already atan(ay/ax)
    } else {
        if (region == 1) r += L_PI / 4.0;
        r = L_PI_2 - r;
    }

    // Apply quadrant
    if (x < 0.0) r = L_PI - r;
    if (y < 0.0) r = -r;
    return r;
}

static inline double l_tan(double x) {
    return l_sin(x) / l_cos(x);
}

static inline double l_asin(double x) {
    if (x != x) return x; // NaN
    if (x < -1.0 || x > 1.0) return 0.0 / 0.0; // NaN for out of range
    if (x == 1.0) return L_PI_2;
    if (x == -1.0) return -L_PI_2;
    // Newton's method: find t such that sin(t) = x, starting from t = x
    double t = x;
    for (int i = 0; i < 50; i++) {
        double st = l_sin(t);
        double ct = l_cos(t);
        if (l_fabs(ct) < 1e-15) break;
        double dt = (st - x) / ct;
        t -= dt;
        if (l_fabs(dt) < 1e-15) break;
    }
    return t;
}

static inline double l_acos(double x) {
    return L_PI_2 - l_asin(x);
}

static inline double l_atan(double x) {
    return l_asin(x / l_sqrt(1.0 + x * x));
}

static inline double l_log10(double x) {
    return l_log(x) / 2.302585092994046; // log(10)
}

static inline double l_log2(double x) {
    return l_log(x) / 0.6931471805599453; // log(2) = L_LN2
}

static inline double l_round(double x) {
    if (x >= 0.0) return l_floor(x + 0.5);
    return l_ceil(x - 0.5);
}

static inline double l_trunc(double x) {
    if (x != x) return x; // NaN
    if (x == 0.0) return x;
    return (double)(long long)x;
}

static inline double l_hypot(double x, double y) {
    x = l_fabs(x);
    y = l_fabs(y);
    if (x == 0.0) return y;
    if (y == 0.0) return x;
    double mx = (x > y) ? x : y;
    double mn = (x > y) ? y : x;
    double r = mn / mx;
    return mx * l_sqrt(1.0 + r * r);
}

//function to reverse a string
static inline void l_reverse(char str[], int length)
{
    int start;
    int end = length -1;
    for(start = 0; start < end; ++start, --end)
    {
        const char ch = str[start];
        str[start] = str[end];
        str[end] = ch;
    }
}

static inline char* l_itoa(int num, char* str, int base)
{
    int i = 0;
    char isNegNum = 0;
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
    }
    else
    {
        // Use unsigned arithmetic to handle INT_MIN without overflow
        unsigned int unum;
        if ((num < 0) && (base == 10))
        {
            isNegNum = 1;
            unum = (unsigned int)(-(num + 1)) + 1u;
        }
        else
        {
            unum = (unsigned int)num;
        }
        do
        {
            const unsigned int rem = unum % (unsigned int)base;
            str[i++] = (rem > 9) ? ((int)(rem - 10) + 'a') : ((int)rem + '0');
            unum = unum / (unsigned int)base;
        }
        while (unum != 0);
        if (isNegNum)
        {
            str[i++] = '-';
        }
        str[i] = '\0';
        l_reverse(str, i);
    }
    return str;
}

static inline void *l_memcpy(void *dst, const void *src, size_t len)
{
    char *d = (char *)dst;
    const char *s = (const char *)src;

    /* Byte-copy until dst is word-aligned. */
    while (len && ((uintptr_t)d & (sizeof(uintptr_t) - 1U))) {
        *d++ = *s++;
        len--;
    }

    /* Word-at-a-time copy when src is also word-aligned.
     * The may_alias attribute lets the compiler know these pointers may alias
     * char *, matching how freestanding memcpy implementations work. */
    if (len >= sizeof(uintptr_t) && !((uintptr_t)s & (sizeof(uintptr_t) - 1U))) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        uptr_alias       *dp = (uptr_alias *)(void *)d;
        const uptr_alias *sp = (const uptr_alias *)(const void *)s;
        size_t nw = len / sizeof(uptr_alias);
        while (nw--)
            *dp++ = *sp++;
        d   = (char *)(void *)dp;
        s   = (const char *)(const void *)sp;
        len &= sizeof(uptr_alias) - 1U;
    }

    /* Remaining bytes (or full copy when pointers have different alignment). */
    while (len--)
        *d++ = *s++;
    return dst;
}

// Shell sort — in-place, no recursion, no malloc, stack-safe
static inline void l_qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {
    if (nmemb < 2) return;
    char *b = (char *)base;

    // Stack buffer for the insertion-sort temp; fall back to byte-by-byte
    // swap when the element is too large (avoids heap allocation).
    char tmp[256];
    int use_tmp = (size <= sizeof(tmp));

    size_t gap = 1;
    while (gap < nmemb / 3) gap = gap * 3 + 1;

    for (; gap > 0; gap = (gap - 1) / 3) {
        for (size_t i = gap; i < nmemb; i++) {
            if (use_tmp) {
                l_memcpy(tmp, b + i * size, size);
                size_t j = i;
                while (j >= gap && cmp(b + (j - gap) * size, tmp) > 0) {
                    l_memcpy(b + j * size, b + (j - gap) * size, size);
                    j -= gap;
                }
                l_memcpy(b + j * size, tmp, size);
            } else {
                // Byte-by-byte swap path for large elements
                size_t j = i;
                while (j >= gap && cmp(b + (j - gap) * size, b + j * size) > 0) {
                    char *a_ptr = b + (j - gap) * size;
                    char *b_ptr = b + j * size;
                    for (size_t k = 0; k < size; k++) {
                        char t = a_ptr[k];
                        a_ptr[k] = b_ptr[k];
                        b_ptr[k] = t;
                    }
                    j -= gap;
                }
            }
        }
    }
}

// Binary search in a sorted array
static inline void *l_bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void *)) {
    const char *b = (const char *)base;
    size_t lo = 0, hi = nmemb;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2;
        int c = cmp(key, b + mid * size);
        if (c == 0) return (void *)(b + mid * size);
        if (c < 0) hi = mid;
        else lo = mid + 1;
    }
    return (void *)0;
}

static inline void *l_memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s;
    const unsigned char uc = (unsigned char)c;

    /* Align to word boundary one byte at a time. */
    while (n && ((uintptr_t)p & (sizeof(uintptr_t) - 1U))) {
        if (*p == uc)
            return (void *)p;
        p++;
        n--;
    }

    /* Word-at-a-time search: replicate target byte into every byte of a word,
     * XOR each buffer word with it, then use the Hacker's Delight has-zero-byte
     * trick to detect a match.  (x ^ repeated == 0 iff x == target byte.) */
    if (n >= sizeof(uintptr_t)) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;
        const uintptr_t highs = lones << 7;
        uintptr_t repeated = uc;
        repeated |= repeated << 8;
        repeated |= repeated << 16;
#if UINTPTR_MAX > 0xFFFFFFFFU
        repeated |= repeated << 32;
#endif
        const uptr_alias *wp = (const uptr_alias *)(const void *)p;
        while (n >= sizeof(uintptr_t)) {
            uintptr_t xw = *wp ^ repeated;
            if ((xw - lones) & ~xw & highs)
                break;
            wp++;
            n -= sizeof(uintptr_t);
        }
        p = (const unsigned char *)(const void *)wp;
    }

    /* Tail scan (or scan within the word that triggered). */
    while (n--) {
        if (*p == uc)
            return (void *)p;
        p++;
    }
    return NULL;
}

static inline void *l_memrchr(const void *s, int c, size_t n)
{
    const unsigned char *p   = (const unsigned char *)s + n;
    const unsigned char  uc  = (unsigned char)c;

    /* Invariant: n == p - s at all times (both decrease together). */

    /* Align p backward to a word boundary one byte at a time. */
    while (n && ((uintptr_t)p & (sizeof(uintptr_t) - 1U))) {
        p--; n--;
        if (*p == uc) return (void *)p;
    }

    /* Word-at-a-time backward scan.  p is word-aligned; the range left
     * to scan is [s, p) == n bytes.  Each iteration reads the word just
     * below p using the Hacker's Delight has-zero-byte test on the XOR
     * with the broadcast byte.  A positive detection (which may be a
     * false positive) triggers a right-to-left byte scan of that word. */
    if (n >= sizeof(uintptr_t)) {
        typedef uintptr_t __attribute__((may_alias)) uptr_alias;
        const uintptr_t lones = (uintptr_t)(-1) / 0xFFu;
        const uintptr_t highs = lones << 7;
        uintptr_t repeated = uc;
        repeated |= repeated << 8;
        repeated |= repeated << 16;
#if UINTPTR_MAX > 0xFFFFFFFFU
        repeated |= repeated << 32;
#endif
        while (n >= sizeof(uintptr_t)) {
            p -= sizeof(uintptr_t);
            n -= sizeof(uintptr_t);
            uintptr_t xw = *(const uptr_alias *)(const void *)p ^ repeated;
            if ((xw - lones) & ~xw & highs) {
                /* Scan byte-by-byte from right to find the last match. */
                size_t i = sizeof(uintptr_t);
                while (i--)
                    if (p[i] == uc) return (void *)(p + i);
                /* False positive — continue to the next word. */
            }
        }
    }

    /* Tail: at most sizeof(uintptr_t) - 1 bytes remain before p. */
    while (n--) {
        p--;
        if (*p == uc) return (void *)p;
    }
    return NULL;
}

static inline size_t l_strnlen(const char *s, size_t maxlen)
{
    const char *end = (const char *)l_memchr(s, '\0', maxlen);
    return end ? (size_t)(end - s) : maxlen;
}

static inline void *l_memmem(const void *haystack, size_t haystacklen,
                      const void *needle,   size_t needlelen)
{
    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;

    if (needlelen == 0)
        return (void *)haystack;
    if (needlelen > haystacklen)
        return NULL;

    /* Short needles: first-byte scan with l_memchr + l_memcmp. */
    if (needlelen < 8) {
        const unsigned char *end   = h + haystacklen - needlelen;
        const unsigned char  first = n[0];
        while (h <= end) {
            h = (const unsigned char *)l_memchr(h, first, (size_t)(end - h + 1));
            if (!h)
                return NULL;
            if (l_memcmp(h, n, needlelen) == 0)
                return (void *)h;
            h++;
        }
        return NULL;
    }

    /* Boyer-Moore-Horspool for needles >= 8 bytes.
       skip[c] = safe advance when c appears at the window's last position. */
    size_t skip[256];
    size_t i;
    for (i = 0; i < 256; i++)
        skip[i] = needlelen;
    for (i = 0; i < needlelen - 1; i++)
        skip[n[i]] = needlelen - 1 - i;

    size_t pos  = 0;
    size_t last = needlelen - 1;
    while (pos + needlelen <= haystacklen) {
        if (h[pos + last] == n[last] && l_memcmp(h + pos, n, needlelen) == 0)
            return (void *)(h + pos);
        pos += skip[h[pos + last]];
    }
    return NULL;
}

#ifdef L_WITHSNPRINTF
/* 64-bit divmod without hardware division — avoids __aeabi_uldivmod
   and __aeabi_uidiv on 32-bit ARM. Bit-by-bit long division. */
static inline unsigned long long l__divmod64(unsigned long long val, unsigned int base, unsigned int *rem)
{
    unsigned long long q = 0;
    unsigned int r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | (unsigned int)((val >> i) & 1);
        if (r >= base) { r -= base; q |= (1ULL << i); }
    }
    *rem = r;
    return q;
}

/* Format a non-negative finite double into buf[0..bufsz-1] (NUL-terminated).
   use_e=0: fixed  "%f" style,  prec digits after decimal point.
   use_e=1: scientific "%e" style, prec digits after decimal point.
   strip_zeros: remove trailing '0' (and lone '.') from fractional part.
   upper: use 'E' instead of 'e'.
   Returns number of characters written (excluding NUL). */
static inline int l__fmt_double(char *buf, int bufsz, double val,
                                int use_e, int prec, int strip_zeros, int upper)
{
    int pos = 0;
#define L_FMT_EMIT(c) do { if (pos < bufsz - 1) buf[pos] = (char)(c); pos++; } while(0)

    int exp10 = 0;

    if (use_e) {
        double mant = val;
        /* Normalise mantissa to [1.0, 10.0) and record exponent */
        if (mant > 0.0) {
            while (mant >= 10.0) { mant /= 10.0; exp10++; }
            while (mant < 1.0)   { mant *= 10.0; exp10--; }
        }
        /* Round mantissa at prec decimal place */
        double rounder = 0.5;
        for (int i = 0; i < prec; i++) rounder *= 0.1;
        mant += rounder;
        if (mant >= 10.0) { mant /= 10.0; exp10++; } /* carry */

        /* Integer digit */
        int d = (int)mant;
        if (d > 9) d = 9;
        L_FMT_EMIT('0' + d);
        mant -= (double)d;

        /* Fractional digits */
        int fstart = pos + 1; /* position after the '.' for strip_zeros */
        if (prec > 0) {
            L_FMT_EMIT('.');
            fstart = pos;
            for (int i = 0; i < prec; i++) {
                mant *= 10.0;
                d = (int)mant;
                if (d > 9) d = 9;
                L_FMT_EMIT('0' + d);
                mant -= (double)d;
            }
            if (strip_zeros) {
                while (pos > fstart && buf[pos - 1] == '0') pos--;
                if (pos > 0 && buf[pos - 1] == '.') pos--;
            }
        }

        /* Exponent */
        L_FMT_EMIT(upper ? 'E' : 'e');
        L_FMT_EMIT(exp10 >= 0 ? '+' : '-');
        int ae = exp10 >= 0 ? exp10 : -exp10;
        if (ae >= 100) { L_FMT_EMIT('0' + ae / 100); ae %= 100; }
        L_FMT_EMIT('0' + ae / 10);
        L_FMT_EMIT('0' + ae % 10);

    } else {
        /* Fixed-point: round, then split integer / fractional */
        double rounder = 0.5;
        for (int i = 0; i < prec; i++) rounder *= 0.1;
        val += rounder;

        /* Integer part — guard against overflow into ULL */
        unsigned long long ipart = (val < 1.8446744073709552e19)
                                   ? (unsigned long long)val
                                   : 18446744073709551615ULL;
        double fpart = val - (double)ipart;

        char ibuf[24]; int ilen = 0;
        if (ipart == 0ULL) {
            ibuf[ilen++] = '0';
        } else {
            unsigned long long v = ipart;
            while (v) {
                unsigned int dig;
                v = l__divmod64(v, 10U, &dig);
                ibuf[ilen++] = (char)('0' + dig);
            }
            for (int a = 0, b = ilen - 1; a < b; a++, b--) {
                char t = ibuf[a]; ibuf[a] = ibuf[b]; ibuf[b] = t;
            }
        }
        for (int i = 0; i < ilen; i++) L_FMT_EMIT(ibuf[i]);

        /* Fractional digits */
        if (prec > 0) {
            L_FMT_EMIT('.');
            int fstart = pos;
            for (int i = 0; i < prec; i++) {
                fpart *= 10.0;
                int d = (int)fpart;
                if (d > 9) d = 9;
                L_FMT_EMIT('0' + d);
                fpart -= (double)d;
            }
            if (strip_zeros) {
                while (pos > fstart && buf[pos - 1] == '0') pos--;
                if (pos > 0 && buf[pos - 1] == '.') pos--;
            }
        }
    }

    if (pos < bufsz) buf[pos] = '\0';
    return pos;
#undef L_FMT_EMIT
}

static inline int l_vsnprintf(char *buf, size_t n, const char *fmt, va_list ap)
{
    size_t pos = 0;
    int total = 0;

#define L_SNPRINTF_EMIT(c) do { \
    if (pos + 1 < n) buf[pos] = (char)(c); \
    pos++; total++; \
} while(0)

    while (*fmt) {
        if (*fmt != '%') {
            unsigned char ch = (unsigned char)*fmt++;
            L_SNPRINTF_EMIT(ch);
            continue;
        }
        fmt++;

        int flag_minus = 0, flag_zero = 0, flag_plus = 0, flag_space = 0;
        for (;;) {
            if      (*fmt == '-') { flag_minus = 1; fmt++; }
            else if (*fmt == '0') { flag_zero  = 1; fmt++; }
            else if (*fmt == '+') { flag_plus  = 1; fmt++; }
            else if (*fmt == ' ') { flag_space = 1; fmt++; }
            else break;
        }

        int width = 0;
        if (*fmt == '*') { fmt++; width = va_arg(ap, int); if (width < 0) { flag_minus = 1; width = -width; } }
        else while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');

        int prec = -1;
        if (*fmt == '.') {
            fmt++; prec = 0;
            if (*fmt == '*') { fmt++; prec = va_arg(ap, int); if (prec < 0) prec = -1; }
            else while (*fmt >= '0' && *fmt <= '9') prec = prec * 10 + (*fmt++ - '0');
        }

        int is_long = 0, is_ll = 0;
        if (*fmt == 'l') {
            is_long = 1; fmt++;
            if (*fmt == 'l') { is_ll = 1; fmt++; }
        } else if (*fmt == 'h') {
            fmt++; if (*fmt == 'h') fmt++;
        } else if (*fmt == 'z' || *fmt == 't') {
            is_long = (sizeof(size_t) > sizeof(int)); fmt++;
        }

        char spec = *fmt++;
        if (spec == '\0') break;

        if (spec == '%') { L_SNPRINTF_EMIT('%'); continue; }

        if (spec == 'c') {
            int cv = va_arg(ap, int);
            int pad = width - 1; if (pad < 0) pad = 0;
            if (!flag_minus) for (int i = 0; i < pad; i++) L_SNPRINTF_EMIT(' ');
            L_SNPRINTF_EMIT(cv);
            if ( flag_minus) for (int i = 0; i < pad; i++) L_SNPRINTF_EMIT(' ');
            continue;
        }

        if (spec == 's') {
            const char *sv = va_arg(ap, const char *);
            if (!sv) sv = "(null)";
            /* C11 §7.21.6.1: when precision is given, the string need not be
             * null-terminated — only 'prec' bytes are examined.  Calling
             * l_strlen on such a pointer reads past the end of the buffer. */
            int slen = (prec >= 0) ? (int)l_strnlen(sv, (size_t)prec)
                                   : (int)l_strlen(sv);
            int pad = width - slen; if (pad < 0) pad = 0;
            /* Bulk helpers: avail = bytes writable before the reserved NUL slot.
             * n==0 guard prevents size_t underflow on (n-1). */
#define L_SNPRINTF_AVAIL() \
    ((n > 0 && pos + 1u < n) ? (n - 1u - pos) : (size_t)0)
            if (!flag_minus) {
                size_t av = L_SNPRINTF_AVAIL();
                size_t cp = (size_t)pad < av ? (size_t)pad : av;
                if (cp) l_memset(buf + pos, ' ', cp);
                pos += (size_t)pad; total += pad;
            }
            {
                size_t av = L_SNPRINTF_AVAIL();
                size_t cp = (size_t)slen < av ? (size_t)slen : av;
                if (cp) l_memcpy(buf + pos, sv, cp);
                pos += (size_t)slen; total += slen;
            }
            if (flag_minus) {
                size_t av = L_SNPRINTF_AVAIL();
                size_t cp = (size_t)pad < av ? (size_t)pad : av;
                if (cp) l_memset(buf + pos, ' ', cp);
                pos += (size_t)pad; total += pad;
            }
#undef L_SNPRINTF_AVAIL
            continue;
        }

        /* Numeric specifiers: d i u x X p */
        unsigned long long uval = 0;
        int neg = 0, base = 10, upper = 0, is_ptr = 0;

        if (spec == 'd' || spec == 'i') {
            long long sv;
            if (is_ll)        sv = va_arg(ap, long long);
            else if (is_long) sv = va_arg(ap, long);
            else               sv = va_arg(ap, int);
            if (sv < 0) {
                neg = 1;
                uval = (sv == (long long)(-9223372036854775807LL - 1))
                    ? (unsigned long long)9223372036854775808ULL
                    : (unsigned long long)(-sv);
            } else {
                uval = (unsigned long long)sv;
            }
        } else if (spec == 'u') {
            if (is_ll)        uval = va_arg(ap, unsigned long long);
            else if (is_long) uval = va_arg(ap, unsigned long);
            else               uval = (unsigned int)va_arg(ap, unsigned int);
        } else if (spec == 'x' || spec == 'X') {
            base = 16; upper = (spec == 'X');
            if (is_ll)        uval = va_arg(ap, unsigned long long);
            else if (is_long) uval = va_arg(ap, unsigned long);
            else               uval = (unsigned int)va_arg(ap, unsigned int);
        } else if (spec == 'p') {
            base = 16; is_ptr = 1;
            uval = (unsigned long long)(uintptr_t)va_arg(ap, void *);
        } else if (spec == 'f' || spec == 'F' ||
                   spec == 'e' || spec == 'E' ||
                   spec == 'g' || spec == 'G') {
            double dval = va_arg(ap, double);
            char lspec = (spec >= 'A' && spec <= 'Z') ? (char)(spec - 'A' + 'a') : spec;
            int upper_f = (spec != lspec);
            if (prec < 0) prec = 6;
            if (prec == 0 && lspec == 'g') prec = 1;

            /* sign */
            int dneg = 0;
            if (dval < 0.0) { dneg = 1; dval = -dval; }

            /* NaN */
            if (dval != dval) {
                const char *ns = upper_f ? "NAN" : "nan";
                int fpad = width - 3; if (fpad < 0) fpad = 0;
                if (!flag_minus) for (int i = 0; i < fpad; i++) L_SNPRINTF_EMIT(' ');
                L_SNPRINTF_EMIT(ns[0]); L_SNPRINTF_EMIT(ns[1]); L_SNPRINTF_EMIT(ns[2]);
                if (flag_minus) for (int i = 0; i < fpad; i++) L_SNPRINTF_EMIT(' ');
                continue;
            }

            /* Inf: any finite double <= DBL_MAX, so > DBL_MAX means infinity */
            if (dval > DBL_MAX) {
                const char *is2 = upper_f ? "INF" : "inf";
                int fcont = (dneg ? 1 : 0) + 3;
                int fpad = width - fcont; if (fpad < 0) fpad = 0;
                char fpc = (!flag_minus && flag_zero) ? '0' : ' ';
                if (!flag_minus && fpc == ' ') for (int i = 0; i < fpad; i++) L_SNPRINTF_EMIT(' ');
                if (dneg) L_SNPRINTF_EMIT('-');
                if (!flag_minus && fpc == '0') for (int i = 0; i < fpad; i++) L_SNPRINTF_EMIT('0');
                L_SNPRINTF_EMIT(is2[0]); L_SNPRINTF_EMIT(is2[1]); L_SNPRINTF_EMIT(is2[2]);
                if (flag_minus) for (int i = 0; i < fpad; i++) L_SNPRINTF_EMIT(' ');
                continue;
            }

            /* Determine format for %g/%G */
            int use_e_fmt = (lspec == 'e');
            int strip_z = 0;
            int fprec = prec;
            if (lspec == 'g') {
                /* exp10 = floor(log10(dval)) */
                int exp10 = 0;
                if (dval > 0.0) {
                    double tmp = dval;
                    while (tmp >= 10.0) { tmp /= 10.0; exp10++; }
                    while (tmp < 1.0)   { tmp *= 10.0; exp10--; }
                }
                use_e_fmt = (exp10 < -4 || exp10 >= prec);
                strip_z = 1;
                if (use_e_fmt) {
                    fprec = prec - 1;
                } else {
                    fprec = prec - (exp10 + 1);
                    if (fprec < 0) fprec = 0;
                }
            }

            char fbuf[64];
            int flen = l__fmt_double(fbuf, (int)sizeof(fbuf), dval,
                                     use_e_fmt, fprec, strip_z, upper_f);
            int fcont2 = (dneg ? 1 : 0) + flen;
            int fpad2 = width - fcont2; if (fpad2 < 0) fpad2 = 0;
            char fpc2 = (!flag_minus && flag_zero) ? '0' : ' ';
            if (!flag_minus && fpc2 == ' ') for (int i = 0; i < fpad2; i++) L_SNPRINTF_EMIT(' ');
            if (dneg) L_SNPRINTF_EMIT('-');
            if (!flag_minus && fpc2 == '0') for (int i = 0; i < fpad2; i++) L_SNPRINTF_EMIT('0');
            for (int i = 0; i < flen; i++) L_SNPRINTF_EMIT(fbuf[i]);
            if (flag_minus) for (int i = 0; i < fpad2; i++) L_SNPRINTF_EMIT(' ');
            continue;
        } else {
            L_SNPRINTF_EMIT(spec); continue;
        }

        /* Convert to digit chars (reverse order first) */
        char nbuf[24]; int nlen = 0;
        if (uval == 0 && prec != 0) {
            nbuf[nlen++] = '0';
        } else {
            unsigned long long v = uval;
            while (v) {
                unsigned int digit;
                v = l__divmod64(v, (unsigned int)base, &digit);
                if (digit < 10) nbuf[nlen++] = (char)('0' + digit);
                else if (upper) nbuf[nlen++] = (char)('A' + digit - 10);
                else            nbuf[nlen++] = (char)('a' + digit - 10);
            }
            for (int a = 0, b = nlen - 1; a < b; a++, b--) {
                char tmp = nbuf[a]; nbuf[a] = nbuf[b]; nbuf[b] = tmp;
            }
        }

        /* prefix: "-", "+", " ", or "0x" for pointers */
        const char *prefix = ""; int preflen = 0;
        if (neg)                   { prefix = "-";  preflen = 1; }
        else if (flag_plus  && (spec == 'd' || spec == 'i'))
                                   { prefix = "+";  preflen = 1; }
        else if (flag_space && (spec == 'd' || spec == 'i'))
                                   { prefix = " ";  preflen = 1; }
        if (is_ptr)                { prefix = "0x"; preflen = 2; }

        /* precision zero-padding (separate from flag_zero) */
        int prec_pad = (prec > nlen) ? prec - nlen : 0;

        /* total content width */
        int content = preflen + prec_pad + nlen;
        int pad = width - content; if (pad < 0) pad = 0;
        char padchar = (!flag_minus && flag_zero && prec < 0) ? '0' : ' ';

        if (!flag_minus && padchar == ' ') for (int i=0; i<pad; i++) L_SNPRINTF_EMIT(' ');
        for (int i = 0; i < preflen; i++) L_SNPRINTF_EMIT(prefix[i]);
        if (!flag_minus && padchar == '0') for (int i=0; i<pad; i++) L_SNPRINTF_EMIT('0');
        for (int i = 0; i < prec_pad; i++) L_SNPRINTF_EMIT('0');
        for (int i = 0; i < nlen; i++) L_SNPRINTF_EMIT(nbuf[i]);
        if ( flag_minus)                   for (int i=0; i<pad; i++) L_SNPRINTF_EMIT(' ');
    }

    if (n > 0) buf[pos < n ? pos : n - 1] = '\0';
    return total;
#undef L_SNPRINTF_EMIT
}

static inline int l_snprintf(char *buf, size_t n, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int ret = l_vsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return ret;
}
#endif // L_WITHSNPRINTF

static inline int l_strcasecmp(const char *s1, const char *s2) {
    for (;;) {
        unsigned char c1 = (unsigned char)l_tolower((unsigned char)*s1);
        unsigned char c2 = (unsigned char)l_tolower((unsigned char)*s2);
        if (c1 != c2)
            return (int)c1 - (int)c2;
        if (c1 == '\0')
            return 0;
        s1++;
        s2++;
    }
}

static inline int l_strncasecmp(const char *s1, const char *s2, size_t n) {
    while (n-- > 0) {
        unsigned char c1 = (unsigned char)l_tolower((unsigned char)*s1);
        unsigned char c2 = (unsigned char)l_tolower((unsigned char)*s2);
        if (c1 != c2)
            return (int)c1 - (int)c2;
        if (c1 == '\0')
            return 0;
        s1++;
        s2++;
    }
    return 0;
}

static inline size_t l_strspn(const char *s, const char *accept) {
    /* 256-bit bitmap (32 bytes): build once from accept, then scan s in O(n+m). */
    unsigned char tbl[32];
    const char *a;
    l_memset(tbl, 0, sizeof(tbl));
    for (a = accept; *a; a++) {
        unsigned char c = (unsigned char)*a;
        tbl[c >> 3] |= (unsigned char)(1u << (c & 7));
    }
    const char *p = s;
    while (*p) {
        unsigned char c = (unsigned char)*p;
        if (!(tbl[c >> 3] & (unsigned char)(1u << (c & 7)))) break;
        p++;
    }
    return (size_t)(p - s);
}

static inline size_t l_strcspn(const char *s, const char *reject) {
    /* 256-bit bitmap: build from reject; NUL always terminates (bit 0 stays 0). */
    unsigned char tbl[32];
    const char *r;
    l_memset(tbl, 0, sizeof(tbl));
    for (r = reject; *r; r++) {
        unsigned char c = (unsigned char)*r;
        tbl[c >> 3] |= (unsigned char)(1u << (c & 7));
    }
    const char *p = s;
    while (*p) {
        unsigned char c = (unsigned char)*p;
        if (tbl[c >> 3] & (unsigned char)(1u << (c & 7))) break;
        p++;
    }
    return (size_t)(p - s);
}

static inline char *l_strpbrk(const char *s, const char *accept) {
    /* 256-bit bitmap: build from accept; find first s char in the set. */
    unsigned char tbl[32];
    const char *a;
    l_memset(tbl, 0, sizeof(tbl));
    for (a = accept; *a; a++) {
        unsigned char c = (unsigned char)*a;
        tbl[c >> 3] |= (unsigned char)(1u << (c & 7));
    }
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (tbl[c >> 3] & (unsigned char)(1u << (c & 7))) return (char *)s;
    }
    return (char *)0;
}

static inline char *l_strtok_r(char *str, const char *delim, char **saveptr) {
    char *p;
    if (str == (char *)0) str = *saveptr;
    str += l_strspn(str, delim);
    if (*str == '\0') { *saveptr = str; return (char *)0; }
    p = str + l_strcspn(str, delim);
    if (*p != '\0') {
        *p = '\0';
        *saveptr = p + 1;
    } else {
        *saveptr = p;
    }
    return str;
}

static inline char *l_strsep(char **stringp, const char *delim) {
    char *s = *stringp;
    if (s == (char *)0) return (char *)0;
    char *tok = s;
    char *p = l_strpbrk(s, delim);
    if (p) {
        *p = '\0';
        *stringp = p + 1;
    } else {
        *stringp = (char *)0;
    }
    return tok;
}

static inline int l_bin2hex(char *dst, const void *src, size_t len) {
    static const char hex[] = "0123456789abcdef";
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < len; i++) {
        dst[i * 2]     = hex[s[i] >> 4];
        dst[i * 2 + 1] = hex[s[i] & 0x0F];
    }
    dst[len * 2] = '\0';
    return (int)(len * 2);
}

static inline int l_hex2bin(void *dst, const char *src, size_t len) {
    if (len % 2 != 0) return -1;
    unsigned char *d = (unsigned char *)dst;
    for (size_t i = 0; i < len; i += 2) {
        unsigned char hi, lo;
        if      (src[i] >= '0' && src[i] <= '9') hi = (unsigned char)(src[i] - '0');
        else if (src[i] >= 'a' && src[i] <= 'f') hi = (unsigned char)(src[i] - 'a' + 10);
        else if (src[i] >= 'A' && src[i] <= 'F') hi = (unsigned char)(src[i] - 'A' + 10);
        else return -1;
        if      (src[i+1] >= '0' && src[i+1] <= '9') lo = (unsigned char)(src[i+1] - '0');
        else if (src[i+1] >= 'a' && src[i+1] <= 'f') lo = (unsigned char)(src[i+1] - 'a' + 10);
        else if (src[i+1] >= 'A' && src[i+1] <= 'F') lo = (unsigned char)(src[i+1] - 'A' + 10);
        else return -1;
        d[i / 2] = (unsigned char)((hi << 4) | lo);
    }
    return (int)(len / 2);
}

static inline const char *l_basename(const char *path) {
    if (!path || !*path) return path;
    const char *last = path;
    const char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\')
            last = p + 1;
        p++;
    }
    return last;
}

static inline char *l_dirname(const char *path, char *buf, size_t bufsize) {
    if (!buf || bufsize == 0) return buf;
    if (!path || !*path) {
        buf[0] = '.';
        if (bufsize > 1) buf[1] = '\0'; else buf[0] = '\0';
        return buf;
    }
    // Find last separator
    const char *last_sep = (const char *)0;
    const char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\')
            last_sep = p;
        p++;
    }
    if (!last_sep) {
        // No separator: directory is "."
        buf[0] = '.';
        if (bufsize > 1) buf[1] = '\0'; else buf[0] = '\0';
        return buf;
    }
    // Root path: separator is the first character
    size_t len = (size_t)(last_sep - path);
    if (len == 0) len = 1; // keep the root separator
    if (len >= bufsize) len = bufsize - 1;
    l_memcpy(buf, path, len);
    buf[len] = '\0';
    return buf;
}

static inline char *l_path_join(char *buf, size_t bufsz, const char *dir, const char *file) {
    if (!buf || bufsz == 0) return buf;
    if (!dir || !*dir) {
        size_t flen = l_strlen(file);
        if (flen >= bufsz) flen = bufsz - 1;
        l_memcpy(buf, file, flen);
        buf[flen] = '\0';
        return buf;
    }
    size_t dlen = l_strlen(dir);
    size_t flen = l_strlen(file);
    int has_sep = (dir[dlen - 1] == '/' || dir[dlen - 1] == '\\');
    size_t pos = dlen < bufsz - 1 ? dlen : bufsz - 1;
    l_memcpy(buf, dir, pos);
    if (!has_sep && pos < bufsz - 1) buf[pos++] = '/';
    size_t rem = bufsz - 1 - pos;
    if (flen > rem) flen = rem;
    l_memcpy(buf + pos, file, flen);
    buf[pos + flen] = '\0';
    return buf;
}

static inline const char *l_path_ext(const char *path) {
    if (!path) return "";
    const char *base = l_basename(path);
    const char *dot = (const char *)0;
    const char *p = base;
    while (*p) {
        if (*p == '.') dot = p;
        p++;
    }
    // No dot, or dot is first char (hidden file like ".bashrc")
    if (!dot || dot == base) return "";
    return dot;
}

// Helper: append string to buf at pos, respecting bufsz
static inline size_t l__buf_append(char *buf, size_t bufsz, size_t pos, const char *s) {
    while (*s && pos < bufsz - 1) buf[pos++] = *s++;
    return pos;
}

static inline int l_ansi_move(char *buf, size_t bufsz, int row, int col) {
    if (!buf || bufsz < 2) return 0;
    char tmp[12];
    size_t pos = 0;
    pos = l__buf_append(buf, bufsz, pos, "\033[");
    l_itoa(row, tmp, 10);
    pos = l__buf_append(buf, bufsz, pos, tmp);
    pos = l__buf_append(buf, bufsz, pos, ";");
    l_itoa(col, tmp, 10);
    pos = l__buf_append(buf, bufsz, pos, tmp);
    pos = l__buf_append(buf, bufsz, pos, "H");
    buf[pos] = '\0';
    return (int)pos;
}

static inline int l_ansi_color(char *buf, size_t bufsz, int fg, int bg) {
    if (!buf || bufsz < 2) return 0;
    char tmp[12];
    size_t pos = 0;
    pos = l__buf_append(buf, bufsz, pos, "\033[");
    if (fg >= 0 && bg >= 0) {
        l_itoa(30 + fg, tmp, 10);
        pos = l__buf_append(buf, bufsz, pos, tmp);
        pos = l__buf_append(buf, bufsz, pos, ";");
        l_itoa(40 + bg, tmp, 10);
        pos = l__buf_append(buf, bufsz, pos, tmp);
    } else if (fg >= 0) {
        l_itoa(30 + fg, tmp, 10);
        pos = l__buf_append(buf, bufsz, pos, tmp);
    } else if (bg >= 0) {
        l_itoa(40 + bg, tmp, 10);
        pos = l__buf_append(buf, bufsz, pos, tmp);
    } else {
        pos = l__buf_append(buf, bufsz, pos, "0");
    }
    pos = l__buf_append(buf, bufsz, pos, "m");
    buf[pos] = '\0';
    return (int)pos;
}

static inline int l_ansi_color_rgb(char *buf, size_t bufsz, int r, int g, int b, int is_bg) {
    if (!buf || bufsz < 2) return 0;
    char tmp[12];
    size_t pos = 0;
    pos = l__buf_append(buf, bufsz, pos, "\033[");
    pos = l__buf_append(buf, bufsz, pos, is_bg ? "48;2;" : "38;2;");
    l_itoa(r, tmp, 10);
    pos = l__buf_append(buf, bufsz, pos, tmp);
    pos = l__buf_append(buf, bufsz, pos, ";");
    l_itoa(g, tmp, 10);
    pos = l__buf_append(buf, bufsz, pos, tmp);
    pos = l__buf_append(buf, bufsz, pos, ";");
    l_itoa(b, tmp, 10);
    pos = l__buf_append(buf, bufsz, pos, tmp);
    pos = l__buf_append(buf, bufsz, pos, "m");
    buf[pos] = '\0';
    return (int)pos;
}

#ifdef L_WITHSOCKETS

static inline unsigned short l_htons(unsigned short h) {
    return (unsigned short)((h >> 8) | (h << 8));
}

static inline unsigned int l_htonl(unsigned int h) {
    return ((h >> 24) & 0xFFu) | ((h >> 8) & 0xFF00u) |
           ((h << 8) & 0xFF0000u) | ((h << 24) & 0xFF000000u);
}

static inline int l_parse_ipv4(const char *ip, unsigned int *out_addr) {
    unsigned int result = 0;
    unsigned int octet = 0;
    int dots = 0;
    int shift = 0;

    if (!ip || !*ip) return 0;
    for (const char *p = ip; ; p++) {
        if (*p >= '0' && *p <= '9') {
            octet = octet * 10 + (unsigned int)(*p - '0');
            if (octet > 255) return 0;
        } else if (*p == '.' || *p == '\0') {
            result |= (octet << shift);
            shift += 8;
            octet = 0;
            if (*p == '.') dots++;
            if (*p == '\0') break;
        } else {
            return 0;
        }
    }
    if (dots != 3) return 0;
    if (out_addr) *out_addr = result;
    return 1;
}

static inline unsigned int l_inet_addr(const char *ip) {
    unsigned int result = 0;
    if (!l_parse_ipv4(ip, &result)) return 0;
    return result;
}

#ifdef _WIN32
static inline int l_wsa_init(void);
#endif

static inline void l_format_ipv4(unsigned int ip, char *out) {
    int pos = 0;
    for (int i = 0; i < 4; i++) {
        unsigned int octet = (ip >> (i * 8)) & 0xFFu;
        if (octet >= 100) out[pos++] = (char)('0' + octet / 100);
        if (octet >= 10) out[pos++] = (char)('0' + (octet / 10) % 10);
        out[pos++] = (char)('0' + octet % 10);
        if (i < 3) out[pos++] = '.';
    }
    out[pos] = '\0';
}

// IPv6 parse: "2001:db8::1" → 16-byte binary. Returns 1 on success, 0 on error.
static inline int l_parse_ipv6(const char *ip, unsigned char out[16]) {
    if (!ip) return 0;
    unsigned char tmp[16];
    l_memset(tmp, 0, 16);
    int groups = 0, dcolon = -1, pos = 0;
    const char *p = ip;

    // Handle leading ::
    if (p[0] == ':' && p[1] == ':') { dcolon = 0; p += 2; if (*p == '\0') { l_memcpy(out, tmp, 16); return 1; } }

    while (*p && groups < 8) {
        if (*p == ':' && p[1] == ':') {
            if (dcolon >= 0) return 0; // only one :: allowed
            dcolon = groups;
            p += 2;
            if (*p == '\0') break;
            continue;
        }
        if (*p == ':') p++;

        // Parse hex group
        unsigned int val = 0;
        int digits = 0;
        while ((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
            val <<= 4;
            if (*p >= '0' && *p <= '9') val += (unsigned)(*p - '0');
            else if (*p >= 'a' && *p <= 'f') val += (unsigned)(*p - 'a' + 10);
            else val += (unsigned)(*p - 'A' + 10);
            digits++;
            p++;
        }
        if (digits == 0 || digits > 4 || val > 0xFFFF) return 0;
        tmp[pos++] = (unsigned char)(val >> 8);
        tmp[pos++] = (unsigned char)(val & 0xFF);
        groups++;
    }
    if (*p != '\0') return 0;

    if (dcolon >= 0) {
        // Expand :: by shifting groups after dcolon to the end
        int head = dcolon * 2;
        int tail = (groups - dcolon) * 2;
        int gap = 16 - head - tail;
        if (gap < 0) return 0;
        l_memset(out, 0, 16);
        l_memcpy(out, tmp, head);
        l_memcpy(out + head + gap, tmp + head, tail);
    } else {
        if (groups != 8) return 0;
        l_memcpy(out, tmp, 16);
    }
    return 1;
}

// IPv6 format: 16-byte binary → compressed text (no :: shortening for simplicity)
static inline char *l_format_ipv6(const unsigned char addr[16], char *buf, size_t bufsz) {
    if (!buf || bufsz < L_INET6_ADDRSTRLEN) return (char *)0;
    static const char hex[] = "0123456789abcdef";
    int pos = 0;
    for (int i = 0; i < 8; i++) {
        unsigned int g = ((unsigned int)addr[i*2] << 8) | addr[i*2+1];
        // Skip leading zeros but always emit at least one digit
        int started = 0;
        for (int shift = 12; shift >= 0; shift -= 4) {
            int nibble = (int)((g >> shift) & 0xF);
            if (nibble || started || shift == 0) {
                buf[pos++] = hex[nibble];
                started = 1;
            }
        }
        if (i < 7) buf[pos++] = ':';
    }
    buf[pos] = '\0';
    return buf;
}

// Construct L_SockAddr from IPv4 dotted-quad string
static inline int l_sockaddr_ipv4(L_SockAddr *sa, const char *ip, int port) {
    unsigned int addr;
    if (!sa || !l_parse_ipv4(ip, &addr)) return -1;
    l_memset(sa, 0, sizeof(*sa));
    sa->family = L_AF_INET;
    sa->port = (unsigned short)port;
    l_memcpy(sa->addr, &addr, 4);
    return 0;
}

// Construct L_SockAddr from IPv6 text string
static inline int l_sockaddr_ipv6(L_SockAddr *sa, const char *ip, int port) {
    if (!sa) return -1;
    l_memset(sa, 0, sizeof(*sa));
    if (!l_parse_ipv6(ip, sa->addr)) return -1;
    sa->family = L_AF_INET6;
    sa->port = (unsigned short)port;
    return 0;
}

static inline int l_valid_hostname(const char *name) {
    size_t label_len = 0;

    if (!name || !*name) return 0;
    for (const char *p = name; ; p++) {
        char ch = *p;
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9')) {
            label_len++;
        } else if (ch == '-') {
            if (label_len == 0) return 0;
            label_len++;
        } else if (ch == '.' || ch == '\0') {
            if (label_len == 0) return 0;
            if (p[-1] == '-') return 0;
            if (label_len > 63) return 0;
            if (ch == '\0') break;
            label_len = 0;
        } else {
            return 0;
        }
    }
    return 1;
}

static inline size_t l_dns_skip_name(const unsigned char *msg, size_t msg_len, size_t off) {
    while (off < msg_len) {
        unsigned char len = msg[off];
        if (len == 0) return off + 1;
        if ((len & 0xC0u) == 0xC0u) {
            if (off + 1 >= msg_len) return 0;
            return off + 2;
        }
        if (len & 0xC0u) return 0;
        off++;
        if (off + len > msg_len) return 0;
        off += len;
    }
    return 0;
}

static inline size_t l_dns_encode_name(unsigned char *buf, size_t buf_sz, const char *name) {
    size_t off = 0;
    size_t start = 0;
    if (!name || !*name) return 0;
    for (;;) {
        size_t label_len = 0;
        while (name[start + label_len] && name[start + label_len] != '.')
            label_len++;
        if (label_len == 0 || label_len > 63 || off + 1 + label_len >= buf_sz) return 0;
        buf[off++] = (unsigned char)label_len;
        l_memcpy(buf + off, name + start, label_len);
        off += label_len;
        if (!name[start + label_len]) break;
        start += label_len + 1;
    }
    if (off >= buf_sz) return 0;
    buf[off++] = 0;
    return off;
}

#ifndef _WIN32
static inline int l_hosts_lookup_ipv4(const char *name, unsigned int *out_addr) {
    char buf[4097];
    size_t name_len;
    size_t i = 0;
    L_FD fd;
    ssize_t n;

    if (!name || !*name) return 0;
    fd = l_open_read("/etc/hosts");
    if (fd < 0) return 0;
    n = l_read(fd, buf, sizeof(buf) - 1);
    l_close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';
    name_len = l_strlen(name);

    while (i < (size_t)n) {
        char ip[16];
        unsigned int addr = 0;
        int addr_ok = 0;
        size_t start, tok_len;

        while (i < (size_t)n && (buf[i] == ' ' || buf[i] == '\t')) i++;
        if (i >= (size_t)n) break;
        if (buf[i] == '#' || buf[i] == '\r' || buf[i] == '\n') {
            while (i < (size_t)n && buf[i] != '\n') i++;
            if (i < (size_t)n) i++;
            continue;
        }

        start = i;
        while (i < (size_t)n && buf[i] != ' ' && buf[i] != '\t' &&
               buf[i] != '\r' && buf[i] != '\n' && buf[i] != '#')
            i++;
        tok_len = i - start;
        if (tok_len < sizeof(ip)) {
            l_memcpy(ip, buf + start, tok_len);
            ip[tok_len] = '\0';
            addr_ok = l_parse_ipv4(ip, &addr);
        }

        while (i < (size_t)n && buf[i] != '\r' && buf[i] != '\n') {
            while (i < (size_t)n && (buf[i] == ' ' || buf[i] == '\t')) i++;
            if (i >= (size_t)n || buf[i] == '#' || buf[i] == '\r' || buf[i] == '\n')
                break;
            start = i;
            while (i < (size_t)n && buf[i] != ' ' && buf[i] != '\t' &&
                   buf[i] != '\r' && buf[i] != '\n' && buf[i] != '#')
                i++;
            tok_len = i - start;
            if (addr_ok && tok_len == name_len &&
                l_strncasecmp(buf + start, name, name_len) == 0) {
                if (out_addr) *out_addr = addr;
                return 1;
            }
        }

        while (i < (size_t)n && buf[i] != '\n') i++;
        if (i < (size_t)n) i++;
    }

    return 0;
}

static inline int l_resolv_conf_nameserver(char *out, size_t out_sz) {
    char buf[2049];
    size_t i = 0;
    L_FD fd;
    ssize_t n;

    if (!out || out_sz < 16) return 0;
    fd = l_open_read("/etc/resolv.conf");
    if (fd < 0) return 0;
    n = l_read(fd, buf, sizeof(buf) - 1);
    l_close(fd);
    if (n <= 0) return 0;
    buf[n] = '\0';

    while (i < (size_t)n) {
        size_t start, tok_len;
        while (i < (size_t)n && (buf[i] == ' ' || buf[i] == '\t')) i++;
        if (i >= (size_t)n) break;
        if (buf[i] == '#' || buf[i] == '\r' || buf[i] == '\n') {
            while (i < (size_t)n && buf[i] != '\n') i++;
            if (i < (size_t)n) i++;
            continue;
        }
        if (i + 10 >= (size_t)n ||
            l_strncmp(buf + i, "nameserver", 10) != 0 ||
            (buf[i + 10] != ' ' && buf[i + 10] != '\t')) {
            while (i < (size_t)n && buf[i] != '\n') i++;
            if (i < (size_t)n) i++;
            continue;
        }
        i += 10;
        while (i < (size_t)n && (buf[i] == ' ' || buf[i] == '\t')) i++;
        start = i;
        while (i < (size_t)n && buf[i] != ' ' && buf[i] != '\t' &&
               buf[i] != '\r' && buf[i] != '\n' && buf[i] != '#')
            i++;
        tok_len = i - start;
        if (tok_len == 0 || tok_len >= out_sz) continue;
        l_memcpy(out, buf + start, tok_len);
        out[tok_len] = '\0';
        return l_parse_ipv4(out, (unsigned int *)0);
    }

    return 0;
}

static inline int l_dns_lookup_ipv4(const char *name, unsigned int *out_addr) {
    unsigned char query[512];
    unsigned char resp[512];
    char nameserver[16];
    unsigned short id;
    size_t off;
    size_t i;
    L_SOCKET sock;
    ptrdiff_t n;
    L_PollFd pfd;

    if (!name || !*name) return 0;
    if (!l_resolv_conf_nameserver(nameserver, sizeof(nameserver))) return 0;

    id = (unsigned short)(l_getpid() ^ (int)l_time((long long *)0));
    query[0] = (unsigned char)(id >> 8);
    query[1] = (unsigned char)(id & 0xFF);
    query[2] = 0x01;
    query[3] = 0x00;
    query[4] = 0x00;
    query[5] = 0x01;
    query[6] = query[7] = query[8] = query[9] = query[10] = query[11] = 0x00;
    off = 12;
    off += l_dns_encode_name(query + off, sizeof(query) - off, name);
    if (off <= 12 || off + 4 > sizeof(query)) return 0;
    query[off++] = 0x00;
    query[off++] = 0x01;
    query[off++] = 0x00;
    query[off++] = 0x01;

    sock = l_socket_udp();
    if (sock < 0) return 0;
    if (l_socket_sendto(sock, query, off, nameserver, 53) < 0) {
        l_socket_close(sock);
        return 0;
    }

    pfd.fd = (L_FD)sock;
    pfd.events = L_POLLIN;
    pfd.revents = 0;
    if (l_poll(&pfd, 1, 2000) <= 0 || !(pfd.revents & L_POLLIN)) {
        l_socket_close(sock);
        return 0;
    }

    n = l_socket_recvfrom(sock, resp, sizeof(resp), (char *)0, (int *)0);
    l_socket_close(sock);
    if (n < 12) return 0;
    if (resp[0] != query[0] || resp[1] != query[1]) return 0;
    if (!(resp[2] & 0x80) || (resp[3] & 0x0F)) return 0;

    off = 12;
    for (i = 0; i < (size_t)((resp[4] << 8) | resp[5]); i++) {
        off = l_dns_skip_name(resp, (size_t)n, off);
        if (off == 0 || off + 4 > (size_t)n) return 0;
        off += 4;
    }

    for (i = 0; i < (size_t)((resp[6] << 8) | resp[7]); i++) {
        unsigned short type, class_, rdlen;
        off = l_dns_skip_name(resp, (size_t)n, off);
        if (off == 0 || off + 10 > (size_t)n) return 0;
        type = (unsigned short)((resp[off] << 8) | resp[off + 1]);
        class_ = (unsigned short)((resp[off + 2] << 8) | resp[off + 3]);
        rdlen = (unsigned short)((resp[off + 8] << 8) | resp[off + 9]);
        off += 10;
        if (off + rdlen > (size_t)n) return 0;
        if (type == 1 && class_ == 1 && rdlen == 4) {
            if (out_addr) {
                *out_addr = (unsigned int)resp[off] |
                            ((unsigned int)resp[off + 1] << 8) |
                            ((unsigned int)resp[off + 2] << 16) |
                            ((unsigned int)resp[off + 3] << 24);
            }
            return 1;
        }
        off += rdlen;
    }

    return 0;
}
#endif

static inline int l_resolve(const char *hostname, char *ip_out) {
    unsigned int addr;
    if (!hostname || !*hostname || !ip_out) return -1;
    if (l_parse_ipv4(hostname, &addr)) {
        l_strcpy(ip_out, hostname);
        return 0;
    }
    if (!l_valid_hostname(hostname)) return -1;
#ifdef _WIN32
    if (l_wsa_init() < 0) return -1;
    {
        struct hostent *host = gethostbyname(hostname);
        if (!host || !host->h_addr_list || !host->h_addr_list[0] || host->h_length != 4)
            return -1;
        l_memcpy(&addr, host->h_addr_list[0], 4);
    }
#else
    if (!l_hosts_lookup_ipv4(hostname, &addr) &&
        !l_dns_lookup_ipv4(hostname, &addr))
        return -1;
#endif
    l_format_ipv4(addr, ip_out);
    return 0;
}

#endif // L_WITHSOCKETS

#ifdef __unix__

#include <asm/unistd.h>

/* all the *at functions */
#ifndef AT_FDCWD
#define AT_FDCWD             -100
#endif

/* lseek */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2



#if defined(__x86_64__)
/* Syscalls for x86_64 :
 *   - registers are 64-bit
 *   - syscall number is passed in rax
 *   - arguments are in rdi, rsi, rdx, r10, r8, r9 respectively
 *   - the system call is performed by calling the syscall instruction
 *   - syscall return comes in rax
 *   - rcx and r8..r11 may be clobbered, others are preserved.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *   - the syscall number is always specified last in order to allow to force
 *     some registers before (gcc refuses a %-register at the last position).
 */

#define my_syscall0(num)                                                      \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret)                                                 \
                         : "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret)                                                 \
                         : "r"(_arg1),                                                 \
                         "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         register long _arg2 asm("rsi") = (long)(arg2);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret)                                                 \
                         : "r"(_arg1), "r"(_arg2),                                     \
                         "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         register long _arg2 asm("rsi") = (long)(arg2);                        \
         register long _arg3 asm("rdx") = (long)(arg3);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret)                                                 \
                         : "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
                         "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         register long _arg2 asm("rsi") = (long)(arg2);                        \
         register long _arg3 asm("rdx") = (long)(arg3);                        \
         register long _arg4 asm("r10") = (long)(arg4);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret), "=r"(_arg4)                                    \
                         : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
                         "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r11", "memory", "cc"                    \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         register long _arg2 asm("rsi") = (long)(arg2);                        \
         register long _arg3 asm("rdx") = (long)(arg3);                        \
         register long _arg4 asm("r10") = (long)(arg4);                        \
         register long _arg5 asm("r8")  = (long)(arg5);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret), "=r"(_arg4), "=r"(_arg5)                       \
                         : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
                         "0"(_num)                                                   \
                         : "rcx", "r9", "r11", "memory", "cc"                          \
                         );                                                            \
         _ret;                                                                 \
})

#define my_syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6)                  \
        ({                                                                            \
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         register long _arg1 asm("rdi") = (long)(arg1);                        \
         register long _arg2 asm("rsi") = (long)(arg2);                        \
         register long _arg3 asm("rdx") = (long)(arg3);                        \
         register long _arg4 asm("r10") = (long)(arg4);                        \
         register long _arg5 asm("r8")  = (long)(arg5);                        \
         register long _arg6 asm("r9")  = (long)(arg6);                        \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret), "=r"(_arg4), "=r"(_arg5)                       \
                         : "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
                         "r"(_arg6), "0"(_num)                                       \
                         : "rcx", "r11", "memory", "cc"                                \
                         );                                                                    \
                         _ret;                                                                 \
                         })

#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_CLOEXEC    0x80000
#define O_DIRECTORY    0x4000

#elif defined(__aarch64__)
/* Syscalls for AARCH64 :
 *   - registers are 64-bit
 *   - stack is 16-byte aligned
 *   - syscall number is passed in x8
 *   - arguments are in x0, x1, x2, x3, x4, x5
 *   - the system call is performed by calling svc 0
 *   - syscall return comes in x0.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *
 * On aarch64, select() is not implemented so we have to use pselect6().
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0");                                        \
	                                                                      \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	                                                                      \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1),                                                 \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	                                                                      \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	register long _arg3 asm("x2") = (long)(arg3);                         \
	                                                                      \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _num  asm("x8") = (num);                                \
	register long _arg1 asm("x0") = (long)(arg1);                         \
	register long _arg2 asm("x1") = (long)(arg2);                         \
	register long _arg3 asm("x2") = (long)(arg3);                         \
	register long _arg4 asm("x3") = (long)(arg4);                         \
	                                                                      \
	asm volatile (                                                        \
		"svc #0\n"                                                    \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "r"(_num)                                                   \
		: "memory", "cc"                                              \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5) \
({                                                     \
    long _ret; \
    register long _num asm("x8") = (num); \
    register long _x0 asm("x0") = (long)(arg1); \
    register long _x1 asm("x1") = (long)(arg2); \
    register long _x2 asm("x2") = (long)(arg3); \
    register long _x3 asm("x3") = (long)(arg4); \
    register long _x4 asm("x4") = (long)(arg5); \
    asm volatile ( \
        "svc #0\n" \
        : "+r"(_x0) \
        : "r"(_num), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4) \
        : "memory" \
    ); \
    _ret = _x0; \
    _ret; \
})

#define my_syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6) \
({                                                           \
    long _ret; \
    register long _num asm("x8") = (num); \
    register long _x0 asm("x0") = (long)(arg1); \
    register long _x1 asm("x1") = (long)(arg2); \
    register long _x2 asm("x2") = (long)(arg3); \
    register long _x3 asm("x3") = (long)(arg4); \
    register long _x4 asm("x4") = (long)(arg5); \
    register long _x5 asm("x5") = (long)(arg6); \
    asm volatile ( \
        "svc #0\n" \
        : "+r"(_x0) \
        : "r"(_num), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4), "r"(_x5) \
        : "memory" \
    ); \
    _ret = _x0; \
    _ret; \
})

// File flags (same as x86_64)
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_CLOEXEC    0x80000
#define O_DIRECTORY   0x10000

#elif defined(__riscv) && __riscv_xlen == 64
/* Syscalls for RISC-V (rv64gc) :
 *   - registers are 64-bit
 *   - stack is 16-byte aligned
 *   - syscall number is passed in a7
 *   - arguments are in a0, a1, a2, a3, a4, a5
 *   - the system call is performed by calling ecall
 *   - syscall return comes in a0.
 *   - the arguments are cast to long and assigned into the target registers
 *     which are then simply passed as registers to the asm code, so that we
 *     don't have to experience issues with register constraints.
 *
 * On RISC-V, select() is not implemented so we have to use ppoll().
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _num  asm("a7") = (num);                                \
	register long _arg1 asm("a0");                                        \
	                                                                      \
	asm volatile (                                                        \
		"ecall\n"                                                     \
		: "=r"(_arg1)                                                 \
		: "r"(_num)                                                   \
		: "memory"                                                    \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _num  asm("a7") = (num);                                \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	                                                                      \
	asm volatile (                                                        \
		"ecall\n"                                                     \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1),                                                 \
		  "r"(_num)                                                   \
		: "memory"                                                    \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _num  asm("a7") = (num);                                \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	                                                                      \
	asm volatile (                                                        \
		"ecall\n"                                                     \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "r"(_num)                                                   \
		: "memory"                                                    \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _num  asm("a7") = (num);                                \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg3 asm("a2") = (long)(arg3);                         \
	                                                                      \
	asm volatile (                                                        \
		"ecall\n"                                                     \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  "r"(_num)                                                   \
		: "memory"                                                    \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _num  asm("a7") = (num);                                \
	register long _arg1 asm("a0") = (long)(arg1);                         \
	register long _arg2 asm("a1") = (long)(arg2);                         \
	register long _arg3 asm("a2") = (long)(arg3);                         \
	register long _arg4 asm("a3") = (long)(arg4);                         \
	                                                                      \
	asm volatile (                                                        \
		"ecall\n"                                                     \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  "r"(_num)                                                   \
		: "memory"                                                    \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5) \
({                                                     \
    long _ret; \
    register long _num asm("a7") = (num); \
    register long _a0 asm("a0") = (long)(arg1); \
    register long _a1 asm("a1") = (long)(arg2); \
    register long _a2 asm("a2") = (long)(arg3); \
    register long _a3 asm("a3") = (long)(arg4); \
    register long _a4 asm("a4") = (long)(arg5); \
    asm volatile ( \
        "ecall\n" \
        : "+r"(_a0) \
        : "r"(_num), "r"(_a1), "r"(_a2), "r"(_a3), "r"(_a4) \
        : "memory" \
    ); \
    _ret = _a0; \
    _ret; \
})

#define my_syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6) \
({                                                           \
    long _ret; \
    register long _num asm("a7") = (num); \
    register long _a0 asm("a0") = (long)(arg1); \
    register long _a1 asm("a1") = (long)(arg2); \
    register long _a2 asm("a2") = (long)(arg3); \
    register long _a3 asm("a3") = (long)(arg4); \
    register long _a4 asm("a4") = (long)(arg5); \
    register long _a5 asm("a5") = (long)(arg6); \
    asm volatile ( \
        "ecall\n" \
        : "+r"(_a0) \
        : "r"(_num), "r"(_a1), "r"(_a2), "r"(_a3), "r"(_a4), "r"(_a5) \
        : "memory" \
    ); \
    _ret = _a0; \
    _ret; \
})

// File flags (same as AArch64)
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_CLOEXEC    0x80000
#define O_DIRECTORY   0x10000

#elif defined(__arm__)
/* Syscalls for ARM (32-bit, EABI) :
 *   - syscall number is passed in r7
 *   - arguments are in r0, r1, r2, r3, r4, r5
 *   - the system call is performed by calling svc #0
 *   - syscall return comes in r0
 *   - lr is clobbered by the svc instruction
 *   - r7 is saved/restored via push/pop to avoid conflicts with
 *     Thumb mode's use of r7 as the frame pointer
 */

#define my_syscall0(num)                                                      \
({                                                                            \
	register long _arg1 asm("r0");                                        \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall1(num, arg1)                                                \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1),                                                 \
		  [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2),                                     \
		  [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall3(num, arg1, arg2, arg3)                                    \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3),                         \
		  [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	register long _arg4 asm("r3") = (long)(arg4);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4),             \
		  [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	register long _arg4 asm("r3") = (long)(arg4);                         \
	register long _arg5 asm("r4") = (long)(arg5);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5),\
		  [nr] "r"(_num)                                              \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

#define my_syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6)                  \
({                                                                            \
	register long _arg1 asm("r0") = (long)(arg1);                         \
	register long _arg2 asm("r1") = (long)(arg2);                         \
	register long _arg3 asm("r2") = (long)(arg3);                         \
	register long _arg4 asm("r3") = (long)(arg4);                         \
	register long _arg5 asm("r4") = (long)(arg5);                         \
	register long _arg6 asm("r5") = (long)(arg6);                         \
	long _num = (long)(num);                                              \
	                                                                      \
	asm volatile (                                                        \
		"push {r7}\n"                                                 \
		"mov r7, %[nr]\n"                                             \
		"svc #0\n"                                                    \
		"pop {r7}\n"                                                  \
		: "=r"(_arg1)                                                 \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5),\
		  "r"(_arg6), [nr] "r"(_num)                                  \
		: "memory", "cc", "lr"                                        \
	);                                                                    \
	_arg1;                                                                \
})

// File flags (same as x86_64)
#define O_RDONLY            0
#define O_WRONLY            1
#define O_RDWR              2
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_CLOEXEC    0x80000
#define O_DIRECTORY    0x4000

#else
#error "Supported architectures: linux x86_64, aarch64, arm, riscv64, and windows. Paste relevant nolibc.h sections for more archs."
#endif

// Dummy function to satisfy libgcc requirement on ARM
#ifdef __arm__
int raise(int sig) {
    (void)sig;
    return 0;
}

/* ARM EABI integer division helpers — ARM32 has no hardware divide instruction.
   clang emits calls to these instead of inlining division.
   __attribute__((used)) prevents LTO/gc-sections from stripping them. */
__attribute__((used)) unsigned __aeabi_uidiv(unsigned num, unsigned den) {
    if (den == 0) return 0;
    unsigned quot = 0;
    int shift = 0;
    while (den <= (num >> 1) && shift < 31) { den <<= 1; shift++; }
    while (shift >= 0) {
        if (num >= den) { num -= den; quot |= (1u << shift); }
        den >>= 1;
        shift--;
    }
    return quot;
}

__attribute__((used)) int __aeabi_idiv(int num, int den) {
    int neg = 0;
    unsigned unum, uden;
    if (num < 0) { unum = (unsigned)(-(num + 1)) + 1u; neg = !neg; } else { unum = (unsigned)num; }
    if (den < 0) { uden = (unsigned)(-(den + 1)) + 1u; neg = !neg; } else { uden = (unsigned)den; }
    int q = (int)__aeabi_uidiv(unum, uden);
    return neg ? -q : q;
}

/* divmod returns {quotient, remainder} in {r0, r1}.
   ARM EABI: unsigned long long returned in r0:r1. */
__attribute__((used)) unsigned long long __aeabi_uidivmod(unsigned num, unsigned den) {
    unsigned q = __aeabi_uidiv(num, den);
    unsigned r = num - q * den;
    return q | ((unsigned long long)r << 32);
}

__attribute__((used)) long long __aeabi_idivmod(int num, int den) {
    int q = __aeabi_idiv(num, den);
    int r = num - q * den;
    return (unsigned)q | ((unsigned long long)(unsigned)r << 32);
}

/* ARM EABI 64-bit shift helpers — ARM32 has no 64-bit shift instructions.
   All use naked asm with only native 32-bit ARM ops, so no recursion risk.
   target("arm") forces ARM mode — these use conditional instructions that
   are not valid in Thumb mode (GCC defaults to Thumb on ARM).
   Input:  r0:r1 = value (low:high), r2 = shift amount
   Output: r0:r1 = result (low:high) */
__attribute__((naked, used, target("arm")))
void __aeabi_llsl(void) {
    __asm__ volatile(
        "subs r3, r2, #32\n\t"
        "rsb ip, r2, #32\n\t"
        "movmi r1, r1, lsl r2\n\t"
        "orrmi r1, r1, r0, lsr ip\n\t"
        "movpl r1, r0, lsl r3\n\t"
        "mov r0, r0, lsl r2\n\t"
        "bx lr\n\t"
    );
}

__attribute__((naked, used, target("arm")))
void __aeabi_llsr(void) {
    __asm__ volatile(
        "subs r3, r2, #32\n\t"
        "rsb ip, r2, #32\n\t"
        "movmi r0, r0, lsr r2\n\t"
        "orrmi r0, r0, r1, lsl ip\n\t"
        "movpl r0, r1, lsr r3\n\t"
        "mov r1, r1, lsr r2\n\t"
        "bx lr\n\t"
    );
}

__attribute__((naked, used, target("arm")))
void __aeabi_lasr(void) {
    __asm__ volatile(
        "subs r3, r2, #32\n\t"
        "rsb ip, r2, #32\n\t"
        "movmi r0, r0, lsr r2\n\t"
        "orrmi r0, r0, r1, lsl ip\n\t"
        "movpl r0, r1, asr r3\n\t"
        "mov r1, r1, asr r2\n\t"
        "bx lr\n\t"
    );
}

/* 64-bit unsigned division helper: returns quotient, stores remainder via pointer.
   Bit-by-bit long division — no hardware divide needed, no recursion into __aeabi_*. */
__attribute__((used))
unsigned long long __udivmoddi4(unsigned long long num, unsigned long long den, unsigned long long *rem) {
    if (den == 0) { if (rem) *rem = 0; return 0; }
    unsigned long long quot = 0, r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((num >> i) & 1);
        if (r >= den) { r -= den; quot |= (1ULL << i); }
    }
    if (rem) *rem = r;
    return quot;
}

/* 64-bit signed division helper: returns quotient, stores remainder via pointer. */
__attribute__((used))
long long __sdivmoddi4(long long num, long long den, long long *rem) {
    int neg_quot = 0, neg_rem = 0;
    unsigned long long unum, uden, urem;
    if (num < 0) { unum = -(unsigned long long)num; neg_quot = 1; neg_rem = 1; }
    else { unum = (unsigned long long)num; }
    if (den < 0) { uden = -(unsigned long long)den; neg_quot ^= 1; }
    else { uden = (unsigned long long)den; }
    unsigned long long q = __udivmoddi4(unum, uden, &urem);
    if (rem) *rem = neg_rem ? -(long long)urem : (long long)urem;
    return neg_quot ? -(long long)q : (long long)q;
}

/* ARM EABI 64-bit unsigned divmod.
   Input:  r0:r1 = numerator, r2:r3 = denominator
   Output: r0:r1 = quotient,  r2:r3 = remainder
   Naked wrapper calls __udivmoddi4 and places remainder in r2:r3. */
__attribute__((naked, used, target("arm")))
void __aeabi_uldivmod(void) {
    __asm__ volatile(
        "push {r4, lr}\n\t"
        "sub sp, sp, #16\n\t"
        "add r4, sp, #8\n\t"
        "str r4, [sp]\n\t"
        "bl __udivmoddi4\n\t"
        "ldr r2, [sp, #8]\n\t"
        "ldr r3, [sp, #12]\n\t"
        "add sp, sp, #16\n\t"
        "pop {r4, pc}\n\t"
    );
}

/* ARM EABI 64-bit signed divmod.
   Input:  r0:r1 = numerator, r2:r3 = denominator
   Output: r0:r1 = quotient,  r2:r3 = remainder */
__attribute__((naked, used, target("arm")))
void __aeabi_ldivmod(void) {
    __asm__ volatile(
        "push {r4, lr}\n\t"
        "sub sp, sp, #16\n\t"
        "add r4, sp, #8\n\t"
        "str r4, [sp]\n\t"
        "bl __sdivmoddi4\n\t"
        "ldr r2, [sp, #8]\n\t"
        "ldr r3, [sp, #12]\n\t"
        "add sp, sp, #16\n\t"
        "pop {r4, pc}\n\t"
    );
}

/* ARM EABI double ↔ unsigned-long-long conversion helpers.
   ARM clang emits calls to these from float/integer code (l_strtod, l_snprintf %f).
   Must NOT cast between double and unsigned long long — that would recurse.
   Instead, manipulate IEEE 754 bits directly via union.
   pcs("aapcs") forces base AAPCS (soft-float): doubles travel in r0:r1,
   matching the ARM EABI calling convention even on hard-float targets. */
__attribute__((used, pcs("aapcs")))
unsigned long long __aeabi_f2ulz(float v)
{
    union { float f; unsigned u; } conv;
    conv.f = v;
    unsigned bits = conv.u;
    if (bits >> 31) return 0;                           /* negative or -0 */
    int exp = (int)((bits >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    if (exp >= 64) return 0xFFFFFFFFFFFFFFFFULL;       /* overflow / NaN */
    unsigned long long mantissa = (unsigned long long)((bits & 0x007FFFFFu) | 0x00800000u);
    if (exp >= 23) return mantissa << (exp - 23);
    return mantissa >> (23 - exp);
}

__attribute__((used, pcs("aapcs")))
long long __aeabi_f2lz(float v)
{
    union { float f; unsigned u; } conv;
    conv.f = v;
    unsigned bits = conv.u;
    int sign = (int)(bits >> 31);
    int exp = (int)((bits >> 23) & 0xFF) - 127;
    if (exp < 0) return 0;
    if (exp >= 63) return sign ? (-9223372036854775807LL - 1) : 9223372036854775807LL;
    unsigned long long mantissa = (unsigned long long)((bits & 0x007FFFFFu) | 0x00800000u);
    unsigned long long result;
    if (exp >= 23) result = mantissa << (exp - 23);
    else result = mantissa >> (23 - exp);
    return sign ? -(long long)result : (long long)result;
}

__attribute__((used, pcs("aapcs")))
unsigned long long __aeabi_d2ulz(double v)
{
    union { double d; unsigned long long u; } conv;
    conv.d = v;
    unsigned long long bits = conv.u;
    if (bits >> 63) return 0;                          /* negative or -0 */
    int exp = (int)((bits >> 52) & 0x7FF) - 1023;
    if (exp < 0) return 0;
    if (exp >= 64) return 0xFFFFFFFFFFFFFFFFULL;       /* overflow / NaN */
    unsigned long long mantissa = (bits & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
    if (exp >= 52) return mantissa << (exp - 52);
    return mantissa >> (52 - exp);
}

__attribute__((used, pcs("aapcs")))
double __aeabi_ul2d(unsigned long long v)
{
    if (v == 0) return 0.0;
    int shift = 0;
    unsigned long long tmp = v;
    while (!(tmp & (1ULL << 63))) { tmp <<= 1; shift++; }
    int exp = 1023 + 63 - shift;
    unsigned long long mantissa = (tmp << 1) >> 12;    /* remove implicit 1, keep 52 bits */
    union { double d; unsigned long long u; } conv;
    conv.u = ((unsigned long long)exp << 52) | mantissa;
    return conv.d;
}

/* Signed variants */
__attribute__((used, pcs("aapcs")))
long long __aeabi_d2lz(double v)
{
    union { double d; unsigned long long u; } conv;
    conv.d = v;
    unsigned long long bits = conv.u;
    int sign = (int)(bits >> 63);
    int exp = (int)((bits >> 52) & 0x7FF) - 1023;
    if (exp < 0) return 0;
    if (exp >= 63) return sign ? (-9223372036854775807LL - 1) : 9223372036854775807LL;
    unsigned long long mantissa = (bits & 0x000FFFFFFFFFFFFFULL) | 0x0010000000000000ULL;
    unsigned long long result;
    if (exp >= 52) result = mantissa << (exp - 52);
    else result = mantissa >> (52 - exp);
    return sign ? -(long long)result : (long long)result;
}

__attribute__((used, pcs("aapcs")))
double __aeabi_l2d(long long v)
{
    if (v == 0) return 0.0;
    int sign = 0;
    unsigned long long uv;
    if (v < 0) { sign = 1; uv = -(unsigned long long)v; }
    else { uv = (unsigned long long)v; }
    int shift = 0;
    unsigned long long tmp = uv;
    while (!(tmp & (1ULL << 63))) { tmp <<= 1; shift++; }
    int exp = 1023 + 63 - shift;
    unsigned long long mantissa = (tmp << 1) >> 12;
    union { double d; unsigned long long u; } conv;
    conv.u = ((unsigned long long)sign << 63) | ((unsigned long long)exp << 52) | mantissa;
    return conv.d;
}
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#ifndef _WIN32
static char **l_envp;
#endif

static noreturn inline void l_exit(int status)
{
    my_syscall1(__NR_exit, status & 255);
    while(1);
}

static inline int l_chdir(const char *path)
{
    return my_syscall1(__NR_chdir, path);
}

static inline char *l_getcwd(char *buf, size_t size)
{
#if defined(__x86_64__)
    long ret = my_syscall2(79 /*__NR_getcwd*/, buf, size);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall2(17 /*__NR_getcwd*/, buf, size);
#elif defined(__arm__)
    long ret = my_syscall2(183 /*__NR_getcwd*/, buf, size);
#endif
    if (ret < 0) return (char *)0;
    return buf;
}

static inline int l_close(L_FD fd)
{
    int ret = my_syscall1(__NR_close, fd);
    l_set_errno_from_ret(ret);
    return ret;
}

static inline int l_dup(L_FD fd)
{
    return my_syscall1(__NR_dup, fd);
}

static inline int l_pipe(L_FD fds[2])
{
    int tmp[2];
#if defined(__x86_64__)
    long ret = my_syscall2(293 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall2(59 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#elif defined(__arm__)
    long ret = my_syscall2(359 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#endif
    if (ret < 0) return -1;
    fds[0] = (L_FD)tmp[0];
    fds[1] = (L_FD)tmp[1];
    return 0;
}

// Forward declaration: l_fstat is defined below but needed here by l_dup2
static inline int l_fstat(L_FD fd, L_Stat *st);

static inline int l_dup2(L_FD oldfd, L_FD newfd)
{
#if defined(__x86_64__)
    return (int)my_syscall2(33 /*__NR_dup2*/, oldfd, newfd);
#elif defined(__aarch64__) || defined(__riscv)
    if (oldfd == newfd) {
        L_Stat st;
        return l_fstat(oldfd, &st) < 0 ? -1 : (int)oldfd;
    }
    return (int)my_syscall3(24 /*__NR_dup3*/, oldfd, newfd, 0);
#elif defined(__arm__)
    return (int)my_syscall2(63 /*__NR_dup2*/, oldfd, newfd);
#endif
}

static inline L_PID l_fork(void)
{
#if defined(__x86_64__)
    return (L_PID)my_syscall5(56 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#elif defined(__aarch64__) || defined(__riscv)
    return (L_PID)my_syscall5(220 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#elif defined(__arm__)
    return (L_PID)my_syscall5(120 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#endif
}

static inline int l_execve(const char *path, char *const argv[], char *const envp[])
{
#if defined(__x86_64__)
    return (int)my_syscall3(59 /*__NR_execve*/, path, argv, envp);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall3(221 /*__NR_execve*/, path, argv, envp);
#elif defined(__arm__)
    return (int)my_syscall3(11 /*__NR_execve*/, path, argv, envp);
#endif
}

static inline L_PID l_waitpid(L_PID pid, int *status, int options)
{
#if defined(__x86_64__)
    return (L_PID)my_syscall4(61 /*__NR_wait4*/, pid, status, options, 0);
#elif defined(__aarch64__) || defined(__riscv)
    return (L_PID)my_syscall4(260 /*__NR_wait4*/, pid, status, options, 0);
#elif defined(__arm__)
    return (L_PID)my_syscall4(114 /*__NR_wait4*/, pid, status, options, 0);
#endif
}

static inline L_PID l_spawn_stdio(const char *path, char *const argv[], char *const envp[],
                           L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd)
{
    L_PID pid = l_fork();
    if (pid < 0) return -1;
    if (pid == 0) {
        static char *const empty_env[] = { (char *)0 };
        char *const *child_env = envp ? envp : l_envp;
        if (!child_env) child_env = empty_env;

        if (stdin_fd != L_SPAWN_INHERIT && stdin_fd != L_STDIN) {
            if (l_dup2(stdin_fd, L_STDIN) < 0) l_exit(127);
        }
        if (stdout_fd != L_SPAWN_INHERIT && stdout_fd != L_STDOUT) {
            if (l_dup2(stdout_fd, L_STDOUT) < 0) l_exit(127);
        }
        if (stderr_fd != L_SPAWN_INHERIT && stderr_fd != L_STDERR) {
            if (l_dup2(stderr_fd, L_STDERR) < 0) l_exit(127);
        }

        {
            L_FD child_fds[3] = { stdin_fd, stdout_fd, stderr_fd };
            int i, j;

            /* Close each original redirected fd exactly once in the child.
             * If multiple stdio streams intentionally reuse the same fd,
             * keep the dup2()-installed stdio endpoints and only close the
             * extra pre-dup descriptor once before execve(). */
            for (i = 0; i < 3; i++) {
                int seen = 0;
                if (child_fds[i] <= L_STDERR) continue;
                for (j = 0; j < i; j++) {
                    if (child_fds[j] == child_fds[i]) {
                        seen = 1;
                        break;
                    }
                }
                if (!seen)
                    l_close(child_fds[i]);
            }
        }

        l_execve(path, argv, (char *const *)child_env);
        l_exit(127);
    }
    return pid;
}

static inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[])
{
    return l_spawn_stdio(path, argv, envp, L_SPAWN_INHERIT, L_SPAWN_INHERIT, L_SPAWN_INHERIT);
}

static inline int l_wait(L_PID pid, int *exitcode)
{
    int status = 0;
    L_PID ret = l_waitpid(pid, &status, 0);
    if (ret < 0) return -1;
    if ((status & 0x7f) == 0)
        *exitcode = (status >> 8) & 0xff;
    else
        *exitcode = -1;
    return 0;
}

static inline off_t l_lseek(L_FD fd, off_t offset, int whence)
{
    return my_syscall3(__NR_lseek, fd, offset, whence);
}

static inline int l_truncate(const char *path, long long size)
{
#if defined(__x86_64__)
    return (int)my_syscall2(76 /*__NR_truncate*/, path, size);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall2(45 /*__NR_truncate*/, path, size);
#elif defined(__arm__)
    return (int)my_syscall2(193 /*__NR_truncate64*/, path, size);
#else
#error Unsupported architecture for l_truncate
#endif
}

static inline int l_ftruncate(L_FD fd, long long size)
{
#if defined(__x86_64__)
    return (int)my_syscall2(77 /*__NR_ftruncate*/, fd, size);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall2(46 /*__NR_ftruncate*/, fd, size);
#elif defined(__arm__)
    return (int)my_syscall2(194 /*__NR_ftruncate64*/, fd, size);
#else
#error Unsupported architecture for l_ftruncate
#endif
}

static inline int l_mkdir(const char *path, mode_t mode)
{
#ifdef __NR_mkdirat
    return my_syscall3(__NR_mkdirat, AT_FDCWD, path, mode);
#elif defined(__NR_mkdir)
    return my_syscall2(__NR_mkdir, path, mode);
#else
#error Neither __NR_mkdirat nor __NR_mkdir defined, cannot implement sys_mkdir()
#endif
}

#ifndef AT_REMOVEDIR
#define AT_REMOVEDIR 0x200
#endif

static inline int l_unlink(const char *path)
{
#ifdef __NR_unlinkat
    return my_syscall3(__NR_unlinkat, AT_FDCWD, path, 0);
#elif defined(__NR_unlink)
    return my_syscall1(__NR_unlink, path);
#else
#error Neither __NR_unlinkat nor __NR_unlink defined, cannot implement l_unlink()
#endif
}

static inline int l_rmdir(const char *path)
{
#ifdef __NR_unlinkat
    return my_syscall3(__NR_unlinkat, AT_FDCWD, path, AT_REMOVEDIR);
#elif defined(__NR_rmdir)
    return my_syscall1(__NR_rmdir, path);
#else
#error Neither __NR_unlinkat nor __NR_rmdir defined, cannot implement l_rmdir()
#endif
}

static inline int l_rename(const char *oldpath, const char *newpath)
{
#if defined(__x86_64__)
    return (int)my_syscall4(264 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#elif defined(__riscv)
    return (int)my_syscall5(276 /*__NR_renameat2*/, AT_FDCWD, oldpath, AT_FDCWD, newpath, 0);
#elif defined(__aarch64__)
    return (int)my_syscall4(38 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#elif defined(__arm__)
    return (int)my_syscall4(329 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#else
#error Unsupported architecture for l_rename
#endif
}

static inline int l_access(const char *path, int mode)
{
#if defined(__x86_64__)
    return (int)my_syscall4(269 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall4(48 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#elif defined(__arm__)
    return (int)my_syscall4(334 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#else
#error Unsupported architecture for l_access
#endif
}

static inline int l_chmod(const char *path, mode_t mode)
{
#if defined(__x86_64__)
    return (int)my_syscall3(268 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall3(53 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#elif defined(__arm__)
    return (int)my_syscall3(333 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#else
#error Unsupported architecture for l_chmod
#endif
}

static inline int l_symlink(const char *target, const char *linkpath)
{
#ifdef __NR_symlinkat
    long ret = my_syscall3(__NR_symlinkat, target, AT_FDCWD, linkpath);
#elif defined(__NR_symlink)
    long ret = my_syscall2(__NR_symlink, target, linkpath);
#else
    /* symlinkat syscall numbers by arch */
#if defined(__x86_64__)
    long ret = my_syscall3(265 /*__NR_symlinkat*/, target, AT_FDCWD, linkpath);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(36 /*__NR_symlinkat*/, target, AT_FDCWD, linkpath);
#elif defined(__arm__)
    long ret = my_syscall3(331 /*__NR_symlinkat*/, target, AT_FDCWD, linkpath);
#else
#error Unsupported architecture for l_symlink
#endif
#endif
    l_set_errno_from_ret(ret);
    return (int)ret;
}

static inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz)
{
#ifdef __NR_readlinkat
    long ret = my_syscall4(__NR_readlinkat, AT_FDCWD, path, buf, bufsiz);
#elif defined(__NR_readlink)
    long ret = my_syscall3(__NR_readlink, path, buf, bufsiz);
#else
#if defined(__x86_64__)
    long ret = my_syscall4(267 /*__NR_readlinkat*/, AT_FDCWD, path, buf, bufsiz);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall4(78 /*__NR_readlinkat*/, AT_FDCWD, path, buf, bufsiz);
#elif defined(__arm__)
    long ret = my_syscall4(332 /*__NR_readlinkat*/, AT_FDCWD, path, buf, bufsiz);
#else
#error Unsupported architecture for l_readlink
#endif
#endif
    l_set_errno_from_ret(ret);
    return (ptrdiff_t)ret;
}

static inline char *l_realpath(const char *path, char *resolved)
{
    /* Open the path with O_PATH (no actual I/O, just resolve) */
    long fd;
#ifdef __NR_openat
    fd = my_syscall4(__NR_openat, AT_FDCWD, path, 0x200000 /*O_PATH*/, 0);
#elif defined(__NR_open)
    fd = my_syscall3(__NR_open, path, 0x200000 /*O_PATH*/, 0);
#else
#if defined(__x86_64__)
    fd = my_syscall4(257 /*__NR_openat*/, AT_FDCWD, path, 0x200000 /*O_PATH*/, 0);
#elif defined(__aarch64__) || defined(__riscv)
    fd = my_syscall4(56 /*__NR_openat*/, AT_FDCWD, path, 0x200000 /*O_PATH*/, 0);
#elif defined(__arm__)
    fd = my_syscall4(322 /*__NR_openat*/, AT_FDCWD, path, 0x200000 /*O_PATH*/, 0);
#else
#error Unsupported architecture for l_realpath
#endif
#endif
    if (fd < 0) { l_set_errno_from_ret(fd); return (char *)0; }

    /* Read the resolved path from /proc/self/fd/<N> */
    char proc_path[64];
    /* Build "/proc/self/fd/<fd>" manually — l_snprintf not yet available */
    {
        const char *prefix = "/proc/self/fd/";
        int i = 0;
        while (prefix[i]) { proc_path[i] = prefix[i]; i++; }
        /* Convert fd number to string (fd is non-negative here) */
        char digits[20];
        int d = 0;
        long tmp = fd;
        if (tmp == 0) { digits[d++] = '0'; }
        else { while (tmp > 0) { digits[d++] = (char)('0' + (tmp % 10)); tmp /= 10; } }
        while (d > 0) { proc_path[i++] = digits[--d]; }
        proc_path[i] = '\0';
    }
    long n;
#if defined(__x86_64__)
    n = my_syscall4(267 /*__NR_readlinkat*/, AT_FDCWD, proc_path, resolved, L_PATH_MAX - 1);
#elif defined(__aarch64__) || defined(__riscv)
    n = my_syscall4(78 /*__NR_readlinkat*/, AT_FDCWD, proc_path, resolved, L_PATH_MAX - 1);
#elif defined(__arm__)
    n = my_syscall4(332 /*__NR_readlinkat*/, AT_FDCWD, proc_path, resolved, L_PATH_MAX - 1);
#else
#error Unsupported architecture for l_realpath
#endif
    my_syscall1(__NR_close, fd);
    if (n < 0) { l_set_errno_from_ret(n); return (char *)0; }
    resolved[n] = '\0';
    return resolved;
}

static inline int l_stat(const char *path, L_Stat *st)
{
#if defined(__x86_64__)
    char buf[144];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall4(262 /*__NR_newfstatat*/, AT_FDCWD, path, buf, 0);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 24);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long long *)(buf + 88);
    return 0;
#elif defined(__aarch64__) || defined(__riscv)
    char buf[144];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall4(79 /*__NR_newfstatat*/, AT_FDCWD, path, buf, 0);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 16);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long long *)(buf + 88);
    return 0;
#elif defined(__arm__)
    char buf[104];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall4(327 /*__NR_fstatat64*/, AT_FDCWD, path, buf, 0);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 16);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long *)(buf + 72);
    return 0;
#else
#error Unsupported architecture for l_stat
#endif
}

static inline int l_fstat(L_FD fd, L_Stat *st)
{
#if defined(__x86_64__)
    char buf[144];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall2(5 /*__NR_fstat*/, fd, buf);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 24);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long long *)(buf + 88);
    return 0;
#elif defined(__aarch64__) || defined(__riscv)
    char buf[144];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall2(80 /*__NR_fstat*/, fd, buf);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 16);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long long *)(buf + 88);
    return 0;
#elif defined(__arm__)
    char buf[104];
    l_memset(buf, 0, sizeof(buf));
    long ret = my_syscall2(197 /*__NR_fstat64*/, fd, buf);
    if (ret < 0) return -1;
    st->st_mode  = *(int *)(buf + 16);
    st->st_size  = *(long long *)(buf + 48);
    st->st_mtime = *(long *)(buf + 72);
    return 0;
#else
#error Unsupported architecture for l_fstat
#endif
}

static inline int l_opendir(const char *path, L_Dir *dir)
{
    // Use openat syscall directly to avoid forward-declaration issues
#define L_O_RDONLY    0
#define L_AT_FDCWD    (-100)
    // O_DIRECTORY differs by arch: x86_64/RISC-V=0x10000, ARM/AArch64=0x4000
#if defined(__x86_64__) || defined(__riscv)
#define L_O_DIRECTORY 0x10000
#elif defined(__aarch64__) || defined(__arm__)
#define L_O_DIRECTORY 0x4000
#endif
#if defined(__x86_64__)
    L_FD fd = (L_FD)my_syscall4(257 /*__NR_openat*/, L_AT_FDCWD, path, L_O_RDONLY | L_O_DIRECTORY, 0);
#elif defined(__aarch64__) || defined(__riscv)
    L_FD fd = (L_FD)my_syscall4(56 /*__NR_openat*/, L_AT_FDCWD, path, L_O_RDONLY | L_O_DIRECTORY, 0);
#elif defined(__arm__)
    L_FD fd = (L_FD)my_syscall4(322 /*__NR_openat*/, L_AT_FDCWD, path, L_O_RDONLY | L_O_DIRECTORY, 0);
#else
#error Unsupported architecture for l_opendir
#endif
    if (fd < 0) return -1;
    dir->fd  = fd;
    dir->pos = 0;
    dir->len = 0;
    return 0;
}

static inline L_DirEntry *l_readdir(L_Dir *dir)
{
    static L_DirEntry entry;
    for (;;) {
        if (dir->pos >= dir->len) {
#if defined(__x86_64__)
            long ret = my_syscall3(217 /*__NR_getdents64*/, dir->fd, dir->buf, sizeof(dir->buf));
#elif defined(__aarch64__) || defined(__riscv)
            long ret = my_syscall3(61 /*__NR_getdents64*/, dir->fd, dir->buf, sizeof(dir->buf));
#elif defined(__arm__)
            long ret = my_syscall3(217 /*__NR_getdents64*/, dir->fd, dir->buf, sizeof(dir->buf));
#else
#error Unsupported architecture for l_readdir
#endif
            if (ret <= 0) return (L_DirEntry *)0;
            dir->pos = 0;
            dir->len = (int)ret;
        }
        // Parse linux_dirent64: d_ino(8), d_off(8), d_reclen(2), d_type(1), d_name(variable)
        char *dp = dir->buf + dir->pos;
        unsigned short reclen = *(unsigned short *)(dp + 16);
        unsigned char d_type  = *(unsigned char *)(dp + 18);
        char *name = dp + 19;
        dir->pos += reclen;

        l_strncpy(entry.d_name, name, 255);
        entry.d_name[255] = '\0';
        if (d_type == 4)      entry.d_type = L_DT_DIR;
        else if (d_type == 8) entry.d_type = L_DT_REG;
        else                  entry.d_type = L_DT_UNKNOWN;
        return &entry;
    }
}

static inline void l_closedir(L_Dir *dir)
{
    l_close(dir->fd);
}

static inline void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset)
{
#if defined(__x86_64__)
    long ret = my_syscall6(9 /*__NR_mmap*/, addr, length, prot, flags, fd, offset);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(222 /*__NR_mmap*/, addr, length, prot, flags, fd, offset);
#elif defined(__arm__)
    long ret = my_syscall6(192 /*__NR_mmap2*/, addr, length, prot, flags, fd, (long)(offset >> 12));
#else
#error Unsupported architecture for l_mmap
#endif
    if (ret < 0 && ret > -4096) return L_MAP_FAILED;
    return (void *)ret;
}

static inline int l_munmap(void *addr, size_t length)
{
#if defined(__x86_64__)
    return (int)my_syscall2(11 /*__NR_munmap*/, addr, length);
#elif defined(__aarch64__) || defined(__riscv)
    return (int)my_syscall2(215 /*__NR_munmap*/, addr, length);
#elif defined(__arm__)
    return (int)my_syscall2(91 /*__NR_munmap*/, addr, length);
#else
#error Unsupported architecture for l_munmap
#endif
}

static inline L_FD l_open(const char *path, int flags, mode_t mode)
{
    long ret;
#ifdef __NR_openat
    ret = my_syscall4(__NR_openat, AT_FDCWD, path, flags, mode);
#elif defined(__NR_open)
    ret = my_syscall3(__NR_open, path, flags, mode);
#else
#error Neither __NR_openat nor __NR_open defined, cannot implement sys_open()
#endif
    l_set_errno_from_ret(ret);
    return ret;
}

static inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    long ret = my_syscall3(__NR_read, fd, buf, count);
    l_set_errno_from_ret(ret);
    return ret;
}

static inline int l_sched_yield(void)
{
    return my_syscall0(__NR_sched_yield);
}

static inline L_PID l_getpid(void)
{
    return (L_PID)my_syscall0(__NR_getpid);
}

static inline L_PID l_getppid(void)
{
    return (L_PID)my_syscall0(__NR_getppid);
}

static inline int l_kill(L_PID pid, int sig)
{
    long ret = my_syscall2(__NR_kill, (long)pid, sig);
    l_set_errno_from_ret(ret);
    return (int)(ret < 0 ? -1 : 0);
}

// Terminal and timing support for Unix
struct l_timespec {
    long tv_sec;
    long tv_nsec;
};

static inline void l_sleep_ms(unsigned int ms)
{
    struct l_timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    my_syscall2(__NR_nanosleep, &ts, (void*)0);
}

static inline long long l_time(long long *t) {
#if defined(__arm__)
    // ARM32: use clock_gettime64 (syscall 403) for Y2038 safety
    struct { long long tv_sec; long long tv_nsec; } ts64;
    long ret = my_syscall2(403, 0, (long)&ts64);
    if (ret < 0) { if (t) *t = 0; return 0; }
    long long val = ts64.tv_sec;
#else
    // x86_64: __NR_clock_gettime = 228, AArch64: __NR_clock_gettime = 113
    struct l_timespec ts;
    long ret = my_syscall2(__NR_clock_gettime, 0, (long)&ts);
    if (ret < 0) { if (t) *t = 0; return 0; }
    long long val = (long long)ts.tv_sec;
#endif
    if (t) *t = val;
    return val;
}

/// Fill buf with len bytes of cryptographic-quality random data. Returns 0 on success, -1 on error.
static inline int l_getrandom(void *buf, size_t len) {
    // getrandom(2) syscall numbers by architecture
#if defined(__x86_64__)
    long ret = my_syscall3(318 /*__NR_getrandom*/, (long)buf, (long)len, 0);
#elif defined(__aarch64__)
    long ret = my_syscall3(278 /*__NR_getrandom*/, (long)buf, (long)len, 0);
#elif defined(__arm__)
    long ret = my_syscall3(384 /*__NR_getrandom*/, (long)buf, (long)len, 0);
#elif defined(__riscv)
    long ret = my_syscall3(278 /*__NR_getrandom*/, (long)buf, (long)len, 0);
#else
    long ret = -1;
#endif
    return ret >= 0 ? 0 : -1;
}

// termios constants for raw mode
#define L_TCGETS   0x5401
#define L_TCSETS   0x5402
#define L_ICANON   0000002
#define L_ECHO     0000010
#define L_ISIG     0000001

struct l_termios {
    unsigned int c_iflag;
    unsigned int c_oflag;
    unsigned int c_cflag;
    unsigned int c_lflag;
    unsigned char c_line;
    unsigned char c_cc[19];
};

static struct l_termios l_saved_termios;

static inline unsigned long l_term_raw(void)
{
    struct l_termios raw;
    my_syscall3(__NR_ioctl, L_STDIN, L_TCGETS, &l_saved_termios);
    l_memcpy(&raw, &l_saved_termios, sizeof(raw));
    raw.c_lflag &= ~(L_ICANON | L_ECHO | L_ISIG);
    raw.c_cc[6] = 0;  // VMIN = 0 (non-blocking)
    raw.c_cc[5] = 0;  // VTIME = 0
    my_syscall3(__NR_ioctl, L_STDIN, L_TCSETS, &raw);
    return l_saved_termios.c_lflag;
}

static inline void l_term_restore(unsigned long old_mode)
{
    (void)old_mode;
    my_syscall3(__NR_ioctl, L_STDIN, L_TCSETS, &l_saved_termios);
}

static inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
{
    // With VMIN=0, VTIME=0 set by l_term_raw, read returns immediately
    return my_syscall3(__NR_read, fd, buf, count);
}

#define L_TIOCGWINSZ 0x5413
struct l_winsize { unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel; };

static inline void l_term_size(int *rows, int *cols)
{
    struct l_winsize ws;
    long ret = my_syscall3(__NR_ioctl, L_STDIN, L_TIOCGWINSZ, &ws);
    if (ret < 0) { *rows = 24; *cols = 80; }
    else { *rows = ws.ws_row; *cols = ws.ws_col; }
}

static inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    long ret = my_syscall3(__NR_write, fd, buf, count);
    l_set_errno_from_ret(ret);
    return ret;
}

static inline L_FD l_open_read(const char* file) {
    return l_open(file, O_RDONLY, 0);
}

static inline L_FD l_open_write(const char* file) {
    return l_open(file, O_WRONLY | O_CREAT, 0644);
}

static inline L_FD l_open_readwrite(const char* file) {
    return l_open(file, O_RDWR | O_CREAT, 0644);
}

static inline L_FD l_open_append(const char* file) {
    return l_open(file, O_WRONLY | O_APPEND | O_CREAT, 0600);
}

static inline L_FD l_open_trunc(const char* file) {
    return l_open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
}

#ifdef L_WITHSOCKETS

static inline L_SOCKET l_socket_tcp(void)
{
#if defined(__x86_64__)
    long ret = my_syscall3(41 /*__NR_socket*/, 2 /*AF_INET*/, 1 /*SOCK_STREAM*/, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(198 /*__NR_socket*/, 2, 1, 0);
#elif defined(__arm__)
    long ret = my_syscall3(281 /*__NR_socket*/, 2, 1, 0);
#endif
    if (ret < 0) return -1;
    return (L_SOCKET)ret;
}

static inline int l_socket_connect(L_SOCKET sock, const char *addr, int port)
{
    L_SockAddrIn sa;
    unsigned int ip;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = 2;
    sa.sin_port = l_htons((unsigned short)port);
    if (!l_parse_ipv4(addr, &ip)) return -1;
    sa.sin_addr = ip;
#if defined(__x86_64__)
    long ret = my_syscall3(42 /*__NR_connect*/, sock, (long)&sa, (long)sizeof(sa));
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(203 /*__NR_connect*/, sock, (long)&sa, (long)sizeof(sa));
#elif defined(__arm__)
    long ret = my_syscall3(284 /*__NR_connect*/, sock, (long)&sa, (long)sizeof(sa));
#endif
    return (int)(ret < 0 ? -1 : 0);
}

static inline int l_socket_bind(L_SOCKET sock, int port)
{
    L_SockAddrIn sa;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = 2;
    sa.sin_port = l_htons((unsigned short)port);
    sa.sin_addr = 0;
#if defined(__x86_64__)
    long ret = my_syscall3(49 /*__NR_bind*/, sock, (long)&sa, (long)sizeof(sa));
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(200 /*__NR_bind*/, sock, (long)&sa, (long)sizeof(sa));
#elif defined(__arm__)
    long ret = my_syscall3(283 /*__NR_bind*/, sock, (long)&sa, (long)sizeof(sa));
#endif
    return (int)(ret < 0 ? -1 : 0);
}

static inline int l_socket_listen(L_SOCKET sock, int backlog)
{
#if defined(__x86_64__)
    long ret = my_syscall2(50 /*__NR_listen*/, sock, backlog);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall2(201 /*__NR_listen*/, sock, backlog);
#elif defined(__arm__)
    long ret = my_syscall2(285 /*__NR_listen*/, sock, backlog);
#endif
    return (int)(ret < 0 ? -1 : 0);
}

static inline L_SOCKET l_socket_accept(L_SOCKET sock)
{
#if defined(__x86_64__)
    long ret = my_syscall3(43 /*__NR_accept*/, sock, 0, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(202 /*__NR_accept*/, sock, 0, 0);
#elif defined(__arm__)
    long ret = my_syscall4(286 /*__NR_accept4*/, sock, 0, 0, 0);
#endif
    if (ret < 0) return -1;
    return (L_SOCKET)ret;
}

static inline ptrdiff_t l_socket_send(L_SOCKET sock, const void *data, size_t len)
{
#if defined(__x86_64__)
    long ret = my_syscall6(44 /*__NR_sendto*/, sock, (long)data, (long)len, 0, 0, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(206 /*__NR_sendto*/, sock, (long)data, (long)len, 0, 0, 0);
#elif defined(__arm__)
    long ret = my_syscall6(291 /*__NR_sendto*/, sock, (long)data, (long)len, 0, 0, 0);
#endif
    if (ret < 0) return -1;
    return (ptrdiff_t)ret;
}

static inline ptrdiff_t l_socket_recv(L_SOCKET sock, void *buf, size_t len)
{
#if defined(__x86_64__)
    long ret = my_syscall6(45 /*__NR_recvfrom*/, sock, (long)buf, (long)len, 0, 0, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(207 /*__NR_recvfrom*/, sock, (long)buf, (long)len, 0, 0, 0);
#elif defined(__arm__)
    long ret = my_syscall6(293 /*__NR_recvfrom*/, sock, (long)buf, (long)len, 0, 0, 0);
#endif
    if (ret < 0) return -1;
    return (ptrdiff_t)ret;
}

static inline void l_socket_close(L_SOCKET sock)
{
    my_syscall1(__NR_close, sock);
}

static inline L_SOCKET l_socket_udp(void)
{
#if defined(__x86_64__)
    long ret = my_syscall3(41 /*__NR_socket*/, 2 /*AF_INET*/, 2 /*SOCK_DGRAM*/, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(198 /*__NR_socket*/, 2, 2, 0);
#elif defined(__arm__)
    long ret = my_syscall3(281 /*__NR_socket*/, 2, 2, 0);
#endif
    if (ret < 0) return -1;
    return (L_SOCKET)ret;
}

static inline ptrdiff_t l_socket_sendto(L_SOCKET s, const void *data, size_t len, const char *addr, int port)
{
    L_SockAddrIn sa;
    unsigned int ip;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = 2;
    sa.sin_port = l_htons((unsigned short)port);
    if (!l_parse_ipv4(addr, &ip)) return -1;
    sa.sin_addr = ip;
#if defined(__x86_64__)
    long ret = my_syscall6(44 /*__NR_sendto*/, s, (long)data, (long)len, 0, (long)&sa, (long)sizeof(sa));
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(206 /*__NR_sendto*/, s, (long)data, (long)len, 0, (long)&sa, (long)sizeof(sa));
#elif defined(__arm__)
    long ret = my_syscall6(291 /*__NR_sendto*/, s, (long)data, (long)len, 0, (long)&sa, (long)sizeof(sa));
#endif
    if (ret < 0) return -1;
    return (ptrdiff_t)ret;
}

static inline ptrdiff_t l_socket_recvfrom(L_SOCKET s, void *buf, size_t len, char *addr_out, int *port_out)
{
    L_SockAddrIn sa;
    long salen = sizeof(sa);
    l_memset(&sa, 0, sizeof(sa));
#if defined(__x86_64__)
    long ret = my_syscall6(45 /*__NR_recvfrom*/, s, (long)buf, (long)len, 0, (long)&sa, (long)&salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(207 /*__NR_recvfrom*/, s, (long)buf, (long)len, 0, (long)&sa, (long)&salen);
#elif defined(__arm__)
    long ret = my_syscall6(293 /*__NR_recvfrom*/, s, (long)buf, (long)len, 0, (long)&sa, (long)&salen);
#endif
    if (ret < 0) return -1;
    if (addr_out) {
        unsigned int ip = sa.sin_addr;
        int pos = 0;
        for (int i = 0; i < 4; i++) {
            unsigned int octet = (ip >> (i * 8)) & 0xFF;
            if (octet >= 100) addr_out[pos++] = (char)('0' + octet / 100);
            if (octet >= 10) addr_out[pos++] = (char)('0' + (octet / 10) % 10);
            addr_out[pos++] = (char)('0' + octet % 10);
            if (i < 3) addr_out[pos++] = '.';
        }
        addr_out[pos] = '\0';
    }
    if (port_out) *port_out = (int)l_htons(sa.sin_port);
    return (ptrdiff_t)ret;
}

// --- Generic address-based socket API (Linux) ---

static inline int l_sockaddr_to_os(const L_SockAddr *sa, void *out, int *outlen) {
    if (sa->family == L_AF_INET) {
        L_SockAddrIn *s = (L_SockAddrIn *)out;
        l_memset(s, 0, sizeof(*s));
        s->sin_family = 2;
        s->sin_port = l_htons(sa->port);
        l_memcpy(&s->sin_addr, sa->addr, 4);
        *outlen = (int)sizeof(L_SockAddrIn);
        return 0;
    }
    if (sa->family == L_AF_INET6) {
        L_SockAddrIn6 *s = (L_SockAddrIn6 *)out;
        l_memset(s, 0, sizeof(*s));
        s->sin6_family = 10;
        s->sin6_port = l_htons(sa->port);
        l_memcpy(s->sin6_addr, sa->addr, 16);
        *outlen = (int)sizeof(L_SockAddrIn6);
        return 0;
    }
    return -1;
}

static inline void l_sockaddr_from_os4(L_SockAddr *sa, const L_SockAddrIn *s) {
    l_memset(sa, 0, sizeof(*sa));
    sa->family = L_AF_INET;
    sa->port = l_htons(s->sin_port);
    l_memcpy(sa->addr, &s->sin_addr, 4);
}

static inline void l_sockaddr_from_os6(L_SockAddr *sa, const L_SockAddrIn6 *s) {
    l_memset(sa, 0, sizeof(*sa));
    sa->family = L_AF_INET6;
    sa->port = l_htons(s->sin6_port);
    l_memcpy(sa->addr, s->sin6_addr, 16);
}

static inline L_SOCKET l_socket_open(int family, int type) {
    int af = (family == L_AF_UNIX) ? 1 : (family == L_AF_INET6) ? 10 : 2;
    int st = (type == L_SOCK_DGRAM) ? 2 : 1;
#if defined(__x86_64__)
    long ret = my_syscall3(41, af, st, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(198, af, st, 0);
#elif defined(__arm__)
    long ret = my_syscall3(281, af, st, 0);
#endif
    return (L_SOCKET)(ret < 0 ? -1 : ret);
}

static inline int l_socket_connect_addr(L_SOCKET sock, const L_SockAddr *addr) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_os(addr, osbuf, &salen) < 0) return -1;
#if defined(__x86_64__)
    long ret = my_syscall3(42, sock, (long)osbuf, salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(203, sock, (long)osbuf, salen);
#elif defined(__arm__)
    long ret = my_syscall3(283, sock, (long)osbuf, salen);
#endif
    return (int)(ret < 0 ? -1 : 0);
}

static inline int l_socket_bind_addr(L_SOCKET sock, const L_SockAddr *addr) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_os(addr, osbuf, &salen) < 0) return -1;
#if defined(__x86_64__)
    long ret = my_syscall3(49, sock, (long)osbuf, salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(200, sock, (long)osbuf, salen);
#elif defined(__arm__)
    long ret = my_syscall3(282, sock, (long)osbuf, salen);
#endif
    return (int)(ret < 0 ? -1 : 0);
}

static inline ptrdiff_t l_socket_sendto_addr(L_SOCKET s, const void *data, size_t len, const L_SockAddr *dest) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_os(dest, osbuf, &salen) < 0) return -1;
#if defined(__x86_64__)
    long ret = my_syscall6(44, s, (long)data, len, 0, (long)osbuf, salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(206, s, (long)data, len, 0, (long)osbuf, salen);
#elif defined(__arm__)
    long ret = my_syscall6(290, s, (long)data, len, 0, (long)osbuf, salen);
#endif
    return (ptrdiff_t)(ret < 0 ? -1 : ret);
}

static inline ptrdiff_t l_socket_recvfrom_addr(L_SOCKET s, void *buf, size_t len, L_SockAddr *src) {
    char sabuf[28];
    long salen = sizeof(sabuf);
    l_memset(sabuf, 0, sizeof(sabuf));
#if defined(__x86_64__)
    long ret = my_syscall6(45, s, (long)buf, len, 0, (long)sabuf, (long)&salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall6(207, s, (long)buf, len, 0, (long)sabuf, (long)&salen);
#elif defined(__arm__)
    long ret = my_syscall6(292, s, (long)buf, len, 0, (long)sabuf, (long)&salen);
#endif
    if (ret < 0) return -1;
    if (src) {
        unsigned short fam;
        l_memcpy(&fam, sabuf, 2);
        if (fam == 10) l_sockaddr_from_os6(src, (const L_SockAddrIn6 *)sabuf);
        else l_sockaddr_from_os4(src, (const L_SockAddrIn *)sabuf);
    }
    return (ptrdiff_t)ret;
}

static inline L_SOCKET l_socket_unix_connect(const char *path) {
    /* sockaddr_un: uint16_t sun_family + char sun_path[108] = 110 bytes */
    char sa[110];
    l_memset(sa, 0, sizeof(sa));
    unsigned short fam = 1; /* AF_UNIX */
    l_memcpy(sa, &fam, 2);
    size_t plen = l_strlen(path);
    if (plen >= 108) return -1;
    l_memcpy(sa + 2, path, plen);
    int salen = (int)(2 + plen + 1);
    /* Create socket */
#if defined(__x86_64__)
    long sfd = my_syscall3(41, 1 /*AF_UNIX*/, 1 /*SOCK_STREAM*/, 0);
#elif defined(__aarch64__) || defined(__riscv)
    long sfd = my_syscall3(198, 1, 1, 0);
#elif defined(__arm__)
    long sfd = my_syscall3(281, 1, 1, 0);
#endif
    if (sfd < 0) return -1;
    /* Connect */
#if defined(__x86_64__)
    long ret = my_syscall3(42, sfd, (long)sa, salen);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall3(203, sfd, (long)sa, salen);
#elif defined(__arm__)
    long ret = my_syscall3(283, sfd, (long)sa, salen);
#endif
    if (ret < 0) { my_syscall1(__NR_close, sfd); return -1; }
    return (L_SOCKET)sfd;
}

#endif // L_WITHSOCKETS
static inline int l_poll(L_PollFd *fds, int nfds, int timeout_ms)
{
    // Kernel pollfd layout: int fd, short events, short revents
    // But L_FD is ptrdiff_t (8 bytes on 64-bit), so we need to convert
    struct { int fd; short events; short revents; } kfds[64];
    if (nfds > 64) nfds = 64;
    for (int i = 0; i < nfds; i++) {
        kfds[i].fd = (int)fds[i].fd;
        kfds[i].events = fds[i].events;
        kfds[i].revents = 0;
    }
#if defined(__x86_64__)
    long ret = my_syscall3(7 /*__NR_poll*/, (long)kfds, nfds, timeout_ms);
#elif defined(__arm__)
    long ret = my_syscall3(168 /*__NR_poll*/, (long)kfds, nfds, timeout_ms);
#elif defined(__aarch64__) || defined(__riscv)
    // AArch64/RISC-V have no poll syscall — use ppoll with NULL sigmask
    struct l_timespec ts;
    struct l_timespec *tsp = (void *)0;
    if (timeout_ms >= 0) {
        ts.tv_sec = timeout_ms / 1000;
        ts.tv_nsec = (long)(timeout_ms % 1000) * 1000000L;
        tsp = &ts;
    }
    long ret = my_syscall5(73 /*__NR_ppoll*/, (long)kfds, nfds, (long)tsp, 0, 0);
#endif
    for (int i = 0; i < nfds; i++)
        fds[i].revents = kfds[i].revents;
    l_set_errno_from_ret(ret);
    return (int)ret;
}

// l_signal — signal handling via rt_sigaction
static inline L_SigHandler l_signal(int sig, L_SigHandler handler)
{
    struct {
        void (*handler)(int);
        unsigned long flags;
        void (*restorer)(void);
        unsigned long mask[2]; // 128-bit mask for 64 signals
    } sa, old_sa;
    l_memset(&sa, 0, sizeof(sa));
    l_memset(&old_sa, 0, sizeof(old_sa));
    sa.handler = handler;
    sa.flags = 0; // no SA_RESTORER — kernel handles return
#if defined(__x86_64__)
    long ret = my_syscall4(13 /*__NR_rt_sigaction*/, sig, (long)&sa, (long)&old_sa, 8);
#elif defined(__aarch64__) || defined(__riscv)
    long ret = my_syscall4(134 /*__NR_rt_sigaction*/, sig, (long)&sa, (long)&old_sa, 8);
#elif defined(__arm__)
    long ret = my_syscall4(174 /*__NR_rt_sigaction*/, sig, (long)&sa, (long)&old_sa, 8);
#endif
    if (ret < 0) return L_SIG_DFL;
    return old_sa.handler;
}

// l_writev — scatter write
static inline ptrdiff_t l_writev(L_FD fd, const L_IoVec *iov, int iovcnt)
{
    // Kernel iovec: { void *base; size_t len } — same layout as L_IoVec
    long ret = my_syscall3(__NR_writev, fd, (long)iov, iovcnt);
    l_set_errno_from_ret(ret);
    return (ptrdiff_t)ret;
}

// l_readv — scatter read
static inline ptrdiff_t l_readv(L_FD fd, L_IoVec *iov, int iovcnt)
{
    long ret = my_syscall3(__NR_readv, fd, (long)iov, iovcnt);
    l_set_errno_from_ret(ret);
    return (ptrdiff_t)ret;
}

// l_isatty — check if fd is a terminal
static inline int l_isatty(L_FD fd)
{
    char buf[64]; // termios structure (we don't need the contents)
    long ret = my_syscall3(__NR_ioctl, fd, L_TCGETS, (long)buf);
    return ret == 0 ? 1 : 0;
}

#pragma GCC diagnostic pop

#elif defined(__wasi__)
// ============================================================
// WASI (WebAssembly System Interface) — experimental support
// ============================================================
// WASI uses imported host functions instead of inline asm syscalls.
// Types and imports are defined at file scope (near top of file).
// Preopened directory fd is L_WASI_PREOPEN_FD (default 3, --dir .).
// Run with: wasmtime --dir . program.wasm

// ABI layout verification
_Static_assert(sizeof(__wasi_filestat_t) == 64, "wasi_filestat_t must be 64 bytes");
_Static_assert(sizeof(__wasi_dirent_t) == 24, "wasi_dirent_t must be 24 bytes");

// WASI errno values (from WASI spec) differ from POSIX L_E* constants.
// Map WASI errno to L_E* so callers can check error codes portably.
static inline int l_wasi_errno_map(__wasi_errno_t e) {
    switch (e) {
        case  0: return 0;
        case  2: return L_EACCES;   // __WASI_ERRNO_ACCES
        case  8: return L_EBADF;    // __WASI_ERRNO_BADF
        case 17: return L_EEXIST;   // __WASI_ERRNO_EXIST
        case 22: return L_EINVAL;   // __WASI_ERRNO_INVAL
        case 28: return L_EISDIR;   // __WASI_ERRNO_ISDIR
        case 36: return L_ENOMEM;   // __WASI_ERRNO_NOMEM
        case 44: return L_ENOENT;   // __WASI_ERRNO_NOENT
        case 54: return L_ENOTDIR;  // __WASI_ERRNO_NOTDIR
        case 55: return L_ENOTEMPTY;// __WASI_ERRNO_NOTEMPTY
        case 58: return L_ENOSPC;   // __WASI_ERRNO_NOSPC
        case  6: return L_EAGAIN;   // __WASI_ERRNO_AGAIN
        default: return (int)e;     // pass through unknown
    }
}

static inline int l_wasi_err(__wasi_errno_t e) {
    if (e == 0) return 0;
    l_set_errno(l_wasi_errno_map(e));
    return -1;
}

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_RDONLY    0x0
#define O_WRONLY    0x1
#define O_RDWR      0x2
#define O_CREAT     0x40
#define O_EXCL      0x80
#define O_NOCTTY    0x100
#define O_TRUNC     0x200
#define O_APPEND    0x400
#define O_NONBLOCK  0x800
#define O_CLOEXEC   0x80000
#define O_DIRECTORY 0x4000

#define AT_FDCWD (-100)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#ifndef L_WITHSTART
static char **l_envp;
#endif

// --- Core I/O functions ---

static noreturn inline void l_exit(int status)
{
    __wasi_proc_exit((unsigned)(status & 255));
    __builtin_unreachable();
}

static inline int l_chdir(const char *path)
{
    (void)path;
    return -1; // @stub design: WASI has no chdir
}

static inline char *l_getcwd(char *buf, size_t size)
{
    (void)size;
    if (buf && size > 1) { buf[0] = '.'; buf[1] = '\0'; }
    return buf;
}

static inline int l_close(L_FD fd)
{
    __wasi_errno_t err = __wasi_fd_close((__wasi_fd_t)fd);
    if (err != 0) { l_set_errno((int)err); return -1; }
    return 0;
}

static inline int l_dup(L_FD fd)
{
    (void)fd;
    return -1; // @stub design: WASI preview 1 has no dup
}

static inline int l_pipe(L_FD fds[2])
{
    (void)fds;
    return -1; // @stub design: WASI has no pipes
}

static inline int l_dup2(L_FD oldfd, L_FD newfd)
{
    (void)oldfd; (void)newfd;
    return -1; // @stub design: WASI preview 1 has no dup2
}

static inline L_PID l_fork(void)
{
    return -1; // @stub design: WASI has no process creation
}

static inline int l_execve(const char *path, char *const argv[], char *const envp[])
{
    (void)path; (void)argv; (void)envp;
    return -1; // @stub design: WASI has no exec
}

static inline L_PID l_waitpid(L_PID pid, int *status, int options)
{
    (void)pid; (void)status; (void)options;
    return -1; // @stub design: WASI has no process management
}

static inline L_PID l_spawn_stdio(const char *path, char *const argv[], char *const envp[],
                           L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd)
{
    (void)path; (void)argv; (void)envp;
    (void)stdin_fd; (void)stdout_fd; (void)stderr_fd;
    return -1; // @stub design: WASI has no process creation
}

static inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[])
{
    (void)path; (void)argv; (void)envp;
    return -1; // @stub design: WASI has no process creation
}

static inline int l_wait(L_PID pid, int *exitcode)
{
    (void)pid; (void)exitcode;
    return -1; // @stub design: WASI has no process management
}

static inline off_t l_lseek(L_FD fd, off_t offset, int whence)
{
    __wasi_whence_t w;
    switch (whence) {
        case SEEK_SET: w = __WASI_WHENCE_SET; break;
        case SEEK_CUR: w = __WASI_WHENCE_CUR; break;
        case SEEK_END: w = __WASI_WHENCE_END; break;
        default: return -1;
    }
    __wasi_filesize_t newoffset = 0;
    __wasi_errno_t err = __wasi_fd_seek((__wasi_fd_t)fd, (__wasi_filedelta_t)offset, w, &newoffset);
    if (err != 0) { l_set_errno((int)err); return -1; }
    return (off_t)newoffset;
}

static inline int l_truncate(const char *path, long long size)
{
    if (size < 0) { l_set_errno(L_EINVAL); return -1; }
    // Open the file, set size, close
    __wasi_fd_t fd;
    __wasi_errno_t err = __wasi_path_open(L_WASI_PREOPEN_FD,
        __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW, path, l_strlen(path),
        0, __WASI_RIGHTS_FD_FILESTAT_SET_SIZE, 0, 0, &fd);
    if (err != 0) return l_wasi_err(err);
    err = __wasi_fd_filestat_set_size(fd, (__wasi_filesize_t)size);
    __wasi_fd_close(fd);
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_ftruncate(L_FD fd, long long size)
{
    if (size < 0) { l_set_errno(L_EINVAL); return -1; }
    __wasi_errno_t err = __wasi_fd_filestat_set_size((__wasi_fd_t)fd, (__wasi_filesize_t)size);
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_mkdir(const char *path, mode_t mode)
{
    (void)mode;
    __wasi_errno_t err = __wasi_path_create_directory(L_WASI_PREOPEN_FD, path, l_strlen(path));
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_unlink(const char *path)
{
    __wasi_errno_t err = __wasi_path_unlink_file(L_WASI_PREOPEN_FD, path, l_strlen(path));
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_rmdir(const char *path)
{
    __wasi_errno_t err = __wasi_path_remove_directory(L_WASI_PREOPEN_FD, path, l_strlen(path));
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_rename(const char *oldpath, const char *newpath)
{
    __wasi_errno_t err = __wasi_path_rename(L_WASI_PREOPEN_FD, oldpath, l_strlen(oldpath),
                                            L_WASI_PREOPEN_FD, newpath, l_strlen(newpath));
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline int l_access(const char *path, int mode)
{
    // Emulate via path_filestat_get — F_OK is reliable, permission bits are approximate
    __wasi_filestat_t fs;
    __wasi_errno_t err = __wasi_path_filestat_get(L_WASI_PREOPEN_FD,
        __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW, path, l_strlen(path), &fs);
    if (err != 0) return l_wasi_err(err);
    if (mode == L_F_OK) return 0;
    // WASI doesn't expose per-file permissions; assume accessible if it exists
    return 0;
}

static inline int l_chmod(const char *path, mode_t mode)
{
    (void)path; (void)mode;
    return -1; // @stub design: WASI has no chmod
}

static inline int l_symlink(const char *target, const char *linkpath)
{
    __wasi_errno_t err = __wasi_path_symlink(target, l_strlen(target),
                                             L_WASI_PREOPEN_FD, linkpath, l_strlen(linkpath));
    if (err != 0) return l_wasi_err(err);
    return 0;
}

static inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz)
{
    __wasi_size_t_w used = 0;
    __wasi_errno_t err = __wasi_path_readlink(L_WASI_PREOPEN_FD,
        path, l_strlen(path), (uint8_t *)buf, (__wasi_size_t_w)bufsiz, &used);
    if (err != 0) { l_set_errno(l_wasi_errno_map(err)); return -1; }
    return (ptrdiff_t)used;
}

static inline char *l_realpath(const char *path, char *resolved)
{
    if (!path || !resolved) return (char *)0;
    // Minimal: just copy path as-is (no /proc/self/fd on WASI)
    size_t len = l_strlen(path);
    l_memcpy(resolved, path, len + 1);
    return resolved;
}

// Map WASI filestat to L_Stat
static inline void l_wasi_filestat_to_l_stat(const __wasi_filestat_t *fs, L_Stat *st)
{
    st->st_size = (long long)fs->size;
    st->st_mtime = (long long)(fs->mtim / 1000000000ULL); // nanoseconds → seconds
    // Map filetype to st_mode with reasonable default permissions
    switch (fs->filetype) {
        case __WASI_FILETYPE_DIRECTORY:     st->st_mode = L_S_IFDIR | 0755; break;
        case __WASI_FILETYPE_SYMBOLIC_LINK: st->st_mode = L_S_IFLNK | 0777; break;
        case __WASI_FILETYPE_REGULAR_FILE:  st->st_mode = L_S_IFREG | 0644; break;
        default:                            st->st_mode = L_S_IFREG | 0644; break;
    }
}

static inline int l_stat(const char *path, L_Stat *st)
{
    __wasi_filestat_t fs;
    __wasi_errno_t err = __wasi_path_filestat_get(L_WASI_PREOPEN_FD,
        __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW, path, l_strlen(path), &fs);
    if (err != 0) return l_wasi_err(err);
    l_wasi_filestat_to_l_stat(&fs, st);
    return 0;
}

static inline int l_fstat(L_FD fd, L_Stat *st)
{
    __wasi_filestat_t fs;
    __wasi_errno_t err = __wasi_fd_filestat_get((__wasi_fd_t)fd, &fs);
    if (err != 0) return l_wasi_err(err);
    l_wasi_filestat_to_l_stat(&fs, st);
    return 0;
}

static inline int l_opendir(const char *path, L_Dir *dir)
{
    __wasi_fd_t fd;
    __wasi_errno_t err = __wasi_path_open(L_WASI_PREOPEN_FD,
        __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW, path, l_strlen(path),
        __WASI_OFLAGS_DIRECTORY,
        __WASI_RIGHTS_FD_READDIR | __WASI_RIGHTS_FD_READ,
        0, 0, &fd);
    if (err != 0) return l_wasi_err(err);
    dir->fd = (L_FD)fd;
    dir->pos = 0;
    dir->len = 0;
    dir->cookie = 0;
    dir->done = 0;
    return 0;
}

static L_DirEntry l__wasi_dirent;

static inline L_DirEntry *l_readdir(L_Dir *dir)
{
    for (;;) {
        // Try to read an entry from the current buffer
        if (dir->pos < dir->len) {
            int remaining = dir->len - dir->pos;
            if (remaining < (int)sizeof(__wasi_dirent_t)) {
                // Partial header — need to refill from this cookie
                dir->pos = dir->len; // force refill
                continue;
            }
            __wasi_dirent_t de;
            l_memcpy(&de, dir->buf + dir->pos, sizeof(__wasi_dirent_t));
            int entry_size = (int)sizeof(__wasi_dirent_t) + (int)de.d_namlen;
            if (dir->pos + entry_size > dir->len) {
                // Partial name — advance cookie and refill
                dir->cookie = de.d_next;
                dir->pos = dir->len; // force refill
                continue;
            }
            // Full entry available — copy name
            int namlen = (int)de.d_namlen;
            if (namlen > 255) namlen = 255;
            l_memcpy(l__wasi_dirent.d_name, dir->buf + dir->pos + sizeof(__wasi_dirent_t), namlen);
            l__wasi_dirent.d_name[namlen] = '\0';
            // Map filetype
            switch (de.d_type) {
                case __WASI_FILETYPE_DIRECTORY:     l__wasi_dirent.d_type = L_DT_DIR; break;
                case __WASI_FILETYPE_REGULAR_FILE:  l__wasi_dirent.d_type = L_DT_REG; break;
                case __WASI_FILETYPE_SYMBOLIC_LINK: l__wasi_dirent.d_type = L_DT_LNK; break;
                default:                            l__wasi_dirent.d_type = L_DT_UNKNOWN; break;
            }
            dir->pos += entry_size;
            dir->cookie = de.d_next;
            return &l__wasi_dirent;
        }
        // Buffer exhausted — refill
        if (dir->done) return (L_DirEntry *)0;
        __wasi_size_t_w used = 0;
        __wasi_errno_t err = __wasi_fd_readdir((__wasi_fd_t)dir->fd,
            dir->buf, (__wasi_size_t_w)sizeof(dir->buf),
            dir->cookie, &used);
        if (err != 0) return (L_DirEntry *)0;
        if (used == 0) { dir->done = 1; return (L_DirEntry *)0; }
        // If less than buffer size returned, this is the last batch
        if (used < (__wasi_size_t_w)sizeof(dir->buf)) dir->done = 1;
        dir->pos = 0;
        dir->len = (int)used;
    }
}

static inline void l_closedir(L_Dir *dir)
{
    __wasi_fd_close((__wasi_fd_t)dir->fd);
}

// l_mmap — WASI: use WebAssembly memory.grow for anonymous mappings
static inline void *l_mmap(void *addr, size_t length, int prot, int flags,
                    L_FD fd, long long offset)
{
    (void)addr; (void)prot; (void)offset;
    if (!(flags & L_MAP_ANONYMOUS) || fd >= 0) return L_MAP_FAILED;
    // WebAssembly pages are 64KB
    size_t pages = (length + 65535) / 65536;
    size_t old_pages = __builtin_wasm_memory_grow(0, pages);
    if (old_pages == (size_t)-1) return L_MAP_FAILED;
    return (void *)(old_pages * 65536);
}

static inline int l_munmap(void *addr, size_t length)
{
    (void)addr; (void)length;
    return 0; // WebAssembly memory cannot be freed
}

static inline L_FD l_open(const char *path, int flags, mode_t mode)
{
    (void)mode;
    __wasi_oflags_t oflags = 0;
    __wasi_fdflags_t fdflags = 0;
    __wasi_rights_t rights_base = 0;

    if (flags & O_CREAT)     oflags |= __WASI_OFLAGS_CREAT;
    if (flags & O_EXCL)      oflags |= __WASI_OFLAGS_EXCL;
    if (flags & O_TRUNC)     oflags |= __WASI_OFLAGS_TRUNC;
    if (flags & O_DIRECTORY)  oflags |= __WASI_OFLAGS_DIRECTORY;
    if (flags & O_APPEND)    fdflags |= __WASI_FDFLAGS_APPEND;
    if (flags & O_NONBLOCK)  fdflags |= __WASI_FDFLAGS_NONBLOCK;

    int access_mode = flags & 3;
    if (access_mode == O_RDONLY || access_mode == O_RDWR)
        rights_base |= __WASI_RIGHTS_FD_READ | __WASI_RIGHTS_FD_SEEK |
                        __WASI_RIGHTS_FD_TELL | __WASI_RIGHTS_FD_READDIR |
                        __WASI_RIGHTS_FD_FILESTAT_GET;
    if (access_mode == O_WRONLY || access_mode == O_RDWR)
        rights_base |= __WASI_RIGHTS_FD_WRITE | __WASI_RIGHTS_FD_SEEK |
                        __WASI_RIGHTS_FD_TELL | __WASI_RIGHTS_FD_ALLOCATE |
                        __WASI_RIGHTS_FD_FILESTAT_SET_SIZE | __WASI_RIGHTS_FD_FILESTAT_GET;
    rights_base |= __WASI_RIGHTS_FD_DATASYNC | __WASI_RIGHTS_FD_SYNC |
                   __WASI_RIGHTS_FD_FDSTAT_SET_FLAGS;

    __wasi_fd_t opened_fd;
    __wasi_errno_t err = __wasi_path_open(
        L_WASI_PREOPEN_FD,
        __WASI_LOOKUPFLAGS_SYMLINK_FOLLOW,
        path, l_strlen(path),
        oflags, rights_base, rights_base,
        fdflags, &opened_fd);
    if (err != 0) return l_wasi_err(err);
    return (L_FD)opened_fd;
}

static inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    __wasi_iovec_t iov;
    iov.buf = (uint8_t *)buf;
    iov.buf_len = (uint32_t)count;
    __wasi_size_t_w nread = 0;
    __wasi_errno_t err = __wasi_fd_read((__wasi_fd_t)fd, &iov, 1, &nread);
    if (err != 0) { l_set_errno((int)err); return -1; }
    return (ssize_t)nread;
}

static inline int l_sched_yield(void)
{
    return 0; // WASI has no yield, just return success
}

static inline L_PID l_getpid(void)
{
    return 0; // WASI has no process IDs
}

static inline L_PID l_getppid(void)
{
    return 0;
}

static inline int l_kill(L_PID pid, int sig)
{
    (void)pid; (void)sig;
    return -1; // @stub design: WASI has no signals
}

static inline void l_sleep_ms(unsigned int ms)
{
    // WASI preview 1 has no sleep; busy-wait using clock_time_get
    __wasi_timestamp_t start, now;
    __wasi_clock_time_get(__WASI_CLOCKID_MONOTONIC, 1000, &start);
    __wasi_timestamp_t target = start + ((__wasi_timestamp_t)ms * 1000000ULL);
    do {
        __wasi_clock_time_get(__WASI_CLOCKID_MONOTONIC, 1000, &now);
    } while (now < target);
}

static inline long long l_time(long long *t) {
    __wasi_timestamp_t ts = 0;
    __wasi_errno_t err = __wasi_clock_time_get(__WASI_CLOCKID_REALTIME, 1000000000ULL, &ts);
    long long val = (err == 0) ? (long long)(ts / 1000000000ULL) : 0;
    if (t) *t = val;
    return val;
}

static inline unsigned long l_term_raw(void)
{
    return 0; // WASI has no terminal control
}

static inline void l_term_restore(unsigned long old_mode)
{
    (void)old_mode;
}

static inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
{
    // WASI has no non-blocking I/O; fall back to regular read
    return l_read(fd, buf, count);
}

static inline void l_term_size(int *rows, int *cols)
{
    *rows = 24;
    *cols = 80; // WASI has no terminal; return defaults
}

static inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    __wasi_ciovec_t iov;
    iov.buf = (const uint8_t *)buf;
    iov.buf_len = (uint32_t)count;
    __wasi_size_t_w nwritten = 0;
    __wasi_errno_t err = __wasi_fd_write((__wasi_fd_t)fd, &iov, 1, &nwritten);
    if (err != 0) { l_set_errno((int)err); return -1; }
    return (ssize_t)nwritten;
}

#pragma GCC diagnostic pop

static inline L_FD l_open_read(const char* file) {
    return l_open(file, O_RDONLY, 0);
}

static inline L_FD l_open_write(const char* file) {
    return l_open(file, O_WRONLY | O_CREAT, 0644);
}

static inline L_FD l_open_readwrite(const char* file) {
    return l_open(file, O_RDWR | O_CREAT, 0644);
}

static inline L_FD l_open_append(const char* file) {
    return l_open(file, O_WRONLY | O_APPEND | O_CREAT, 0600);
}

static inline L_FD l_open_trunc(const char* file) {
    return l_open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
}

// l_signal — WASI has no signal support
typedef void (*L_SigHandler)(int);
#define L_SIG_DFL ((L_SigHandler)0)

static inline L_SigHandler l_signal(int sig, L_SigHandler handler)
{
    (void)sig; (void)handler;
    return L_SIG_DFL;
}

// l_writev — scatter write using multiple __wasi_fd_write iovecs
static inline ptrdiff_t l_writev(L_FD fd, const L_IoVec *iov, int iovcnt)
{
    ptrdiff_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].len == 0) continue;
        ssize_t n = l_write(fd, iov[i].base, iov[i].len);
        if (n < 0) return total > 0 ? total : -1;
        total += n;
        if ((size_t)n < iov[i].len) break;
    }
    return total;
}

// l_readv — scatter read
static inline ptrdiff_t l_readv(L_FD fd, L_IoVec *iov, int iovcnt)
{
    ptrdiff_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].len == 0) continue;
        ssize_t n = l_read(fd, iov[i].base, iov[i].len);
        if (n < 0) return total > 0 ? total : -1;
        if (n == 0) break;
        total += n;
        if ((size_t)n < iov[i].len) break;
    }
    return total;
}

// l_isatty — WASI has no terminal
static inline int l_isatty(L_FD fd)
{
    (void)fd;
    return 0;
}

#else // Windows starts here

static noreturn inline void l_exit(int status)
{
     ExitProcess(status);
    while(1);
}

#define O_RDONLY            GENERIC_READ
#define O_WRONLY            GENERIC_WRITE
#define O_RDWR              (GENERIC_READ | GENERIC_WRITE)
#define O_CREAT          0x40
#define O_EXCL           0x80
#define O_NOCTTY        0x100
#define O_TRUNC         0x200
#define O_APPEND        0x400
#define O_NONBLOCK      0x800
#define O_DIRECTORY   0x10000

#define RETURN_CHECK(toReturn) if(result) return toReturn; else return -1

static inline int l_utf8_to_wide(const char *path, wchar_t *wbuf, size_t wbuf_len);
static inline int l_wide_to_utf8(const wchar_t *wbuf, char *buf, int buf_len);

/// Maps a Win32 error code (from GetLastError) to an L_E* constant
static inline int l_win_error_to_errno(DWORD err)
{
    switch (err) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:     return L_ENOENT;
        case ERROR_ACCESS_DENIED:      return L_EACCES;
        case ERROR_INVALID_HANDLE:     return L_EBADF;
        case ERROR_FILE_EXISTS:
        case ERROR_ALREADY_EXISTS:     return L_EEXIST;
        case ERROR_INVALID_PARAMETER:  return L_EINVAL;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:        return L_ENOMEM;
        case ERROR_BROKEN_PIPE:        return L_EPIPE;
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:   return L_ENOSPC;
        case ERROR_DIRECTORY:          return L_ENOTDIR;
        case ERROR_DIR_NOT_EMPTY:      return L_ENOTEMPTY;
        default:                       return L_EINVAL;
    }
}

static inline HANDLE l_win_dup_handle(HANDLE src, BOOL inherit)
{
    HANDLE proc = GetCurrentProcess();
    HANDLE dup = INVALID_HANDLE_VALUE;
    if (!src || src == INVALID_HANDLE_VALUE) return INVALID_HANDLE_VALUE;
    if (!DuplicateHandle(proc, src, proc, &dup, 0, inherit, DUPLICATE_SAME_ACCESS))
        return INVALID_HANDLE_VALUE;
    return dup;
}

#define L_WIN_MAX_FDS 256
#define L_WIN_FD_VALID 1

typedef struct {
    HANDLE handle;
    unsigned char flags;
} L_WinFdEntry;

typedef struct {
    int initialized;
    L_WinFdEntry entries[L_WIN_MAX_FDS];
} L_WinFdTable;

static L_WinFdTable l_win_fd_table;

static inline void l_win_fd_table_init(void)
{
    HANDLE raw;
    HANDLE dup;
    int i;

    if (l_win_fd_table.initialized) return;

    l_win_fd_table.initialized = 1;
    for (i = 0; i < L_WIN_MAX_FDS; i++) {
        l_win_fd_table.entries[i].handle = INVALID_HANDLE_VALUE;
        l_win_fd_table.entries[i].flags = 0;
    }

    raw = GetStdHandle(STD_INPUT_HANDLE);
    dup = l_win_dup_handle(raw, FALSE);
    if (dup != INVALID_HANDLE_VALUE) {
        l_win_fd_table.entries[L_STDIN].handle = dup;
        l_win_fd_table.entries[L_STDIN].flags = L_WIN_FD_VALID;
    }

    raw = GetStdHandle(STD_OUTPUT_HANDLE);
    dup = l_win_dup_handle(raw, FALSE);
    if (dup != INVALID_HANDLE_VALUE) {
        l_win_fd_table.entries[L_STDOUT].handle = dup;
        l_win_fd_table.entries[L_STDOUT].flags = L_WIN_FD_VALID;
    }

    raw = GetStdHandle(STD_ERROR_HANDLE);
    dup = l_win_dup_handle(raw, FALSE);
    if (dup != INVALID_HANDLE_VALUE) {
        l_win_fd_table.entries[L_STDERR].handle = dup;
        l_win_fd_table.entries[L_STDERR].flags = L_WIN_FD_VALID;
    }
}

static inline HANDLE l_win_fd_handle(L_FD fd)
{
    l_win_fd_table_init();
    if (fd < 0 || fd >= L_WIN_MAX_FDS) return INVALID_HANDLE_VALUE;
    if (!(l_win_fd_table.entries[fd].flags & L_WIN_FD_VALID)) return INVALID_HANDLE_VALUE;
    return l_win_fd_table.entries[fd].handle;
}

static inline L_FD l_win_alloc_fd(HANDLE handle)
{
    L_FD fd;

    l_win_fd_table_init();
    if (!handle || handle == INVALID_HANDLE_VALUE) return -1;

    for (fd = 3; fd < L_WIN_MAX_FDS; fd++) {
        if (!(l_win_fd_table.entries[fd].flags & L_WIN_FD_VALID)) {
            l_win_fd_table.entries[fd].handle = handle;
            l_win_fd_table.entries[fd].flags = L_WIN_FD_VALID;
            return fd;
        }
    }
    return -1;
}

static inline void l_win_fd_clear(L_FD fd)
{
    l_win_fd_table_init();
    if (fd < 0 || fd >= L_WIN_MAX_FDS) return;
    l_win_fd_table.entries[fd].handle = INVALID_HANDLE_VALUE;
    l_win_fd_table.entries[fd].flags = 0;
}

static inline HANDLE l_win_fd_replace(L_FD fd, HANDLE handle)
{
    HANDLE prev = INVALID_HANDLE_VALUE;

    l_win_fd_table_init();
    if (fd < 0 || fd >= L_WIN_MAX_FDS) return INVALID_HANDLE_VALUE;
    if (l_win_fd_table.entries[fd].flags & L_WIN_FD_VALID)
        prev = l_win_fd_table.entries[fd].handle;
    l_win_fd_table.entries[fd].handle = handle;
    l_win_fd_table.entries[fd].flags = (handle && handle != INVALID_HANDLE_VALUE) ? L_WIN_FD_VALID : 0;
    return prev;
}

static inline int l_win_cmd_push(wchar_t *buf, size_t cch, size_t *pos, wchar_t ch)
{
    if (*pos + 1 >= cch) return 0;
    buf[(*pos)++] = ch;
    buf[*pos] = L'\0';
    return 1;
}

static inline int l_win_cmd_repeat(wchar_t *buf, size_t cch, size_t *pos, wchar_t ch, size_t count)
{
    while (count--) {
        if (!l_win_cmd_push(buf, cch, pos, ch)) return 0;
    }
    return 1;
}

static inline int l_win_cmd_needs_quotes(const wchar_t *arg)
{
    if (!*arg) return 1;
    for (; *arg; arg++) {
        if (*arg == L' ' || *arg == L'\t' || *arg == L'"') return 1;
    }
    return 0;
}

static inline int l_win_cmd_append_arg(wchar_t *buf, size_t cch, size_t *pos, const char *arg)
{
    wchar_t warg[512];
    size_t slashes = 0;

    if (!l_utf8_to_wide(arg, warg, 512)) return 0;
    if (!l_win_cmd_needs_quotes(warg)) {
        for (size_t i = 0; warg[i]; i++) {
            if (!l_win_cmd_push(buf, cch, pos, warg[i])) return 0;
        }
        return 1;
    }

    if (!l_win_cmd_push(buf, cch, pos, L'"')) return 0;
    for (size_t i = 0;; i++) {
        wchar_t ch = warg[i];
        if (ch == L'\\') {
            slashes++;
            continue;
        }
        if (ch == L'"') {
            if (!l_win_cmd_repeat(buf, cch, pos, L'\\', slashes * 2 + 1)) return 0;
            if (!l_win_cmd_push(buf, cch, pos, L'"')) return 0;
            slashes = 0;
            continue;
        }
        if (ch == L'\0') {
            if (!l_win_cmd_repeat(buf, cch, pos, L'\\', slashes * 2)) return 0;
            break;
        }
        if (!l_win_cmd_repeat(buf, cch, pos, L'\\', slashes)) return 0;
        slashes = 0;
        if (!l_win_cmd_push(buf, cch, pos, ch)) return 0;
    }
    return l_win_cmd_push(buf, cch, pos, L'"');
}

static inline int l_win_build_cmdline(char *const argv[], wchar_t *cmdline, size_t cch)
{
    size_t pos = 0;
    if (!argv || !argv[0] || cch == 0) return 0;
    cmdline[0] = L'\0';
    for (int i = 0; argv[i]; i++) {
        if (i > 0 && !l_win_cmd_push(cmdline, cch, &pos, L' ')) return 0;
        if (!l_win_cmd_append_arg(cmdline, cch, &pos, argv[i])) return 0;
    }
    return 1;
}

static inline wchar_t *l_win_build_env_block(char *const envp[], wchar_t *buf, size_t cch)
{
    size_t pos = 0;
    if (!envp) return (wchar_t *)0;
    for (int i = 0; envp[i]; i++) {
        wchar_t wentry[512];
        if (!l_utf8_to_wide(envp[i], wentry, 512)) return (wchar_t *)0;
        for (size_t j = 0;; j++) {
            if (pos + 1 >= cch) return (wchar_t *)0;
            buf[pos++] = wentry[j];
            if (wentry[j] == L'\0') break;
        }
    }
    if (pos + 1 >= cch) return (wchar_t *)0;
    buf[pos++] = L'\0';
    return buf;
}

static inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    DWORD written;
    HANDLE out = l_win_fd_handle(fd);
    BOOL result = WriteFile(out, buf, count, &written, NULL);
    if (result) { l_set_errno(0); return written; }
    l_set_errno(l_win_error_to_errno(GetLastError()));
    return -1;
}

static inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    DWORD readden;
    HANDLE in = l_win_fd_handle(fd);
    BOOL result = ReadFile(in, buf, count, &readden, NULL);
    if (result) { l_set_errno(0); return readden; }
    l_set_errno(l_win_error_to_errno(GetLastError()));
    return -1;
}


static inline int l_close(L_FD fd) {
    HANDLE handle = l_win_fd_handle(fd);
    BOOL result;
    if (handle == INVALID_HANDLE_VALUE) { l_set_errno(L_EBADF); return -1; }
    result = CloseHandle(handle);
    if (!result) { l_set_errno(l_win_error_to_errno(GetLastError())); return -1; }
    l_win_fd_clear(fd);
    l_set_errno(0);
    return 0;
}

// Terminal and timing support for Windows
static inline void l_sleep_ms(unsigned int ms)
{
    Sleep(ms);
}

static inline long long l_time(long long *t) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    unsigned long long ticks = ((unsigned long long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    long long val = (long long)((ticks - 116444736000000000ULL) / 10000000ULL);
    if (t) *t = val;
    return val;
}

static inline int l_getrandom(void *buf, size_t len) {
    // Use RtlGenRandom (SystemFunction036) — available since Windows XP, no extra libs needed
    // Declared in ntsecapi.h but we avoid that header; prototype manually
    typedef unsigned char (WINAPI *RtlGenRandomFn)(void *, unsigned long);
    static RtlGenRandomFn fn = NULL;
    if (!fn) {
        HMODULE advapi = LoadLibraryA("advapi32.dll");
        if (!advapi) return -1;
        fn = (RtlGenRandomFn)(void *)GetProcAddress(advapi, "SystemFunction036");
        if (!fn) return -1;
    }
    return fn(buf, (unsigned long)len) ? 0 : -1;
}

static DWORD l_saved_console_mode;

static inline unsigned long l_term_raw(void)
{
    HANDLE in = l_win_fd_handle(L_STDIN);
    if (in == INVALID_HANDLE_VALUE) return 0;
    if (!GetConsoleMode(in, &l_saved_console_mode)) return 0;
    DWORD raw = l_saved_console_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(in, raw);
    // Enable ANSI escape sequences on stdout
    HANDLE out = l_win_fd_handle(L_STDOUT);
    DWORD out_mode;
    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &out_mode))
        SetConsoleMode(out, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return l_saved_console_mode;
}

static inline void l_term_restore(unsigned long old_mode)
{
    HANDLE in = l_win_fd_handle(L_STDIN);
    if (in != INVALID_HANDLE_VALUE)
        SetConsoleMode(in, (DWORD)old_mode);
}

static inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
{
    HANDLE in = l_win_fd_handle(fd);
    INPUT_RECORD ir;
    DWORD events;
    DWORD mode;
    if (count == 0) return 0;
    if (in == INVALID_HANDLE_VALUE) return -1;
    if (!GetConsoleMode(in, &mode)) {
        if (GetFileType(in) == FILE_TYPE_PIPE) {
            DWORD avail = 0;
            if (!PeekNamedPipe(in, NULL, 0, NULL, &avail, NULL)) return -1;
            if (avail == 0) return 0;
        }
        return l_read(fd, buf, count);
    }
    while (PeekConsoleInputW(in, &ir, 1, &events) && events > 0) {
        ReadConsoleInputW(in, &ir, 1, &events);
        if (ir.EventType == KEY_EVENT && ir.Event.KeyEvent.bKeyDown) {
            char c = ir.Event.KeyEvent.uChar.AsciiChar;
            if (c) {
                *(char*)buf = c;
                return 1;
            }
        }
    }
    return 0;
}

static inline void l_term_size(int *rows, int *cols)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE out = l_win_fd_handle(L_STDOUT);
    if (out != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(out, &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *rows = 24; *cols = 80;
    }
}


static inline int l_utf8_to_wide(const char *path, wchar_t *wbuf, size_t wbuf_len) {
    size_t utf8Len = l_strlen(path) + 1;
    if (utf8Len * 2 > wbuf_len * sizeof(wchar_t)) return 0;
    return MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path, (int)utf8Len, wbuf, (int)wbuf_len);
}

static inline int l_wide_to_utf8(const wchar_t *wbuf, char *buf, int buf_len) {
    return WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, buf, buf_len, NULL, NULL);
}

static inline int l_chdir(const char *path)
{
    wchar_t wpath[512];
    if (!l_utf8_to_wide(path, wpath, 512)) return -1;
    return SetCurrentDirectoryW(wpath) ? 0 : -1;
}

static inline char *l_getcwd(char *buf, size_t size)
{
    wchar_t wbuf[512];
    DWORD len = GetCurrentDirectoryW(512, wbuf);
    if (len == 0 || len >= 512) return (char *)0;
    if (!l_wide_to_utf8(wbuf, buf, (int)size)) return (char *)0;
    return buf;
}

static inline int l_pipe(L_FD fds[2])
{
    L_FD read_fd;
    L_FD write_fd;
    HANDLE rd, wr;
    if (!CreatePipe(&rd, &wr, NULL, 0)) return -1;

    read_fd = l_win_alloc_fd(rd);
    if (read_fd < 0) {
        CloseHandle(rd);
        CloseHandle(wr);
        return -1;
    }

    write_fd = l_win_alloc_fd(wr);
    if (write_fd < 0) {
        l_close(read_fd);
        CloseHandle(wr);
        return -1;
    }

    fds[0] = read_fd;
    fds[1] = write_fd;
    return 0;
}

static inline int l_dup(L_FD fd)
{
    HANDLE src = l_win_fd_handle(fd);
    HANDLE dup;
    L_FD newfd;

    if (!src || src == INVALID_HANDLE_VALUE) return -1;

    dup = l_win_dup_handle(src, FALSE);
    if (dup == INVALID_HANDLE_VALUE) return -1;

    newfd = l_win_alloc_fd(dup);
    if (newfd < 0) {
        CloseHandle(dup);
        return -1;
    }
    return (int)newfd;
}

static inline int l_dup2(L_FD oldfd, L_FD newfd)
{
    HANDLE src = l_win_fd_handle(oldfd);
    HANDLE dup;

    if (!src || src == INVALID_HANDLE_VALUE) return -1;
    if (newfd < 0 || newfd >= L_WIN_MAX_FDS) return -1;
    if (oldfd == newfd) return (int)newfd;

    dup = l_win_dup_handle(src, FALSE);
    if (dup == INVALID_HANDLE_VALUE) return -1;

    {
        HANDLE prev = l_win_fd_replace(newfd, dup);
        if (prev != INVALID_HANDLE_VALUE)
            CloseHandle(prev);
    }
    return (int)newfd;
}

static inline off_t l_lseek(L_FD fd, off_t offset, int whence)
{
    HANDLE handle = l_win_fd_handle(fd);
    LARGE_INTEGER distance;
    LARGE_INTEGER newpos;
    DWORD move_method;

    if (handle == INVALID_HANDLE_VALUE) return (off_t)-1;

    switch (whence) {
    case SEEK_SET: move_method = FILE_BEGIN; break;
    case SEEK_CUR: move_method = FILE_CURRENT; break;
    case SEEK_END: move_method = FILE_END; break;
    default: return (off_t)-1;
    }

    distance.QuadPart = (LONGLONG)offset;
    if (!SetFilePointerEx(handle, distance, &newpos, move_method))
        return (off_t)-1;
    return (off_t)newpos.QuadPart;
}

static inline int l_truncate(const char *path, long long size)
{
    wchar_t wpath[1024];
    HANDLE h;
    LARGE_INTEGER li;
    int ret = -1;
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    h = CreateFileW(wpath, GENERIC_WRITE, 0, (void *)0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (void *)0);
    if (h == INVALID_HANDLE_VALUE) return -1;
    li.QuadPart = (LONGLONG)size;
    if (SetFilePointerEx(h, li, (void *)0, FILE_BEGIN) && SetEndOfFile(h))
        ret = 0;
    CloseHandle(h);
    return ret;
}

static inline int l_ftruncate(L_FD fd, long long size)
{
    HANDLE h = l_win_fd_handle(fd);
    LARGE_INTEGER li;
    if (h == INVALID_HANDLE_VALUE) return -1;
    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx(h, li, (void *)0, FILE_BEGIN)) return -1;
    return SetEndOfFile(h) ? 0 : -1;
}

static inline int l_mkdir(const char *path, mode_t mode)
{
    wchar_t wpath[1024];
    (void)mode;
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return CreateDirectoryW(wpath, NULL) ? 0 : -1;
}

static inline int l_sched_yield(void)
{
    Sleep(0);
    return 0;
}

static inline L_PID l_spawn_stdio(const char *path, char *const argv[], char *const envp[],
                           L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd)
{
    wchar_t wpath[1024];
    wchar_t cmdline[2048];
    wchar_t envbuf[4096];
    wchar_t *envblock = (wchar_t *)0;
    HANDLE child_in = INVALID_HANDLE_VALUE;
    HANDLE child_out = INVALID_HANDLE_VALUE;
    HANDLE child_err = INVALID_HANDLE_VALUE;
    HANDLE src;
    DWORD flags = 0;

    if (!path || !argv || !argv[0]) return -1;
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    if (!l_win_build_cmdline(argv, cmdline, 2048)) return -1;

    src = l_win_fd_handle(stdin_fd == L_SPAWN_INHERIT ? L_STDIN : stdin_fd);
    child_in = l_win_dup_handle(src, TRUE);
    if (child_in == INVALID_HANDLE_VALUE) goto fail;

    src = l_win_fd_handle(stdout_fd == L_SPAWN_INHERIT ? L_STDOUT : stdout_fd);
    child_out = l_win_dup_handle(src, TRUE);
    if (child_out == INVALID_HANDLE_VALUE) goto fail;

    src = l_win_fd_handle(stderr_fd == L_SPAWN_INHERIT ? L_STDERR : stderr_fd);
    child_err = l_win_dup_handle(src, TRUE);
    if (child_err == INVALID_HANDLE_VALUE) goto fail;

    if (envp) {
        envblock = l_win_build_env_block(envp, envbuf, 4096);
        if (!envblock) goto fail;
        flags |= CREATE_UNICODE_ENVIRONMENT;
    }

    STARTUPINFOW si;
    l_memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = child_in;
    si.hStdOutput = child_out;
    si.hStdError = child_err;

    PROCESS_INFORMATION pi;
    l_memset(&pi, 0, sizeof(pi));

    if (!CreateProcessW(wpath, cmdline, NULL, NULL, TRUE, flags, envblock, NULL, &si, &pi))
        goto fail;

    CloseHandle(child_in);
    CloseHandle(child_out);
    CloseHandle(child_err);
    CloseHandle(pi.hThread);
    return (L_PID)pi.hProcess;

fail:
    if (child_in != INVALID_HANDLE_VALUE) CloseHandle(child_in);
    if (child_out != INVALID_HANDLE_VALUE) CloseHandle(child_out);
    if (child_err != INVALID_HANDLE_VALUE) CloseHandle(child_err);
    return -1;
}

static inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[])
{
    return l_spawn_stdio(path, argv, envp, L_SPAWN_INHERIT, L_SPAWN_INHERIT, L_SPAWN_INHERIT);
}

static inline int l_wait(L_PID pid, int *exitcode)
{
    HANDLE proc = (HANDLE)pid;
    if (WaitForSingleObject(proc, INFINITE) != WAIT_OBJECT_0) {
        CloseHandle(proc);
        return -1;
    }
    DWORD code;
    if (!GetExitCodeProcess(proc, &code)) {
        CloseHandle(proc);
        return -1;
    }
    if (exitcode) *exitcode = (int)code;
    CloseHandle(proc);
    return 0;
}

static inline L_PID l_getpid(void)
{
    return (L_PID)GetCurrentProcessId();
}

static inline int l_unlink(const char *path) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return DeleteFileW(wpath) ? 0 : -1;
}

static inline int l_rmdir(const char *path) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return RemoveDirectoryW(wpath) ? 0 : -1;
}

static inline int l_rename(const char *oldpath, const char *newpath) {
    wchar_t wold[1024], wnew[1024];
    if (!l_utf8_to_wide(oldpath, wold, 1024)) return -1;
    if (!l_utf8_to_wide(newpath, wnew, 1024)) return -1;
    return MoveFileExW(wold, wnew, 1 /*MOVEFILE_REPLACE_EXISTING*/) ? 0 : -1;
}

static inline int l_access(const char *path, int mode) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    DWORD attr = GetFileAttributesW(wpath);
    if (attr == (DWORD)-1) return -1;
    if ((mode & L_W_OK) && (attr & 1 /*FILE_ATTRIBUTE_READONLY*/)) return -1;
    return 0;
}

static inline int l_chmod(const char *path, mode_t mode) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    DWORD attr = GetFileAttributesW(wpath);
    if (attr == (DWORD)-1) return -1;
    if (mode & 0200 /*S_IWUSR*/)
        attr &= ~(DWORD)1 /*FILE_ATTRIBUTE_READONLY*/;
    else
        attr |= (DWORD)1 /*FILE_ATTRIBUTE_READONLY*/;
    return SetFileAttributesW(wpath, attr) ? 0 : -1;
}

static inline int l_symlink(const char *target, const char *linkpath) {
    wchar_t wtarget[1024], wlink[1024];
    if (!l_utf8_to_wide(target, wtarget, 1024)) return -1;
    if (!l_utf8_to_wide(linkpath, wlink, 1024)) return -1;
    /* Check if target is a directory for the SYMBOLIC_LINK_FLAG_DIRECTORY flag */
    DWORD flags = 0x2; /* SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE */
    DWORD attr = GetFileAttributesW(wtarget);
    if (attr != (DWORD)-1 && (attr & FILE_ATTRIBUTE_DIRECTORY))
        flags |= 0x1; /* SYMBOLIC_LINK_FLAG_DIRECTORY */
    if (CreateSymbolicLinkW(wlink, wtarget, flags))
        return 0;
    l_set_errno(l_win_error_to_errno(GetLastError()));
    return -1;
}

static inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    HANDLE h = CreateFileW(wpath,
        0 /*no access needed*/,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING,
        FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
        NULL);
    if (h == INVALID_HANDLE_VALUE) {
        l_set_errno(l_win_error_to_errno(GetLastError()));
        return -1;
    }
    /* Use GetFinalPathNameByHandleW on the actual target, but we need the
       raw symlink target. Read reparse data instead. */
    char rdbuf[16384]; /* REPARSE_DATA_BUFFER is variable-length */
    DWORD returned = 0;
    /* DeviceIoControl with FSCTL_GET_REPARSE_POINT */
    if (!DeviceIoControl(h, 0x000900A8 /*FSCTL_GET_REPARSE_POINT*/,
                         NULL, 0, rdbuf, sizeof(rdbuf), &returned, NULL)) {
        DWORD err = GetLastError();
        CloseHandle(h);
        l_set_errno(l_win_error_to_errno(err));
        return -1;
    }
    CloseHandle(h);
    /* Parse REPARSE_DATA_BUFFER for IO_REPARSE_TAG_SYMLINK (0xA000000C) */
    DWORD tag;
    l_memcpy(&tag, rdbuf, 4);
    if (tag != 0xA000000CUL) {
        l_set_errno(L_EINVAL);
        return -1;
    }
    /* Offsets in REPARSE_DATA_BUFFER SymbolicLinkReparseBuffer:
       +8:  SubstituteNameOffset (USHORT)
       +10: SubstituteNameLength (USHORT)
       +12: PrintNameOffset (USHORT)
       +14: PrintNameLength (USHORT)
       +16: Flags (ULONG)
       +20: PathBuffer (WCHAR[]) */
    unsigned short pname_off, pname_len;
    l_memcpy(&pname_off, rdbuf + 12, 2);
    l_memcpy(&pname_len, rdbuf + 14, 2);
    wchar_t *pname = (wchar_t *)(rdbuf + 20 + pname_off);
    int wchars = pname_len / 2;
    /* Null-terminate for conversion */
    wchar_t saved = pname[wchars];
    pname[wchars] = L'\0';
    int written = l_wide_to_utf8(pname, buf, (int)bufsiz);
    pname[wchars] = saved;
    if (written <= 0) { l_set_errno(L_EINVAL); return -1; }
    /* wide_to_utf8 includes null terminator in count; readlink doesn't */
    return (ptrdiff_t)(written - 1);
}

static inline char *l_realpath(const char *path, char *resolved) {
    wchar_t wpath[1024], wresolved[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return (char *)0;
    HANDLE h = CreateFileW(wpath, 0 /*no access*/,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        l_set_errno(l_win_error_to_errno(GetLastError()));
        return (char *)0;
    }
    DWORD len = GetFinalPathNameByHandleW(h, wresolved, 1024, 0 /*VOLUME_NAME_DOS*/);
    CloseHandle(h);
    if (len == 0 || len >= 1024) { l_set_errno(L_EINVAL); return (char *)0; }
    /* Strip \\?\ prefix if present */
    wchar_t *src = wresolved;
    if (len >= 4 && src[0] == L'\\' && src[1] == L'\\' && src[2] == L'?' && src[3] == L'\\')
        src += 4;
    if (!l_wide_to_utf8(src, resolved, L_PATH_MAX)) return (char *)0;
    return resolved;
}

static inline int l_stat(const char *path, L_Stat *st) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(wpath, GetFileExInfoStandard, &fad)) return -1;
    st->st_size = ((long long)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
    if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        st->st_mode = L_S_IFDIR | 0755;
    else
        st->st_mode = L_S_IFREG | 0644;
    // Convert FILETIME to Unix timestamp (seconds since 1970-01-01)
    long long ft = ((long long)fad.ftLastWriteTime.dwHighDateTime << 32) | fad.ftLastWriteTime.dwLowDateTime;
    st->st_mtime = (ft - 116444736000000000LL) / 10000000LL;
    return 0;
}

static inline int l_fstat(L_FD fd, L_Stat *st) {
    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(l_win_fd_handle(fd), &info)) return -1;
    st->st_size = ((long long)info.nFileSizeHigh << 32) | info.nFileSizeLow;
    if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        st->st_mode = L_S_IFDIR | 0755;
    else
        st->st_mode = L_S_IFREG | 0644;
    long long ft = ((long long)info.ftLastWriteTime.dwHighDateTime << 32) | info.ftLastWriteTime.dwLowDateTime;
    st->st_mtime = (ft - 116444736000000000LL) / 10000000LL;
    return 0;
}

static inline int l_opendir(const char *path, L_Dir *dir) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    // Append \* for FindFirstFileW
    size_t len = l_wcslen(wpath);
    if (len > 0 && wpath[len - 1] != L'\\' && wpath[len - 1] != L'/') {
        wpath[len] = L'\\';
        len++;
    }
    wpath[len] = L'*';
    wpath[len + 1] = L'\0';

    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(wpath, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;
    dir->handle = (void *)h;
    dir->first = 1;
    dir->done = 0;
    // Convert first entry name to UTF-8
    WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, dir->current.d_name, 256, NULL, NULL);
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        dir->current.d_type = L_DT_DIR;
    else
        dir->current.d_type = L_DT_REG;
    return 0;
}

static inline L_DirEntry *l_readdir(L_Dir *dir) {
    if (dir->done) return (L_DirEntry *)0;
    if (dir->first) {
        dir->first = 0;
        return &dir->current;
    }
    WIN32_FIND_DATAW fd;
    if (!FindNextFileW((HANDLE)dir->handle, &fd)) {
        dir->done = 1;
        return (L_DirEntry *)0;
    }
    WideCharToMultiByte(CP_UTF8, 0, fd.cFileName, -1, dir->current.d_name, 256, NULL, NULL);
    if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        dir->current.d_type = L_DT_DIR;
    else
        dir->current.d_type = L_DT_REG;
    return &dir->current;
}

static inline void l_closedir(L_Dir *dir) {
    FindClose((HANDLE)dir->handle);
}

static inline void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset)
{
    (void)addr;
    // Anonymous mapping: use VirtualAlloc
    if (flags & L_MAP_ANONYMOUS) {
        DWORD flProtect = PAGE_READWRITE;
        if (prot & L_PROT_EXEC) flProtect = PAGE_EXECUTE_READWRITE;
        void *p = VirtualAlloc(NULL, length, MEM_COMMIT | MEM_RESERVE, flProtect);
        return p ? p : L_MAP_FAILED;
    }
    // File mapping
    DWORD flProtect;
    DWORD dwAccess;
    if (prot & L_PROT_WRITE) {
        flProtect = PAGE_READWRITE;
        dwAccess = FILE_MAP_WRITE;
    } else {
        flProtect = PAGE_READONLY;
        dwAccess = FILE_MAP_READ;
    }
    if (prot & L_PROT_EXEC) {
        flProtect = (prot & L_PROT_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
        dwAccess |= FILE_MAP_EXECUTE;
    }
    DWORD sizeHi = 0;
    DWORD sizeLo = 0;
    if (prot & L_PROT_WRITE) {
        sizeHi = (DWORD)((unsigned long long)(offset + length) >> 32);
        sizeLo = (DWORD)((unsigned long long)(offset + length) & 0xFFFFFFFF);
    }
    HANDLE hFile = l_win_fd_handle(fd);
    if (hFile == INVALID_HANDLE_VALUE) return L_MAP_FAILED;
    HANDLE hMap = CreateFileMappingW(hFile, NULL, flProtect, sizeHi, sizeLo, NULL);
    if (!hMap) return L_MAP_FAILED;
    DWORD offHi = (DWORD)((unsigned long long)offset >> 32);
    DWORD offLo = (DWORD)((unsigned long long)offset & 0xFFFFFFFF);
    void *p = MapViewOfFile(hMap, dwAccess, offHi, offLo, length);
    CloseHandle(hMap);
    return p ? p : L_MAP_FAILED;
}

static inline int l_munmap(void *addr, size_t length)
{
    (void)length;
    // Try UnmapViewOfFile first (for file mappings), then VirtualFree (for anonymous)
    if (UnmapViewOfFile(addr)) return 0;
    if (VirtualFree(addr, 0, MEM_RELEASE)) return 0;
    return -1;
}

static inline L_FD l_win_open_gen(const char* file, DWORD desired, DWORD shared, DWORD dispo) {

    size_t utf8Len = strlen(file) + 1; // +1 for terminating null
    // The worst case scenario is for each byte to be a separate codepoint, which takes 2 bytes in u16.
    size_t utf16Len = utf8Len * 2;
    wchar_t buffer[utf16Len];

    int result = MultiByteToWideChar(
                     CP_UTF8,                // convert from UTF-8
                     MB_ERR_INVALID_CHARS,   // error on invalid chars
                     file,                   // source UTF-8 string
                     utf8Len,                // total length of source UTF-8 string,
                     // in CHAR's (= bytes), including end-of-string \0
                     buffer,                 // destination buffer
                     utf16Len                // size of destination buffer, in WCHAR's
                 );

    if(!result) { l_set_errno(L_EINVAL); return -1; }

    HANDLE hFile = CreateFileW(buffer,        // file to open
                               desired,          // open for reading
                               shared,       // share for reading
                               NULL,                  // default security
                               dispo,         // existing file only
                               FILE_ATTRIBUTE_NORMAL, // normal file
                               NULL);                 // no attr. template

    if(hFile == INVALID_HANDLE_VALUE) { l_set_errno(l_win_error_to_errno(GetLastError())); return -1; }
    {
        L_FD fd = l_win_alloc_fd(hFile);
        if (fd < 0) {
            CloseHandle(hFile);
            l_set_errno(L_ENOMEM);
            return -1;
        }
        l_set_errno(0);
        return fd;
    }
}
static inline L_FD l_open_read(const char* file) {
    return l_win_open_gen(file, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
}

static inline L_FD l_open_write(const char* file) {
    return l_win_open_gen(file, GENERIC_WRITE, 0, CREATE_ALWAYS);
}

static inline L_FD l_open_readwrite(const char* file) {
    return l_win_open_gen(file, GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS);
}

static inline L_FD l_open_append(const char* file) {
    return l_win_open_gen(file, FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
}

static inline L_FD l_open_trunc(const char* file) {
    return l_win_open_gen(file, GENERIC_WRITE, 0, CREATE_ALWAYS);
}

static inline L_FD l_open(const char *path, int flags, mode_t mode) {
    (void)mode; // unused on Windows; Unix permissions don't apply
    DWORD desired = 0;
    DWORD shared = 0;
    DWORD dispo = 0;
    
    // Handle access flags
    if ((flags & O_RDWR) == O_RDWR) {
        desired = GENERIC_READ | GENERIC_WRITE;
    } else if (flags & O_WRONLY) {
        desired = GENERIC_WRITE;
    } else {
        desired = GENERIC_READ;
    }
    
    // Handle append flag
    if (flags & O_APPEND) {
        desired = FILE_APPEND_DATA;
        shared = FILE_SHARE_READ;
    }
    
    // Handle creation and truncation flags
    if (flags & O_CREAT) {
        if (flags & O_EXCL) {
            dispo = CREATE_NEW;
        } else if (flags & O_TRUNC) {
            dispo = CREATE_ALWAYS;
        } else {
            dispo = OPEN_ALWAYS;
        }
    } else {
        if (flags & O_TRUNC) {
            dispo = TRUNCATE_EXISTING;
        } else {
            dispo = OPEN_EXISTING;
        }
    }
    
    return l_win_open_gen(path, desired, shared, dispo);
}

#ifdef L_WITHSOCKETS

static int l_wsa_initialized = 0;

static inline int l_wsa_init(void) {
    if (l_wsa_initialized) return 0;
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return -1;
    l_wsa_initialized = 1;
    return 0;
}

static inline L_SOCKET l_socket_tcp(void)
{
    if (l_wsa_init() < 0) return -1;
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return -1;
    return (L_SOCKET)s;
}

static inline int l_socket_connect(L_SOCKET sock, const char *addr, int port)
{
    struct sockaddr_in sa;
    unsigned int ip;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = l_htons((unsigned short)port);
    if (!l_parse_ipv4(addr, &ip)) return -1;
    sa.sin_addr.s_addr = ip;
    if (connect((SOCKET)sock, (const struct sockaddr *)&sa, (int)sizeof(sa)) == SOCKET_ERROR)
        return -1;
    return 0;
}

static inline int l_socket_bind(L_SOCKET sock, int port)
{
    struct sockaddr_in sa;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = l_htons((unsigned short)port);
    sa.sin_addr.s_addr = 0;
    if (bind((SOCKET)sock, (const struct sockaddr *)&sa, (int)sizeof(sa)) == SOCKET_ERROR)
        return -1;
    return 0;
}

static inline int l_socket_listen(L_SOCKET sock, int backlog)
{
    if (listen((SOCKET)sock, backlog) == SOCKET_ERROR)
        return -1;
    return 0;
}

static inline L_SOCKET l_socket_accept(L_SOCKET sock)
{
    SOCKET s = accept((SOCKET)sock, NULL, NULL);
    if (s == INVALID_SOCKET) return -1;
    return (L_SOCKET)s;
}

static inline ptrdiff_t l_socket_send(L_SOCKET sock, const void *data, size_t len)
{
    int ret = send((SOCKET)sock, (const char *)data, (int)len, 0);
    if (ret == SOCKET_ERROR) return -1;
    return (ptrdiff_t)ret;
}

static inline ptrdiff_t l_socket_recv(L_SOCKET sock, void *buf, size_t len)
{
    int ret = recv((SOCKET)sock, (char *)buf, (int)len, 0);
    if (ret == SOCKET_ERROR) return -1;
    return (ptrdiff_t)ret;
}

static inline void l_socket_close(L_SOCKET sock)
{
    closesocket((SOCKET)sock);
}

static inline L_SOCKET l_socket_udp(void)
{
    if (l_wsa_init() < 0) return -1;
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) return -1;
    return (L_SOCKET)s;
}

static inline ptrdiff_t l_socket_sendto(L_SOCKET s, const void *data, size_t len, const char *addr, int port)
{
    struct sockaddr_in sa;
    unsigned int ip;
    l_memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = l_htons((unsigned short)port);
    if (!l_parse_ipv4(addr, &ip)) return -1;
    sa.sin_addr.s_addr = ip;
    int ret = sendto((SOCKET)s, (const char *)data, (int)len, 0, (const struct sockaddr *)&sa, (int)sizeof(sa));
    if (ret == SOCKET_ERROR) return -1;
    return (ptrdiff_t)ret;
}

static inline ptrdiff_t l_socket_recvfrom(L_SOCKET s, void *buf, size_t len, char *addr_out, int *port_out)
{
    struct sockaddr_in sa;
    int salen = (int)sizeof(sa);
    l_memset(&sa, 0, sizeof(sa));
    int ret = recvfrom((SOCKET)s, (char *)buf, (int)len, 0, (struct sockaddr *)&sa, &salen);
    if (ret == SOCKET_ERROR) return -1;
    if (addr_out) {
        unsigned int ip = sa.sin_addr.s_addr;
        int pos = 0;
        for (int i = 0; i < 4; i++) {
            unsigned int octet = (ip >> (i * 8)) & 0xFF;
            if (octet >= 100) addr_out[pos++] = (char)('0' + octet / 100);
            if (octet >= 10) addr_out[pos++] = (char)('0' + (octet / 10) % 10);
            addr_out[pos++] = (char)('0' + octet % 10);
            if (i < 3) addr_out[pos++] = '.';
        }
        addr_out[pos] = '\0';
    }
    if (port_out) *port_out = (int)l_htons(sa.sin_port);
    return (ptrdiff_t)ret;
}

// --- Generic address-based socket API (Windows) ---

static inline int l_sockaddr_to_wsa(const L_SockAddr *sa, void *out, int *outlen) {
    if (sa->family == L_AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)out;
        l_memset(s, 0, sizeof(*s));
        s->sin_family = AF_INET;
        s->sin_port = l_htons(sa->port);
        l_memcpy(&s->sin_addr.s_addr, sa->addr, 4);
        *outlen = (int)sizeof(struct sockaddr_in);
        return 0;
    }
    if (sa->family == L_AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)out;
        l_memset(s, 0, sizeof(*s));
        s->sin6_family = AF_INET6;
        s->sin6_port = l_htons(sa->port);
        l_memcpy(&s->sin6_addr, sa->addr, 16);
        *outlen = (int)sizeof(struct sockaddr_in6);
        return 0;
    }
    return -1;
}

static inline L_SOCKET l_socket_open(int family, int type) {
    if (l_wsa_init() < 0) return -1;
    int af = (family == L_AF_INET6) ? AF_INET6 : AF_INET;
    int st = (type == L_SOCK_DGRAM) ? SOCK_DGRAM : SOCK_STREAM;
    int proto = (type == L_SOCK_DGRAM) ? IPPROTO_UDP : IPPROTO_TCP;
    SOCKET s = socket(af, st, proto);
    if (s == INVALID_SOCKET) return -1;
    return (L_SOCKET)s;
}

static inline int l_socket_connect_addr(L_SOCKET sock, const L_SockAddr *addr) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_wsa(addr, osbuf, &salen) < 0) return -1;
    return connect((SOCKET)sock, (const struct sockaddr *)osbuf, salen) == 0 ? 0 : -1;
}

static inline int l_socket_bind_addr(L_SOCKET sock, const L_SockAddr *addr) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_wsa(addr, osbuf, &salen) < 0) return -1;
    return bind((SOCKET)sock, (const struct sockaddr *)osbuf, salen) == 0 ? 0 : -1;
}

static inline ptrdiff_t l_socket_sendto_addr(L_SOCKET s, const void *data, size_t len, const L_SockAddr *dest) {
    char osbuf[28];
    int salen;
    if (l_sockaddr_to_wsa(dest, osbuf, &salen) < 0) return -1;
    int ret = sendto((SOCKET)s, (const char *)data, (int)len, 0, (const struct sockaddr *)osbuf, salen);
    return ret == SOCKET_ERROR ? -1 : (ptrdiff_t)ret;
}

static inline ptrdiff_t l_socket_recvfrom_addr(L_SOCKET s, void *buf, size_t len, L_SockAddr *src) {
    char sabuf[28];
    int salen = (int)sizeof(sabuf);
    l_memset(sabuf, 0, sizeof(sabuf));
    int ret = recvfrom((SOCKET)s, (char *)buf, (int)len, 0, (struct sockaddr *)sabuf, &salen);
    if (ret == SOCKET_ERROR) return -1;
    if (src) {
        unsigned short fam;
        l_memcpy(&fam, sabuf, 2);
        if (fam == AF_INET6) {
            struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)sabuf;
            l_memset(src, 0, sizeof(*src));
            src->family = L_AF_INET6;
            src->port = l_htons(s6->sin6_port);
            l_memcpy(src->addr, &s6->sin6_addr, 16);
        } else {
            struct sockaddr_in *s4 = (struct sockaddr_in *)sabuf;
            l_memset(src, 0, sizeof(*src));
            src->family = L_AF_INET;
            src->port = l_htons(s4->sin_port);
            l_memcpy(src->addr, &s4->sin_addr.s_addr, 4);
        }
    }
    return (ptrdiff_t)ret;
}

#endif // L_WITHSOCKETS
static inline int l_poll(L_PollFd *fds, int nfds, int timeout_ms)
{
    if (nfds <= 0) return 0;
    int count = nfds < 64 ? nfds : 64;
    for (int i = 0; i < count; i++) fds[i].revents = 0;

#ifdef L_WITHSOCKETS
    HANDLE handles[64];
    int all_handles = 1;
    for (int i = 0; i < count; i++) {
        handles[i] = l_win_fd_handle(fds[i].fd);
        if (handles[i] == INVALID_HANDLE_VALUE)
            all_handles = 0;
    }

    if (all_handles) {
        DWORD start = GetTickCount();
        for (;;) {
            int ready = 0;
            for (int i = 0; i < count; i++) {
                DWORD type = GetFileType(handles[i]);
                if ((fds[i].events & L_POLLIN) && type == FILE_TYPE_PIPE) {
                    DWORD avail = 0;
                    if (PeekNamedPipe(handles[i], NULL, 0, NULL, &avail, NULL)) {
                        if (avail > 0) {
                            fds[i].revents |= L_POLLIN;
                            ready++;
                        }
                    } else {
                        DWORD err = GetLastError();
                        if (err == ERROR_BROKEN_PIPE || err == ERROR_PIPE_NOT_CONNECTED)
                            fds[i].revents |= L_POLLHUP;
                        else
                            fds[i].revents |= L_POLLERR;
                        ready++;
                    }
                } else {
                    DWORD result = WaitForSingleObject(handles[i], 0);
                    if (result == WAIT_OBJECT_0) {
                        fds[i].revents |= fds[i].events;
                        ready++;
                    } else if (result == WAIT_FAILED) {
                        fds[i].revents |= L_POLLERR;
                        ready++;
                    }
                }
            }
            if (ready > 0) return ready;
            if (timeout_ms == 0) return 0;
            if (timeout_ms > 0 && (DWORD)timeout_ms <= GetTickCount() - start) return 0;
            Sleep(1);
        }
    }

    /* Use select() — works for sockets on Windows.
       L_SOCKET values are raw Winsock SOCKET handles (not fd-table indices). */
    fd_set rfds, wfds, efds;
    FD_ZERO(&rfds); FD_ZERO(&wfds); FD_ZERO(&efds);
    for (int i = 0; i < count; i++) {
        SOCKET s = (SOCKET)(uintptr_t)fds[i].fd;
        if (fds[i].events & L_POLLIN)  FD_SET(s, &rfds);
        if (fds[i].events & L_POLLOUT) FD_SET(s, &wfds);
        FD_SET(s, &efds);
    }
    struct timeval tv, *tvp = (struct timeval *)0;
    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tvp = &tv;
    }
    int ret = select(0, &rfds, &wfds, &efds, tvp);
    if (ret <= 0) return ret;
    int ready = 0;
    for (int i = 0; i < count; i++) {
        SOCKET s = (SOCKET)(uintptr_t)fds[i].fd;
        if ((fds[i].events & L_POLLIN)  && FD_ISSET(s, &rfds))  fds[i].revents |= L_POLLIN;
        if ((fds[i].events & L_POLLOUT) && FD_ISSET(s, &wfds))  fds[i].revents |= L_POLLOUT;
        if (FD_ISSET(s, &efds)) fds[i].revents |= L_POLLERR;
        if (fds[i].revents) ready++;
    }
    return ready;
#else
    /* Non-socket fallback: WaitForMultipleObjects for pipes/handles */
    HANDLE handles[64];
    for (int i = 0; i < count; i++)
        handles[i] = l_win_fd_handle(fds[i].fd);
    DWORD timeout = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;
    DWORD wret = WaitForMultipleObjects((DWORD)count, handles, FALSE, timeout);
    if (wret == WAIT_TIMEOUT) return 0;
    if (wret == WAIT_FAILED) return -1;
    int ready = 0;
    for (int i = 0; i < count; i++) {
        DWORD result = WaitForSingleObject(handles[i], 0);
        if (result == WAIT_OBJECT_0) {
            fds[i].revents = fds[i].events;
            ready++;
        }
    }
    return ready;
#endif
}

// l_signal — Windows: SetConsoleCtrlHandler for SIGINT/SIGTERM
#ifndef L_SIGINT
#define L_SIGINT  2
#define L_SIGTERM 15
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wstatic-in-inline"
#endif

static L_SigHandler l_win_sig_int_handler;
static L_SigHandler l_win_sig_term_handler;

static BOOL WINAPI l_win_ctrl_handler(DWORD ctrl)
{
    if (ctrl == CTRL_C_EVENT && l_win_sig_int_handler && l_win_sig_int_handler != L_SIG_DFL && l_win_sig_int_handler != L_SIG_IGN) {
        l_win_sig_int_handler(L_SIGINT);
        return TRUE;
    }
    if (ctrl == CTRL_BREAK_EVENT && l_win_sig_term_handler && l_win_sig_term_handler != L_SIG_DFL && l_win_sig_term_handler != L_SIG_IGN) {
        l_win_sig_term_handler(L_SIGTERM);
        return TRUE;
    }
    return FALSE;
}

static inline L_SigHandler l_signal(int sig, L_SigHandler handler)
{
    static int ctrl_handler_installed = 0;
    L_SigHandler prev = L_SIG_DFL;
    if (!ctrl_handler_installed) {
        SetConsoleCtrlHandler(l_win_ctrl_handler, TRUE);
        ctrl_handler_installed = 1;
    }
    if (sig == L_SIGINT) {
        prev = l_win_sig_int_handler ? l_win_sig_int_handler : L_SIG_DFL;
        l_win_sig_int_handler = handler;
    } else if (sig == L_SIGTERM) {
        prev = l_win_sig_term_handler ? l_win_sig_term_handler : L_SIG_DFL;
        l_win_sig_term_handler = handler;
    }
    return prev;
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif

// l_writev — Windows: loop over iovecs
static inline ptrdiff_t l_writev(L_FD fd, const L_IoVec *iov, int iovcnt)
{
    ptrdiff_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].len == 0) continue;
        ssize_t n = l_write(fd, iov[i].base, iov[i].len);
        if (n < 0) return total > 0 ? total : -1;
        total += n;
        if ((size_t)n < iov[i].len) break;
    }
    return total;
}

// l_readv — Windows: loop over iovecs
static inline ptrdiff_t l_readv(L_FD fd, L_IoVec *iov, int iovcnt)
{
    ptrdiff_t total = 0;
    for (int i = 0; i < iovcnt; i++) {
        if (iov[i].len == 0) continue;
        ssize_t n = l_read(fd, iov[i].base, iov[i].len);
        if (n < 0) return total > 0 ? total : -1;
        if (n == 0) break;
        total += n;
        if ((size_t)n < iov[i].len) break;
    }
    return total;
}

// l_isatty — Windows: GetConsoleMode
static inline int l_isatty(L_FD fd)
{
    HANDLE h = l_win_fd_handle(fd);
    DWORD mode;
    return (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)) ? 1 : 0;
}

#endif

#ifdef __cplusplus
}
#endif

static inline void l_exitif(bool condition, int code, char *message) {
    if(condition) {
        if(message) l_write(L_STDERR, message, l_strlen(message));
        l_exit(code);
    }
}

static inline void puts(const char* s) {
  l_write(L_STDOUT, s, strlen(s));
}

// Unbuffered line reader — reads one byte at a time until newline or buffer full
static inline ptrdiff_t l_read_line(L_FD fd, char *buf, size_t bufsz) {
    if (bufsz == 0) return -1;
    size_t pos = 0;
    while (pos < bufsz - 1) {
        char c;
        ptrdiff_t n = (ptrdiff_t)l_read(fd, &c, 1);
        if (n <= 0) {
            if (pos > 0) break;
            return -1;
        }
        if (c == '\n') break;
        if (c == '\r') continue;
        buf[pos++] = c;
    }
    buf[pos] = '\0';
    return (ptrdiff_t)pos;
}

// Buffered line reader — reads 4096 bytes at a time instead of one byte per syscall
static inline void l_linebuf_init(L_LineBuf *lb, L_FD fd)
{
    lb->fd  = fd;
    lb->pos = 0;
    lb->end = 0;
    lb->eof = 0;
}

static inline ptrdiff_t l_linebuf_read(L_LineBuf *lb, char *out, size_t outsz)
{
    if (outsz == 0) return -1;
    size_t written = 0;

    while (written < outsz - 1) {
        /* Refill the internal buffer when it is exhausted. */
        if (lb->pos >= lb->end) {
            if (lb->eof) break;
            ptrdiff_t n = (ptrdiff_t)l_read(lb->fd, lb->buf, L_LINEBUF_CAP);
            if (n <= 0) { lb->eof = 1; break; }
            lb->pos = 0;
            lb->end = (int)n;
        }

        char c = lb->buf[lb->pos++];
        if (c == '\r') continue;   /* skip CR for Windows CRLF */
        if (c == '\n') {
            out[written] = '\0';
            return (ptrdiff_t)written;
        }
        out[written++] = c;
    }

    out[written] = '\0';
    if (written == 0) return -1;   /* EOF or error with no data */
    return (ptrdiff_t)written;
}

#ifdef L_WITHSNPRINTF
/// Writes formatted output to file descriptor fd. Returns number of bytes written.
static inline int l_dprintf(L_FD fd, const char *fmt, ...) {
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = l_vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0) {
        int written = (int)l_write(fd, buf, (size_t)(n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1));
        return written;
    }
    return 0;
}

/// Writes formatted output to file descriptor fd via va_list. Returns number of bytes written.
static inline int l_vfprintf(L_FD fd, const char *fmt, va_list ap) {
    char buf[1024];
    int n = l_vsnprintf(buf, sizeof(buf), fmt, ap);
    if (n > 0) {
        int written = (int)l_write(fd, buf, (size_t)(n < (int)sizeof(buf) ? n : (int)sizeof(buf) - 1));
        return written;
    }
    return 0;
}

/// Writes formatted output to stdout via va_list. Returns number of bytes written.
static inline int l_vprintf(const char *fmt, va_list ap) {
    return l_vfprintf(L_STDOUT, fmt, ap);
}

/// Writes formatted output to stdout. Returns number of bytes written.
static inline int l_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = l_vfprintf(L_STDOUT, fmt, ap);
    va_end(ap);
    return written;
}

/// Writes formatted output to file descriptor fd. Returns number of bytes written.
static inline int l_fprintf(L_FD fd, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int written = l_vfprintf(fd, fmt, ap);
    va_end(ap);
    return written;
}
#endif // L_WITHSNPRINTF
// On Unix: walks envp derived from argv (call l_getenv_init from main first).
// On WASI: envp is set up in _start, l_getenv_init is a no-op.
// On Windows: uses GetEnvironmentVariableW API directly.

#ifdef _WIN32

static inline void l_getenv_init(int argc, char *argv[]) {
    (void)argc; (void)argv;
}

static inline char *l_getenv(const char *name) {
    if (!name || !*name) return (char*)0;
    size_t name_len = l_strlen(name) + 1;
    wchar_t wname[256];
    MultiByteToWideChar(CP_UTF8, 0, name, name_len, wname, 256);
    static char result[4096];
    wchar_t wresult[4096];
    DWORD len = GetEnvironmentVariableW(wname, wresult, 4096);
    if (len == 0) return (char*)0;
    WideCharToMultiByte(CP_UTF8, 0, wresult, -1, result, 4096, NULL, NULL);
    return result;
}

#elif defined(__wasi__)

static inline void l_getenv_init(int argc, char *argv[]) {
    (void)argc; (void)argv; // envp already populated in _start
}

static inline char *l_getenv(const char *name) {
    if (!l_envp || !name || !*name) return (char*)0;
    size_t len = l_strlen(name);
    for (char **ep = l_envp; *ep; ep++) {
        if (l_strncmp(*ep, name, len) == 0 && (*ep)[len] == '=')
            return *ep + len + 1;
    }
    return (char*)0;
}

#else

static inline void l_getenv_init(int argc, char *argv[]) {
    l_envp = argv + argc + 1;
}

static inline char *l_getenv(const char *name) {
    if (!l_envp || !name || !*name) return (char*)0;
    size_t len = l_strlen(name);
    for (char **ep = l_envp; *ep; ep++) {
        if (l_strncmp(*ep, name, len) == 0 && (*ep)[len] == '=')
            return *ep + len + 1;
    }
    return (char*)0;
}

#endif

// l_env_start / l_env_next / l_env_end: iterate all environment variables.
// Usage:
//   void *h = l_env_start();
//   void *it = h;
//   char buf[4096];
//   const char *e;
//   while ((e = l_env_next(&it, buf, sizeof(buf))) != NULL) { ... }
//   l_env_end(h);

#ifdef _WIN32

static inline void *l_env_start(void) {
    return (void *)GetEnvironmentStringsW();
}

static inline const char *l_env_next(void **iter, char *buf, size_t bufsz) {
    wchar_t *p = (wchar_t *)*iter;
    if (!p || !*p) return (const char *)0;
    l_wide_to_utf8(p, buf, (int)bufsz);
    while (*p) p++;
    p++; // skip past null terminator to next entry
    *iter = (void *)p;
    return buf;
}

static inline void l_env_end(void *handle) {
    if (handle) FreeEnvironmentStringsW((wchar_t *)handle);
}

#else

static inline void *l_env_start(void) {
    return (void *)l_envp;
}

static inline const char *l_env_next(void **iter, char *buf, size_t bufsz) {
    (void)buf; (void)bufsz;
    char **ep = (char **)*iter;
    if (!ep || !*ep) return (const char *)0;
    const char *entry = *ep;
    *iter = (void *)(ep + 1);
    return entry;
}

static inline void l_env_end(void *handle) {
    (void)handle;
}

#endif

// l_setenv / l_unsetenv
#ifdef _WIN32

static inline int l_setenv(const char *name, const char *value) {
    if (!name || !*name) return -1;
    wchar_t wname[256], wvalue[4096];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 256);
    MultiByteToWideChar(CP_UTF8, 0, value, -1, wvalue, 4096);
    return SetEnvironmentVariableW(wname, wvalue) ? 0 : -1;
}

static inline int l_unsetenv(const char *name) {
    if (!name || !*name) return -1;
    wchar_t wname[256];
    MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 256);
    return SetEnvironmentVariableW(wname, (void *)0) ? 0 : -1;
}

#else

// Linux: manipulate the environ array directly.
// Uses a static pool for new/modified entries.
// 512 slots handles CI runners with 100-200+ env vars plus headroom for new entries.
#define L_ENV_POOL_SIZE 512
#define L_ENV_BUF_SIZE 8192

static char l_env_buf[L_ENV_BUF_SIZE];
static size_t l_env_buf_used = 0;
static char *l_env_pool[L_ENV_POOL_SIZE];
static int l_env_pool_used = 0;
static int l_env_pool_init = 0;

static inline int l_setenv(const char *name, const char *value) {
    if (!name || !*name || !l_envp) return -1;
    size_t nlen = l_strlen(name);
    size_t vlen = l_strlen(value);
    size_t entry_len = nlen + 1 + vlen + 1; // "name=value\0"

    // First time: copy envp pointers into our pool
    if (!l_env_pool_init) {
        l_env_pool_init = 1;
        int i = 0;
        for (char **ep = l_envp; *ep && i < L_ENV_POOL_SIZE - 1; ep++, i++)
            l_env_pool[i] = *ep;
        l_env_pool_used = i;
        l_env_pool[i] = (char *)0;
        l_envp = l_env_pool;
    }

    // Allocate the new entry in our static buffer
    if (l_env_buf_used + entry_len > L_ENV_BUF_SIZE) return -1;
    char *entry = l_env_buf + l_env_buf_used;
    l_memcpy(entry, name, nlen);
    entry[nlen] = '=';
    l_memcpy(entry + nlen + 1, value, vlen);
    entry[nlen + 1 + vlen] = '\0';
    l_env_buf_used += entry_len;

    // Search for existing key and replace
    for (int i = 0; i < l_env_pool_used; i++) {
        if (l_strncmp(l_env_pool[i], name, nlen) == 0 && l_env_pool[i][nlen] == '=') {
            l_env_pool[i] = entry;
            return 0;
        }
    }

    // Append new entry
    if (l_env_pool_used >= L_ENV_POOL_SIZE - 1) return -1;
    l_env_pool[l_env_pool_used++] = entry;
    l_env_pool[l_env_pool_used] = (char *)0;
    return 0;
}

static inline int l_unsetenv(const char *name) {
    if (!name || !*name || !l_envp) return -1;
    size_t nlen = l_strlen(name);

    if (!l_env_pool_init) {
        l_env_pool_init = 1;
        int i = 0;
        for (char **ep = l_envp; *ep && i < L_ENV_POOL_SIZE - 1; ep++, i++)
            l_env_pool[i] = *ep;
        l_env_pool_used = i;
        l_env_pool[i] = (char *)0;
        l_envp = l_env_pool;
    }

    for (int i = 0; i < l_env_pool_used; i++) {
        if (l_strncmp(l_env_pool[i], name, nlen) == 0 && l_env_pool[i][nlen] == '=') {
            // Shift remaining entries down
            for (int j = i; j < l_env_pool_used - 1; j++)
                l_env_pool[j] = l_env_pool[j + 1];
            l_env_pool_used--;
            l_env_pool[l_env_pool_used] = (char *)0;
            return 0;
        }
    }
    return -1;
}

#endif

// l_find_executable: resolve a command name to a full executable path.
// Searches PATH with platform-appropriate separator and extension logic.

static inline int l_find_executable(const char *cmd, char *out, size_t outsz) {
    if (!cmd || !*cmd || !out || outsz < 2) return 0;

#ifdef _WIN32
    const char path_sep = ';';
    const char dir_sep = '\\';
    static const char *win_exts[] = { ".exe", ".bat", ".com", (const char *)0 };
#else
    const char path_sep = ':';
    const char dir_sep = '/';
#endif

    int isz = (int)outsz;

#ifdef _WIN32
    /* Check if cmd already has a file extension (dot after last separator) */
    int cmd_has_ext = 0;
    {
        const char *dot = (const char *)0;
        const char *sep = (const char *)0;
        for (const char *s = cmd; *s; s++) {
            if (*s == '.') dot = s;
            if (*s == '/' || *s == '\\') sep = s;
        }
        if (dot && (!sep || dot > sep)) cmd_has_ext = 1;
    }
#endif

    /* Does the command already have a path separator? Use directly. */
    {
        int has_sep = 0;
        for (const char *s = cmd; *s; s++)
            if (*s == '/' || *s == '\\') { has_sep = 1; break; }

        if (has_sep) {
            l_strncpy(out, cmd, outsz - 1);
            out[outsz - 1] = '\0';
#ifdef _WIN32
            /* On Windows, try extensions first when cmd has no extension */
            if (!cmd_has_ext) {
                int base_len = (int)l_strlen(out);
                for (const char **ext = win_exts; *ext; ext++) {
                    if (base_len + 5 < isz) {
                        l_strncpy(out + base_len, *ext, outsz - (size_t)base_len);
                        if (l_access(out, L_F_OK) == 0) return 1;
                    }
                }
                out[base_len] = '\0'; /* restore bare name for fallback */
            }
#endif
            if (l_access(out, L_F_OK) == 0) return 1;
            return 0;
        }
    }

    /* Search PATH directories */
    {
        char *path = l_getenv("PATH");
        if (!path) return 0;

        const char *p = path;
        while (*p) {
            const char *e = l_strchr(p, path_sep);
            int dlen = e ? (int)(e - p) : (int)l_strlen(p);
            if (dlen > 0 && dlen + 2 < isz) {
                l_strncpy(out, p, (size_t)dlen);
                out[dlen] = dir_sep;
                out[dlen + 1] = '\0';
                l_strncat(out, cmd, outsz - (size_t)dlen - 2);

#ifdef _WIN32
                /* On Windows, try extensions first when cmd has no extension */
                if (!cmd_has_ext) {
                    int base_len = (int)l_strlen(out);
                    for (const char **ext = win_exts; *ext; ext++) {
                        if (base_len + 5 < isz) {
                            l_strncpy(out + base_len, *ext, outsz - (size_t)base_len);
                            if (l_access(out, L_F_OK) == 0) return 1;
                        }
                    }
                    out[base_len] = '\0'; /* restore bare name for fallback */
                }
#endif
                if (l_access(out, L_F_OK) == 0) return 1;
            }
            if (!e) break;
            p = e + 1;
        }
    }
    return 0;
}

// --- L_Arena: bump allocator ---

static inline L_Arena l_arena_init(size_t size) {
    L_Arena a;
    void *p = l_mmap((void *)0, size, L_PROT_READ | L_PROT_WRITE,
                     L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
    if (p == L_MAP_FAILED) {
        a.base = (unsigned char *)0;
        a.used = 0;
        a.cap = 0;
    } else {
        a.base = (unsigned char *)p;
        a.used = 0;
        a.cap = size;
    }
    return a;
}

static inline void *l_arena_alloc(L_Arena *a, size_t n) {
    size_t align = sizeof(void *);
    size_t aligned = (a->used + align - 1) & ~(align - 1);
    if (aligned + n > a->cap) return (void *)0;
    void *ptr = a->base + aligned;
    a->used = aligned + n;
    return ptr;
}

static inline void l_arena_reset(L_Arena *a) {
    a->used = 0;
}

static inline void l_arena_free(L_Arena *a) {
    if (a->base) l_munmap(a->base, a->cap);
    a->base = (unsigned char *)0;
    a->used = 0;
    a->cap = 0;
}

// --- L_Buf: growable byte buffer ---

static inline void l_buf_init(L_Buf *b) {
    b->data = (unsigned char *)0;
    b->len = 0;
    b->cap = 0;
}

static inline int l_buf_push(L_Buf *b, const void *src, size_t n) {
    if (b->len + n > b->cap) {
        size_t newcap = b->cap ? b->cap : 4096;
        while (newcap < b->len + n) newcap *= 2;
        void *p = l_mmap((void *)0, newcap, L_PROT_READ | L_PROT_WRITE,
                         L_MAP_PRIVATE | L_MAP_ANONYMOUS, (L_FD)-1, 0);
        if (p == L_MAP_FAILED) return -1;
        if (b->data) {
            l_memcpy(p, b->data, b->len);
            l_munmap(b->data, b->cap);
        }
        b->data = (unsigned char *)p;
        b->cap = newcap;
    }
    l_memcpy(b->data + b->len, src, n);
    b->len += n;
    return 0;
}

#ifdef L_WITHSNPRINTF
static inline int l_buf_printf(L_Buf *b, const char *fmt, ...) {
    char tmp[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = l_vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (n < 0) return -1;
    size_t wrote = (size_t)n < sizeof(tmp) ? (size_t)n : sizeof(tmp) - 1;
    if (l_buf_push(b, tmp, wrote) < 0) return -1;
    return (int)wrote;
}
#endif

static inline void l_buf_clear(L_Buf *b) {
    b->len = 0;
}

static inline void l_buf_free(L_Buf *b) {
    if (b->data) l_munmap(b->data, b->cap);
    b->data = (unsigned char *)0;
    b->len = 0;
    b->cap = 0;
}

// ---------------------------------------------------------------------------
// L_Str — fat string definitions
// ---------------------------------------------------------------------------

// Constructors
static inline L_Str l_str(const char *cstr) {
    L_Str s;
    s.data = cstr;
    s.len = cstr ? l_strlen(cstr) : 0;
    return s;
}
static inline L_Str l_str_from(const char *data, size_t len) {
    L_Str s;
    s.data = data;
    s.len = data ? len : 0;
    return s;
}
static inline L_Str l_str_null(void) {
    L_Str s;
    s.data = (const char *)0;
    s.len = 0;
    return s;
}

// Comparison
static inline int l_str_eq(L_Str a, L_Str b) {
    if (a.len != b.len) return 0;
    if (a.len == 0) return 1;
    return l_memcmp(a.data, b.data, a.len) == 0;
}
static inline int l_str_cmp(L_Str a, L_Str b) {
    size_t n = a.len < b.len ? a.len : b.len;
    int r = n ? l_memcmp(a.data, b.data, n) : 0;
    if (r != 0) return r;
    if (a.len < b.len) return -1;
    if (a.len > b.len) return 1;
    return 0;
}
static inline int l_str_startswith(L_Str s, L_Str prefix) {
    if (prefix.len > s.len) return 0;
    return l_memcmp(s.data, prefix.data, prefix.len) == 0;
}
static inline int l_str_endswith(L_Str s, L_Str suffix) {
    if (suffix.len > s.len) return 0;
    return l_memcmp(s.data + s.len - suffix.len, suffix.data, suffix.len) == 0;
}
static inline int l_str_contains(L_Str s, L_Str needle) {
    if (needle.len == 0) return 1;
    if (needle.len > s.len) return 0;
    return l_memmem(s.data, s.len, needle.data, needle.len) != (void *)0;
}

// Slicing (zero-copy)
static inline L_Str l_str_sub(L_Str s, size_t start, size_t len) {
    if (start >= s.len) return l_str_null();
    if (start + len > s.len) len = s.len - start;
    return l_str_from(s.data + start, len);
}
static inline L_Str l_str_ltrim(L_Str s) {
    while (s.len > 0 && l_isspace((unsigned char)*s.data)) { s.data++; s.len--; }
    return s;
}
static inline L_Str l_str_rtrim(L_Str s) {
    while (s.len > 0 && l_isspace((unsigned char)s.data[s.len - 1])) s.len--;
    return s;
}
static inline L_Str l_str_trim(L_Str s) {
    return l_str_rtrim(l_str_ltrim(s));
}
static inline ptrdiff_t l_str_chr(L_Str s, char c) {
    for (size_t i = 0; i < s.len; i++) if (s.data[i] == c) return (ptrdiff_t)i;
    return -1;
}
static inline ptrdiff_t l_str_rchr(L_Str s, char c) {
    for (size_t i = s.len; i > 0; i--) if (s.data[i - 1] == c) return (ptrdiff_t)(i - 1);
    return -1;
}
static inline ptrdiff_t l_str_find(L_Str haystack, L_Str needle) {
    if (needle.len == 0) return 0;
    if (needle.len > haystack.len) return -1;
    void *p = l_memmem(haystack.data, haystack.len, needle.data, needle.len);
    if (!p) return -1;
    return (ptrdiff_t)((const char *)p - haystack.data);
}

// Arena operations
static inline L_Str l_str_dup(L_Arena *a, L_Str s) {
    if (s.len == 0) return l_str_null();
    char *p = (char *)l_arena_alloc(a, s.len);
    if (!p) return l_str_null();
    l_memcpy(p, s.data, s.len);
    return l_str_from(p, s.len);
}
static inline L_Str l_str_cat(L_Arena *a, L_Str x, L_Str y) {
    size_t total = x.len + y.len;
    if (total == 0) return l_str_null();
    char *p = (char *)l_arena_alloc(a, total);
    if (!p) return l_str_null();
    if (x.len) l_memcpy(p, x.data, x.len);
    if (y.len) l_memcpy(p + x.len, y.data, y.len);
    return l_str_from(p, total);
}
static inline char *l_str_cstr(L_Arena *a, L_Str s) {
    char *p = (char *)l_arena_alloc(a, s.len + 1);
    if (!p) return (char *)0;
    if (s.len) l_memcpy(p, s.data, s.len);
    p[s.len] = '\0';
    return p;
}
static inline L_Str l_str_from_cstr(L_Arena *a, const char *cstr) {
    return l_str_dup(a, l_str(cstr));
}

// Split/Join (arena-backed)
static inline int l_str_split(L_Arena *a, L_Str s, L_Str delim, L_Str **out) {
    int count = 0;
    size_t pos = 0;
    if (delim.len == 0) {
        *out = (L_Str *)l_arena_alloc(a, sizeof(L_Str));
        if (!*out) return 0;
        (*out)[0] = s;
        return s.len > 0 ? 1 : 0;
    }
    // First pass: count parts
    while (pos <= s.len) {
        void *found = (pos + delim.len <= s.len)
            ? l_memmem(s.data + pos, s.len - pos, delim.data, delim.len)
            : (void *)0;
        count++;
        if (!found) break;
        pos = (size_t)((const char *)found - s.data) + delim.len;
    }
    *out = (L_Str *)l_arena_alloc(a, (size_t)count * sizeof(L_Str));
    if (!*out) return 0;
    // Second pass: fill parts (zero-copy into original)
    int idx = 0;
    pos = 0;
    while (pos <= s.len && idx < count) {
        void *found = (pos + delim.len <= s.len)
            ? l_memmem(s.data + pos, s.len - pos, delim.data, delim.len)
            : (void *)0;
        if (found) {
            size_t fpos = (size_t)((const char *)found - s.data);
            (*out)[idx++] = l_str_from(s.data + pos, fpos - pos);
            pos = fpos + delim.len;
        } else {
            (*out)[idx++] = l_str_from(s.data + pos, s.len - pos);
            break;
        }
    }
    return count;
}

static inline L_Str l_str_join(L_Arena *a, const L_Str *parts, int count, L_Str sep) {
    if (count <= 0) return l_str_null();
    size_t total = 0;
    for (int i = 0; i < count; i++) {
        total += parts[i].len;
        if (i > 0) total += sep.len;
    }
    if (total == 0) return l_str_null();
    char *p = (char *)l_arena_alloc(a, total);
    if (!p) return l_str_null();
    size_t off = 0;
    for (int i = 0; i < count; i++) {
        if (i > 0 && sep.len) { l_memcpy(p + off, sep.data, sep.len); off += sep.len; }
        if (parts[i].len) { l_memcpy(p + off, parts[i].data, parts[i].len); off += parts[i].len; }
    }
    return l_str_from(p, total);
}

// Case conversion (ASCII, arena-backed)
static inline L_Str l_str_upper(L_Arena *a, L_Str s) {
    if (s.len == 0) return l_str_null();
    char *p = (char *)l_arena_alloc(a, s.len);
    if (!p) return l_str_null();
    for (size_t i = 0; i < s.len; i++) p[i] = (char)l_toupper((unsigned char)s.data[i]);
    return l_str_from(p, s.len);
}
static inline L_Str l_str_lower(L_Arena *a, L_Str s) {
    if (s.len == 0) return l_str_null();
    char *p = (char *)l_arena_alloc(a, s.len);
    if (!p) return l_str_null();
    for (size_t i = 0; i < s.len; i++) p[i] = (char)l_tolower((unsigned char)s.data[i]);
    return l_str_from(p, s.len);
}

static inline L_Str l_str_replace(L_Arena *a, L_Str s, L_Str find, L_Str repl) {
    if (find.len == 0) return l_str_dup(a, s);
    // First pass: count occurrences to compute result size
    size_t count = 0;
    size_t pos = 0;
    while (pos <= s.len - find.len) {
        L_Str tail = l_str_from(s.data + pos, s.len - pos);
        ptrdiff_t idx = l_str_find(tail, find);
        if (idx < 0) break;
        count++;
        pos += (size_t)idx + find.len;
    }
    if (count == 0) return l_str_dup(a, s);
    size_t newlen = s.len - count * find.len + count * repl.len;
    char *out = (char *)l_arena_alloc(a, newlen);
    if (!out) return l_str_null();
    size_t src = 0, dst = 0;
    while (src < s.len) {
        L_Str tail = l_str_from(s.data + src, s.len - src);
        ptrdiff_t idx = l_str_find(tail, find);
        if (idx < 0) break;
        if (idx > 0) { l_memcpy(out + dst, s.data + src, (size_t)idx); dst += (size_t)idx; }
        if (repl.len) { l_memcpy(out + dst, repl.data, repl.len); dst += repl.len; }
        src += (size_t)idx + find.len;
    }
    if (src < s.len) { l_memcpy(out + dst, s.data + src, s.len - src); dst += s.len - src; }
    return l_str_from(out, dst);
}

// L_Buf helpers for L_Str
static inline int l_buf_push_str(L_Buf *b, L_Str s) {
    return s.len ? l_buf_push(b, s.data, s.len) : 0;
}
static inline int l_buf_push_cstr(L_Buf *b, const char *s) {
    return s ? l_buf_push(b, s, l_strlen(s)) : 0;
}
static inline int l_buf_push_int(L_Buf *b, int value) {
    char tmp[12];
    l_itoa(value, tmp, 10);
    return l_buf_push(b, tmp, l_strlen(tmp));
}
static inline L_Str l_buf_as_str(const L_Buf *b) {
    return l_str_from((const char *)b->data, b->len);
}

// --- L_Map: arena-backed hash table (FNV-1a, open addressing, linear probing) ---

static inline unsigned int l_map_hash(const char *key, size_t len) {
    unsigned int h = 2166136261u;
    for (size_t i = 0; i < len; i++) {
        h ^= (unsigned char)key[i];
        h *= 16777619u;
    }
    return h;
}

static inline L_Map l_map_init(L_Arena *a, size_t capacity) {
    L_Map m;
    // Round up to power of 2
    size_t cap = 16;
    while (cap < capacity) cap <<= 1;
    m.arena = a;
    m.cap = cap;
    m.len = 0;
    m.slots = (L_MapSlot *)l_arena_alloc(a, cap * sizeof(L_MapSlot));
    if (m.slots) l_memset(m.slots, 0, cap * sizeof(L_MapSlot));
    return m;
}

static inline void *l_map_get(L_Map *m, const char *key, size_t keylen) {
    if (!m->slots || m->cap == 0) return (void *)0;
    unsigned int h = l_map_hash(key, keylen);
    size_t mask = m->cap - 1;
    size_t idx = h & mask;
    for (size_t i = 0; i < m->cap; i++) {
        L_MapSlot *s = &m->slots[(idx + i) & mask];
        if (s->occupied == 0) return (void *)0;
        if (s->occupied == 1 && s->hash == h && s->keylen == keylen && l_memcmp(s->key, key, keylen) == 0)
            return s->value;
    }
    return (void *)0;
}

static inline int l_map_put(L_Map *m, const char *key, size_t keylen, void *value) {
    if (!m->slots || m->cap == 0) return -1;
    unsigned int h = l_map_hash(key, keylen);
    size_t mask = m->cap - 1;
    size_t idx = h & mask;
    size_t first_tomb = m->cap; /* sentinel: index of first tombstone seen */
    for (size_t i = 0; i < m->cap; i++) {
        size_t si = (idx + i) & mask;
        L_MapSlot *s = &m->slots[si];
        if (s->occupied == 0) {
            /* Empty slot: capacity check only needed for new insertions. */
            if (m->len * 4 >= m->cap * 3) return -1;
            size_t target = (first_tomb < m->cap) ? first_tomb : si;
            L_MapSlot *t = &m->slots[target];
            t->key = key; t->keylen = keylen; t->value = value; t->hash = h; t->occupied = 1;
            m->len++;
            return 0;
        }
        if (s->occupied == 2 && first_tomb == m->cap) first_tomb = si;
        if (s->occupied == 1 && s->hash == h && s->keylen == keylen && l_memcmp(s->key, key, keylen) == 0) {
            s->value = value; /* update existing — no capacity check needed */
            return 0;
        }
    }
    /* Probe chain exhausted with no empty slot; use tombstone if available. */
    if (first_tomb < m->cap) {
        if (m->len * 4 >= m->cap * 3) return -1;
        L_MapSlot *t = &m->slots[first_tomb];
        t->key = key; t->keylen = keylen; t->value = value; t->hash = h; t->occupied = 1;
        m->len++;
        return 0;
    }
    return -1;
}

static inline int l_map_del(L_Map *m, const char *key, size_t keylen) {
    if (!m->slots || m->cap == 0) return -1;
    unsigned int h = l_map_hash(key, keylen);
    size_t mask = m->cap - 1;
    size_t idx = h & mask;
    for (size_t i = 0; i < m->cap; i++) {
        L_MapSlot *s = &m->slots[(idx + i) & mask];
        if (s->occupied == 0) return -1;
        if (s->occupied == 1 && s->hash == h && s->keylen == keylen && l_memcmp(s->key, key, keylen) == 0) {
            s->occupied = 2; // tombstone
            m->len--;
            return 0;
        }
    }
    return -1;
}

// --- L_Tm: time conversion ---

static inline L_Tm l_gmtime(long long timestamp) {
    L_Tm tm;
    long long secs = timestamp;
    int sign = 1;
    if (secs < 0) { sign = -1; secs = -secs; }

    long long day_secs = secs % 86400;
    if (sign < 0 && day_secs > 0) day_secs = 86400 - day_secs;
    tm.sec = (int)(day_secs % 60);
    tm.min = (int)((day_secs / 60) % 60);
    tm.hour = (int)(day_secs / 3600);

    long long days = timestamp / 86400;
    if (timestamp < 0 && (timestamp % 86400) != 0) days--;

    tm.wday = (int)((days + 4) % 7); // Jan 1 1970 = Thursday
    if (tm.wday < 0) tm.wday += 7;

    // Civil date from day count (Rata Die algorithm variant for Unix epoch)
    long long z = days + 719468; // days from 0000-03-01
    long long era = (z >= 0 ? z : z - 146096) / 146097;
    long long doe = z - era * 146097; // day of era [0, 146096]
    long long yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; // year of era
    long long y = yoe + era * 400;
    long long doy = doe - (365*yoe + yoe/4 - yoe/100); // day of year [0, 365]
    long long mp = (5*doy + 2) / 153; // month [0, 11], March=0
    long long d = doy - (153*mp + 2) / 5 + 1;
    long long m = mp + (mp < 10 ? 3 : -9);
    if (m <= 2) y++;

    tm.year = (int)(y - 1900);
    tm.mon = (int)(m - 1);
    tm.mday = (int)d;

    // Day of year
    {
        static const int mdays[] = {0,31,59,90,120,151,181,212,243,273,304,334};
        tm.yday = mdays[tm.mon] + tm.mday - 1;
        // leap year check
        int yr = tm.year + 1900;
        if (tm.mon > 1 && ((yr % 4 == 0 && yr % 100 != 0) || yr % 400 == 0))
            tm.yday++;
    }

    return tm;
}

static inline L_Tm l_localtime(long long timestamp) {
    // Get timezone offset
    long long offset = 0;
#ifdef _WIN32
    TIME_ZONE_INFORMATION tzi;
    DWORD r = GetTimeZoneInformation(&tzi);
    offset = -(long long)tzi.Bias * 60;
    if (r == TIME_ZONE_ID_DAYLIGHT)
        offset -= (long long)tzi.DaylightBias * 60;
#else
    // Try TZ env var for simple UTC+N format, otherwise default to UTC
    const char *tz = l_getenv("TZ");
    if (tz) {
        // Parse simple formats: "UTC+5", "UTC-3", "EST5", etc.
        const char *p = tz;
        while (*p && *p != '+' && *p != '-') p++;
        if (*p == '+' || *p == '-') {
            int neg = (*p == '+'); // POSIX: UTC+5 means 5 hours WEST (negative offset)
            p++;
            int hours = 0;
            while (*p >= '0' && *p <= '9') { hours = hours * 10 + (*p - '0'); p++; }
            offset = (long long)hours * 3600;
            if (neg) offset = -offset;
        }
    }
#endif
    return l_gmtime(timestamp + offset);
}

static inline long long l_mktime(L_Tm *tm) {
    // Convert broken-down UTC time to Unix timestamp
    long long y = (long long)tm->year + 1900;
    int m = tm->mon + 1; // 1-based month
    // Adjust year/month so March=1 for leap year calculation
    if (m <= 2) { y--; m += 12; }
    // Days from epoch using Rata Die variant
    long long era = (y >= 0 ? y : y - 399) / 400;
    long long yoe = y - era * 400; // year of era [0, 399]
    long long doy = (153 * (m - 3) + 2) / 5 + tm->mday - 1; // day of year [0, 365]
    long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; // day of era [0, 146096]
    long long days = era * 146097 + doe - 719468; // days from Unix epoch
    return days * 86400 + (long long)tm->hour * 3600 + (long long)tm->min * 60 + (long long)tm->sec;
}

static inline int l_strftime(char *buf, size_t max, const char *fmt, const L_Tm *tm) {
    static const char *wdays[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    static const char *months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
    size_t pos = 0;
    if (max == 0) return 0;
    for (const char *f = fmt; *f && pos < max - 1; f++) {
        if (*f != '%') { buf[pos++] = *f; continue; }
        f++;
        if (!*f) break;
        char tmp[8];
        const char *s = tmp;
        int len = 0;
        switch (*f) {
            case 'Y': { int y = tm->year + 1900;
                tmp[0]=(char)('0'+y/1000); tmp[1]=(char)('0'+(y/100)%10);
                tmp[2]=(char)('0'+(y/10)%10); tmp[3]=(char)('0'+y%10);
                len = 4; break; }
            case 'm': tmp[0]=(char)('0'+(tm->mon+1)/10); tmp[1]=(char)('0'+(tm->mon+1)%10); len=2; break;
            case 'd': tmp[0]=(char)('0'+tm->mday/10); tmp[1]=(char)('0'+tm->mday%10); len=2; break;
            case 'H': tmp[0]=(char)('0'+tm->hour/10); tmp[1]=(char)('0'+tm->hour%10); len=2; break;
            case 'M': tmp[0]=(char)('0'+tm->min/10); tmp[1]=(char)('0'+tm->min%10); len=2; break;
            case 'S': tmp[0]=(char)('0'+tm->sec/10); tmp[1]=(char)('0'+tm->sec%10); len=2; break;
            case 'a': s = wdays[tm->wday % 7]; len = 3; break;
            case 'b': s = months[tm->mon % 12]; len = 3; break;
            case '%': tmp[0] = '%'; len = 1; break;
            default: tmp[0] = '%'; tmp[1] = *f; len = 2; break;
        }
        for (int i = 0; i < len && pos < max - 1; i++) buf[pos++] = s[i];
    }
    buf[pos] = '\0';
    return (int)pos;
}

// --- Convenience file/IO helpers ---

static inline long long l_file_size(const char *path) {
    L_Stat st;
    if (l_stat(path, &st) != 0) return -1;
    return (long long)st.st_size;
}

static inline ptrdiff_t l_read_all(L_FD fd, void *buf, size_t count) {
    size_t total = 0;
    while (total < count) {
        ptrdiff_t n = l_read(fd, (char *)buf + total, count - total);
        if (n <= 0) { if (total > 0) return (ptrdiff_t)total; return n; }
        total += (size_t)n;
    }
    return (ptrdiff_t)total;
}

static inline ptrdiff_t l_write_all(L_FD fd, const void *buf, size_t count) {
    size_t total = 0;
    while (total < count) {
        ptrdiff_t n = l_write(fd, (const char *)buf + total, count - total);
        if (n <= 0) return n;
        total += (size_t)n;
    }
    return (ptrdiff_t)total;
}

// --- l_fnmatch: glob pattern matching ---

static inline int l_fnmatch(const char *pattern, const char *string) {
    const char *p = pattern, *s = string;
    const char *star_p = (void *)0, *star_s = (void *)0;

    while (*s) {
        if (*p == '\\' && *(p+1)) {
            p++;
            if (*p == *s) { p++; s++; continue; }
            goto backtrack;
        }
        if (*p == '*') {
            star_p = ++p;
            star_s = s;
            continue;
        }
        if (*p == '?') { p++; s++; continue; }
        if (*p == '[') {
            p++;
            int negate = 0, match = 0;
            if (*p == '!' || *p == '^') { negate = 1; p++; }
            while (*p && *p != ']') {
                char lo = *p++;
                if (*p == '-' && *(p+1) && *(p+1) != ']') {
                    p++; // skip '-'
                    char hi = *p++;
                    if ((unsigned char)*s >= (unsigned char)lo && (unsigned char)*s <= (unsigned char)hi)
                        match = 1;
                } else {
                    if (*s == lo) match = 1;
                }
            }
            if (*p == ']') p++;
            if (negate ? match : !match) goto backtrack;
            s++;
            continue;
        }
        if (*p == *s) { p++; s++; continue; }
    backtrack:
        if (star_p) {
            p = star_p;
            s = ++star_s;
            continue;
        }
        return -1;
    }
    while (*p == '*') p++;
    return *p ? -1 : 0;
}

// --- l_glob: single-level wildcard expansion ---

static inline int l_glob(const char *pattern, L_Str **out_paths, int *out_count, L_Arena *a) {
    char dirbuf[L_PATH_MAX];
    char joinbuf[L_PATH_MAX];
    const char *base = l_basename(pattern);
    const char *dir;
    int count = 0;
    L_Dir d;
    L_DirEntry *ent;

    // Split pattern into directory + filename parts
    if (base == pattern) {
        dir = ".";
    } else {
        l_dirname(pattern, dirbuf, sizeof(dirbuf));
        dir = dirbuf;
    }

    if (l_opendir(dir, &d) != 0) {
        *out_paths = (L_Str *)(void *)0;
        *out_count = 0;
        return -1;
    }

    // First pass: count matches
    while ((ent = l_readdir(&d)) != (void *)0) {
        if (ent->d_name[0] == '.' && base[0] != '.') continue;
        if (l_fnmatch(base, ent->d_name) == 0) count++;
    }
    l_closedir(&d);

    if (count == 0) {
        *out_paths = (L_Str *)(void *)0;
        *out_count = 0;
        return 0;
    }

    *out_paths = (L_Str *)l_arena_alloc(a, (size_t)count * sizeof(L_Str));
    if (!*out_paths) { *out_count = 0; return -1; }

    // Second pass: collect matches
    if (l_opendir(dir, &d) != 0) { *out_count = 0; return -1; }
    int idx = 0;
    while ((ent = l_readdir(&d)) != (void *)0 && idx < count) {
        if (ent->d_name[0] == '.' && base[0] != '.') continue;
        if (l_fnmatch(base, ent->d_name) == 0) {
            l_path_join(joinbuf, sizeof(joinbuf), dir, ent->d_name);
            (*out_paths)[idx++] = l_str_from_cstr(a, joinbuf);
        }
    }
    l_closedir(&d);
    *out_count = idx;
    return idx;
}

// --- l_system: execute shell command ---

static inline int l_system(const char *cmd) {
    L_PID pid;
    int exitcode = -1;
#ifdef _WIN32
    char *argv[4];
    argv[0] = (char *)"cmd.exe";
    argv[1] = (char *)"/c";
    argv[2] = (char *)cmd;
    argv[3] = (char *)(void *)0;
    pid = l_spawn("cmd.exe", argv, (char *const *)(void *)0);
#else
    char *argv[4];
    argv[0] = (char *)"/bin/sh";
    argv[1] = (char *)"-c";
    argv[2] = (char *)cmd;
    argv[3] = (char *)(void *)0;
    pid = l_spawn("/bin/sh", argv, (char *const *)(void *)0);
#endif
    if (pid < 0) return -1;
    if (l_wait(pid, &exitcode) != 0) return -1;
    return exitcode;
}

// --- L_Sha256: SHA-256 implementation ---

static inline void l_sha256_init(L_Sha256 *ctx) {
    ctx->state[0] = 0x6a09e667u; ctx->state[1] = 0xbb67ae85u;
    ctx->state[2] = 0x3c6ef372u; ctx->state[3] = 0xa54ff53au;
    ctx->state[4] = 0x510e527fu; ctx->state[5] = 0x9b05688cu;
    ctx->state[6] = 0x1f83d9abu; ctx->state[7] = 0x5be0cd19u;
    ctx->count = 0;
    l_memset(ctx->buf, 0, 64);
}

static inline unsigned int l_sha256_rotr(unsigned int x, int n) {
    return (x >> n) | (x << (32 - n));
}

static inline void l_sha256_transform(unsigned int state[8], const unsigned char block[64]) {
    static const unsigned int K[64] = {
        0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
        0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
        0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
        0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
        0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
        0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
        0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
        0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
    };
    unsigned int w[64], a, b, c, d, e, f, g, h;
    for (int i = 0; i < 16; i++)
        w[i] = ((unsigned int)block[i*4]<<24) | ((unsigned int)block[i*4+1]<<16) |
               ((unsigned int)block[i*4+2]<<8) | (unsigned int)block[i*4+3];
    for (int i = 16; i < 64; i++) {
        unsigned int s0 = l_sha256_rotr(w[i-15],7) ^ l_sha256_rotr(w[i-15],18) ^ (w[i-15]>>3);
        unsigned int s1 = l_sha256_rotr(w[i-2],17) ^ l_sha256_rotr(w[i-2],19) ^ (w[i-2]>>10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    a=state[0]; b=state[1]; c=state[2]; d=state[3];
    e=state[4]; f=state[5]; g=state[6]; h=state[7];
    for (int i = 0; i < 64; i++) {
        unsigned int S1 = l_sha256_rotr(e,6) ^ l_sha256_rotr(e,11) ^ l_sha256_rotr(e,25);
        unsigned int ch = (e & f) ^ (~e & g);
        unsigned int t1 = h + S1 + ch + K[i] + w[i];
        unsigned int S0 = l_sha256_rotr(a,2) ^ l_sha256_rotr(a,13) ^ l_sha256_rotr(a,22);
        unsigned int maj = (a & b) ^ (a & c) ^ (b & c);
        unsigned int t2 = S0 + maj;
        h=g; g=f; f=e; e=d+t1; d=c; c=b; b=a; a=t1+t2;
    }
    state[0]+=a; state[1]+=b; state[2]+=c; state[3]+=d;
    state[4]+=e; state[5]+=f; state[6]+=g; state[7]+=h;
}

static inline void l_sha256_update(L_Sha256 *ctx, const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    size_t fill = (size_t)(ctx->count & 63);
    ctx->count += len;
    if (fill && fill + len >= 64) {
        size_t n = 64 - fill;
        l_memcpy(ctx->buf + fill, p, n);
        l_sha256_transform(ctx->state, ctx->buf);
        p += n; len -= n; fill = 0;
    }
    while (len >= 64) {
        l_sha256_transform(ctx->state, p);
        p += 64; len -= 64;
    }
    if (len) l_memcpy(ctx->buf + fill, p, len);
}

static inline void l_sha256_final(L_Sha256 *ctx, unsigned char hash[32]) {
    unsigned long long bits = ctx->count * 8;
    size_t fill = (size_t)(ctx->count & 63);
    ctx->buf[fill++] = 0x80;
    if (fill > 56) {
        l_memset(ctx->buf + fill, 0, 64 - fill);
        l_sha256_transform(ctx->state, ctx->buf);
        fill = 0;
    }
    l_memset(ctx->buf + fill, 0, 56 - fill);
    for (int i = 0; i < 8; i++)
        ctx->buf[56 + i] = (unsigned char)(bits >> (56 - i * 8));
    l_sha256_transform(ctx->state, ctx->buf);
    for (int i = 0; i < 8; i++) {
        hash[i*4]   = (unsigned char)(ctx->state[i] >> 24);
        hash[i*4+1] = (unsigned char)(ctx->state[i] >> 16);
        hash[i*4+2] = (unsigned char)(ctx->state[i] >> 8);
        hash[i*4+3] = (unsigned char)(ctx->state[i]);
    }
}

static inline void l_sha256(const void *data, size_t len, unsigned char hash[32]) {
    L_Sha256 ctx;
    l_sha256_init(&ctx);
    l_sha256_update(&ctx, data, len);
    l_sha256_final(&ctx, hash);
}

// --- Base64 encode / decode ---

static inline ptrdiff_t l_base64_encode(const void *data, size_t len, char *out, size_t outsz)
{
    static const char enc[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t outlen = ((len + 2) / 3) * 4;
    if (!out || outsz < outlen + 1) return -1;

    const unsigned char *p = (const unsigned char *)data;
    char *q = out;
    size_t i;
    for (i = 0; i + 2 < len; i += 3) {
        unsigned int v = ((unsigned int)p[i] << 16) |
                         ((unsigned int)p[i+1] << 8) | p[i+2];
        *q++ = enc[(v >> 18) & 0x3f];
        *q++ = enc[(v >> 12) & 0x3f];
        *q++ = enc[(v >>  6) & 0x3f];
        *q++ = enc[ v        & 0x3f];
    }
    if (i < len) {
        unsigned int v = (unsigned int)p[i] << 16;
        if (i + 1 < len) v |= (unsigned int)p[i+1] << 8;
        *q++ = enc[(v >> 18) & 0x3f];
        *q++ = enc[(v >> 12) & 0x3f];
        *q++ = (i + 1 < len) ? enc[(v >> 6) & 0x3f] : '=';
        *q++ = '=';
    }
    *q = '\0';
    return (ptrdiff_t)outlen;
}

static inline ptrdiff_t l_base64_decode(const char *in, size_t inlen, void *out, size_t outsz)
{
    /* 6-bit decode table: 255 = invalid, 64 = padding/whitespace */
    static const unsigned char dec[256] = {
        255,255,255,255,255,255,255,255,255, 64,64,255,255, 64,255,255, /* 0x00-0x0f */
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255, /* 0x10-0x1f */
         64,255,255,255,255,255,255,255,255,255,255, 62,255, 62,255, 63, /* 0x20-0x2f: sp,+,-,/ */
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255, 64,255,255, /* 0x30-0x3f: 0-9,= */
        255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /* 0x40-0x4f: A-O */
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255, 63, /* 0x50-0x5f: P-Z,_ */
        255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /* 0x60-0x6f: a-o */
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255, /* 0x70-0x7f: p-z */
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
        255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    };

    unsigned char *dst = (unsigned char *)out;
    size_t written = 0;
    unsigned int acc = 0;
    int bits = 0;

    for (size_t i = 0; i < inlen; i++) {
        unsigned char c = dec[(unsigned char)in[i]];
        if (c == 64) continue;   /* whitespace or padding — skip */
        if (c == 255) return -1; /* invalid character */
        acc = (acc << 6) | c;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            if (written >= outsz) return -1;
            dst[written++] = (unsigned char)((acc >> bits) & 0xff);
        }
    }
    return (ptrdiff_t)written;
}

static inline int l_path_exists(const char *path) {
    return l_access(path, L_F_OK) == 0 ? 1 : 0;
}

static inline int l_path_isdir(const char *path) {
    L_Stat st;
    if (l_stat(path, &st) != 0) return 0;
    return L_S_ISDIR(st.st_mode) ? 1 : 0;
}

#endif // L_OSH

// Provide non-inline linker symbols for memset/memcpy/memmove.
// Compilers (especially clang LTO on ARM) emit calls to these for
// large struct copies/zeroing even in freestanding mode.
#ifdef L_MAINFILE
#ifndef L_MEMFUNCS_DONE
#define L_MEMFUNCS_DONE
#undef memset
#undef memcpy
#undef memmove
#undef memcmp
#undef strlen
void *memset(void *dst, int b, size_t len) { return l_memset(dst, b, len); }
void *memcpy(void *dst, const void *src, size_t len) { return l_memcpy(dst, src, len); }
void *memmove(void *dst, const void *src, size_t len) { return l_memmove(dst, src, len); }
int memcmp(const void *a, const void *b, size_t n) { return l_memcmp(a, b, n); }
size_t strlen(const char *s) { return l_strlen(s); }
#ifndef L_DONTOVERRIDE
#define memset l_memset
#define memcpy l_memcpy
#define memmove l_memmove
#endif
#endif // L_MEMFUNCS_DONE
#endif // L_MAINFILE
