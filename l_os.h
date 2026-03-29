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

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
typedef DWORD mode_t;

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

// Portable stat struct
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

// CLang warns for 'asm'
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif

#ifdef L_WITHDEFS

// String functions
/// Returns the length of a wide character string
size_t l_wcslen(const wchar_t *s);
/// Returns the length of a null-terminated string
size_t l_strlen(const char *str);
/// Copies src string to dst, returns dst
char *l_strcpy(char *dst, const char *src);
/// Copies up to n characters from src to dst, padding with nulls
char *l_strncpy(char *dst, const char *src, size_t n);
/// Appends src string to dst, returns dst
char *l_strcat(char *dst, const char *src);
/// Appends at most n characters of src to dst, always null-terminates, returns dst
char *l_strncat(char *dst, const char *src, size_t n);
/// Returns pointer to first occurrence of c in s, or NULL
char *l_strchr(const char *s, int c);
/// Returns pointer to last occurrence of c in s, or NULL
char *l_strrchr(const char *s, int c);
/// Returns pointer to first occurrence of s2 in s1, or NULL
char *l_strstr(const char *s1, const char *s2);
/// Compares two strings, returns <0, 0, or >0
int l_strcmp(const char *s1, const char *s2);
/// Compares up to n characters of two strings
int l_strncmp(const char *s1, const char *s2, size_t n);
/// Case-insensitive string comparison
int l_strcasecmp(const char *s1, const char *s2);
/// Case-insensitive comparison of up to n characters
int l_strncasecmp(const char *s1, const char *s2, size_t n);
/// Returns length of initial segment of s consisting entirely of bytes in accept
size_t l_strspn(const char *s, const char *accept);
/// Returns length of initial segment of s consisting entirely of bytes NOT in reject
size_t l_strcspn(const char *s, const char *reject);
/// Returns pointer to first occurrence in s of any character in accept, or NULL
char *l_strpbrk(const char *s, const char *accept);
/// Splits str into tokens delimited by any char in delim; saves state in *saveptr (reentrant)
char *l_strtok_r(char *str, const char *delim, char **saveptr);
/// Returns pointer to the filename component of path (after last '/' or '\')
const char *l_basename(const char *path);
/// Writes the directory component of path into buf (up to bufsize), returns buf
char *l_dirname(const char *path, char *buf, size_t bufsize);
/// Reverses a string in place
void l_reverse(char str[], int length);

// Conversion functions
/// Returns non-zero if c is a whitespace character (space, tab, newline, etc.)
int l_isspace(int c);
/// Returns non-zero if c is a digit ('0'-'9')
int l_isdigit(int c);
/// Returns non-zero if c is an alphabetic character ('A'-'Z' or 'a'-'z')
int l_isalpha(int c);
/// Returns non-zero if c is alphanumeric (l_isalpha or l_isdigit)
int l_isalnum(int c);
/// Returns non-zero if c is an uppercase letter ('A'-'Z')
int l_isupper(int c);
/// Returns non-zero if c is a lowercase letter ('a'-'z')
int l_islower(int c);
/// Converts c to uppercase; returns c unchanged if not a lowercase letter
int l_toupper(int c);
/// Converts c to lowercase; returns c unchanged if not an uppercase letter
int l_tolower(int c);
/// Converts a string to a long integer, skipping leading whitespace
long l_atol(const char *s);
/// Converts a string to an integer
int l_atoi(const char *s);
/// Converts a string to an unsigned long, auto-detecting base when base==0 (0x=hex, 0=octal, else decimal); sets *endptr past last digit
unsigned long l_strtoul(const char *nptr, char **endptr, int base);
/// Converts a string to a long, auto-detecting base when base==0; handles leading sign; sets *endptr past last digit
long l_strtol(const char *nptr, char **endptr, int base);
/// Converts a string to an unsigned long long (64-bit); auto-detects base when base==0; sets *endptr past last digit
unsigned long long l_strtoull(const char *nptr, char **endptr, int base);
/// Converts a string to a long long (64-bit); auto-detects base when base==0; handles leading sign; sets *endptr past last digit
long long l_strtoll(const char *nptr, char **endptr, int base);
/// Converts an integer to a string in the given radix (2-36)
char *l_itoa(int in, char* buffer, int radix);

// Memory functions
/// Copies len bytes from src to dst, handling overlapping regions
void *l_memmove(void *dst, const void *src, size_t len);
/// Fills len bytes of dst with byte value b
void *l_memset(void *dst, int b, size_t len);
/// Compares n bytes of s1 and s2, returns <0, 0, or >0
int l_memcmp(const void *s1, const void *s2, size_t n);
/// Copies len bytes from src to dst
void *l_memcpy(void *dst, const void *src, size_t len);
/// Finds first occurrence of byte c in the first n bytes of s, or NULL
void *l_memchr(const void *s, int c, size_t n);
/// Finds last occurrence of byte c in the first n bytes of s, or NULL
void *l_memrchr(const void *s, int c, size_t n);
/// Returns the length of s, but at most maxlen (does not scan past maxlen bytes)
size_t l_strnlen(const char *s, size_t maxlen);
/// Finds first occurrence of needle (needlelen bytes) in haystack (haystacklen bytes), or NULL
void *l_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);

// Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)
#ifdef L_WITHSNPRINTF
/// Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written
static int l_vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);
/// Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written
static int l_snprintf(char *buf, size_t n, const char *fmt, ...);
#endif

// System functions
/// Terminates the process with the given status code
noreturn void l_exit(int status);
/// Opens a file with the given flags and mode, returns file descriptor
L_FD l_open(const char *path, int flags, mode_t mode);
/// Closes a file descriptor
int l_close(L_FD fd);
/// Reads up to count bytes from fd into buf
ssize_t l_read(L_FD fd, void *buf, size_t count);
/// Writes up to count bytes from buf to fd
ssize_t l_write(L_FD fd, const void *buf, size_t count);
/// Writes a string to stdout
void l_puts(const char* s);
/// Exits with code and message if condition is true
void l_exitif(bool condition, int code, char *message);
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

// Convenience file openers
/// Opens a file for reading
L_FD l_open_read(const char* file);
/// Opens or creates a file for writing
L_FD l_open_write(const char* file);
/// Opens or creates a file for reading and writing
L_FD l_open_readwrite(const char* file);
/// Opens or creates a file for appending
L_FD l_open_append(const char* file);
/// Opens or creates a file, truncating to zero length
L_FD l_open_trunc(const char* file);

// Error reporting
/// Returns the error code from the most recent failed syscall (0 if last call succeeded)
static inline int l_errno(void);
/// Returns a human-readable string for the given error code
static inline const char *l_strerror(int errnum);

// Terminal and timing functions (cross-platform)
/// Sleeps for the given number of milliseconds
void l_sleep_ms(unsigned int ms);
/// Sets stdin to raw mode (no echo, no line buffering), returns old mode
static unsigned long l_term_raw(void);
/// Restores terminal mode from value returned by l_term_raw
static void l_term_restore(unsigned long old_mode);
/// Reads from fd without blocking, returns 0 if no data available
ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count);
/// Gets terminal size in rows and columns
void l_term_size(int *rows, int *cols);

// File system functions (cross-platform)
/// Deletes a file, returns 0 on success, -1 on error
int l_unlink(const char *path);
/// Removes an empty directory, returns 0 on success, -1 on error
int l_rmdir(const char *path);
/// Renames (or moves) a file or directory. Returns 0 on success, -1 on error.
int l_rename(const char *oldpath, const char *newpath);
/// Checks access to a file. mode: L_F_OK (exists), L_R_OK, L_W_OK, L_X_OK. Returns 0 if ok, -1 on error.
int l_access(const char *path, int mode);
/// Changes permission bits of a file. Returns 0 on success, -1 on error.
int l_chmod(const char *path, mode_t mode);
/// Creates a symbolic link at linkpath pointing to target. Returns 0 on success, -1 on error.
int l_symlink(const char *target, const char *linkpath);
/// Reads the target of a symbolic link into buf (up to bufsiz bytes). Returns number of bytes read, or -1 on error.
ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz);
/// Resolves path to its canonical absolute form into resolved (at least L_PATH_MAX bytes). Returns resolved on success, NULL on error.
char *l_realpath(const char *path, char *resolved);
/// Gets file metadata by path. Returns 0 on success, -1 on error.
int l_stat(const char *path, L_Stat *st);
/// Gets file metadata by open file descriptor. Returns 0 on success, -1 on error.
int l_fstat(L_FD fd, L_Stat *st);
/// Opens a directory for reading. Returns 0 on success, -1 on error.
int l_opendir(const char *path, L_Dir *dir);
/// Reads the next directory entry. Returns pointer to L_DirEntry or NULL when done.
static L_DirEntry *l_readdir(L_Dir *dir);
/// Closes a directory handle.
void l_closedir(L_Dir *dir);

/// Maps a file or anonymous memory into the process address space
void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset);
/// Unmaps a previously mapped region
int l_munmap(void *addr, size_t length);

/// Gets the current working directory into buf (up to size bytes). Returns buf on success, NULL on error.
char *l_getcwd(char *buf, size_t size);
/// Changes the current working directory
int l_chdir(const char *path);

/// Creates a pipe. fds[0] is the read end, fds[1] is the write end. Returns 0 on success, -1 on error.
int l_pipe(L_FD fds[2]);

/// Duplicates fd, returning a new descriptor on success or -1 on error.
int l_dup(L_FD fd);

/// Duplicates oldfd onto newfd. Returns newfd on success, -1 on error.
int l_dup2(L_FD oldfd, L_FD newfd);

/// Spawns a new process with explicit stdio. Use L_SPAWN_INHERIT to keep the parent's stream.
static L_PID l_spawn_stdio(const char *path, char *const argv[], char *const envp[],
                    L_FD stdin_fd, L_FD stdout_fd, L_FD stderr_fd);

/// Spawns a new process, inheriting the current stdio descriptors.
/// Returns process ID/handle on success, -1 on error.
/// path: executable path. argv: NULL-terminated argument array. envp: NULL-terminated environment (NULL = inherit).
L_PID l_spawn(const char *path, char *const argv[], char *const envp[]);

/// Waits for a spawned process to finish. Returns 0 on success, -1 on error.
/// exitcode receives the process exit code.
int l_wait(L_PID pid, int *exitcode);

#ifdef __unix__
// Unix-only functions
/// Repositions the file offset of fd
off_t l_lseek(L_FD fd, off_t offset, int whence);
/// Creates a directory with the given permissions
int l_mkdir(const char *path, mode_t mode);
/// Yields the processor to other threads
int l_sched_yield(void);
/// Fork the current process. Returns child pid to parent, 0 to child, -1 on error.
L_PID l_fork(void);
/// Replace the current process image. Does not return on success.
int l_execve(const char *path, char *const argv[], char *const envp[]);
/// Wait for a child process. Returns child pid on success, -1 on error.
L_PID l_waitpid(L_PID pid, int *status, int options);
#endif

#endif // L_WITHDEFS

#ifdef L_WITHSTART

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
#endif

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
#  define atol l_atol
#  define atoi l_atoi
#  define strtoul l_strtoul
#  define strtol l_strtol
#  define strtoull l_strtoull
#  define strtoll l_strtoll
#  define itoa l_itoa

#  define memmove l_memmove
#  define memset l_memset
#  define memcmp l_memcmp
#  define memcpy l_memcpy
#  define memchr l_memchr
#  define memrchr l_memrchr
#  define strnlen l_strnlen
#  define memmem  l_memmem

#  define vsnprintf l_vsnprintf
#  define snprintf  l_snprintf

#  define exit l_exit
#  define close l_close
#  define read l_read
#  define write l_write
#  define puts l_puts
#  define lseek l_lseek
#  define dup l_dup
#  define mkdir l_mkdir
#  define chdir l_chdir
#  define getcwd l_getcwd
#  define pipe l_pipe
#  define dup2 l_dup2
#  define sched_yield l_sched_yield
#  define unlink l_unlink
#  define rmdir l_rmdir
#  define rename l_rename
#  define access l_access
#  define chmod l_chmod
#  define symlink l_symlink
#  define readlink l_readlink
#  define realpath l_realpath
#  define PATH_MAX L_PATH_MAX
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

// Option parser state (single-threaded; no TLS in freestanding)
/// Points to the argument of the current option (set by l_getopt)
static char *l_optarg;
/// Index of the next argv element to process (starts at 1)
static int   l_optind = 1;
/// Reserved for POSIX compat; l_getopt itself does no I/O
static int   l_opterr = 1;
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

inline size_t l_wcslen(const wchar_t *s) {
    size_t len = 0;
    while(s[len] != L'\0') ++len;
    return len;
}

inline size_t l_strlen(const char *str)
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

inline void *l_memmove(void *dst, const void *src, size_t len)
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

inline void *l_memset(void *dst, int b, size_t len)
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

inline int l_memcmp(const void *s1, const void *s2, size_t n)
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

inline char *l_strcpy(char *dst, const char *src)
{
    char *ret = dst;

    while ((*dst++ = *src++));
    return ret;
}

inline char *l_strncpy(char *dst, const char *src, size_t n)
{
    char *ret = dst;
    size_t i;

    for (i = 0; i < n && src[i] != '\0'; i++)
        dst[i] = src[i];
    for (; i < n; i++)
        dst[i] = '\0';
    return ret;
}

inline char *l_strcat(char *dst, const char *src)
{
    char *ret = dst;

    while (*dst)
        dst++;
    while ((*dst++ = *src++));
    return ret;
}

inline char *l_strncat(char *dst, const char *src, size_t n)
{
    char *ret = dst;

    while (*dst)
        dst++;
    while (n-- && *src)
        *dst++ = *src++;
    *dst = '\0';
    return ret;
}

inline char *l_strchr(const char *s, int c)
{
    while (*s) {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return (char)c == '\0' ? (char *)s : NULL;
}

inline char *l_strrchr(const char *s, int c)
{
    const char *ret = NULL;

    while (*s) {
        if (*s == (char)c)
            ret = s;
        s++;
    }
    return (char)c == '\0' ? (char *)s : (char *)ret;
}

inline int l_strncmp(const char *s1, const char *s2, size_t n) {
    unsigned char u1, u2;

    while (n-- > 0)
    {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
    return 0;
}

inline int l_strcmp(const char *s1, const char *s2) {
    unsigned char u1, u2;

    for (;;) {
        u1 = (unsigned char) *s1++;
        u2 = (unsigned char) *s2++;
        if (u1 != u2)
            return u1 - u2;
        if (u1 == '\0')
            return 0;
    }
}

inline char *l_strstr(const char *s1, const char *s2) {
    const size_t len = l_strlen(s2);
    if (len == 0) return (char *)s1;
    const size_t slen = l_strlen(s1);
    if (len > slen) return NULL;
    const char *end = s1 + slen - len;
    const char first = s2[0];
    /* Fast first-byte check avoids l_memcmp on non-matching positions. */
    while (s1 <= end) {
        if (*s1 == first && !l_memcmp(s1, s2, len))
            return (char *)s1;
        ++s1;
    }
    return NULL;
}

inline int l_isspace(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

inline int l_isdigit(int c)
{
    return (unsigned int)(c - '0') <= 9;
}

inline int l_isalpha(int c)
{
    return (unsigned int)((c | 0x20) - 'a') <= ('z' - 'a');
}

inline int l_isalnum(int c)
{
    return l_isalpha(c) || l_isdigit(c);
}

inline int l_isupper(int c)
{
    return (unsigned int)(c - 'A') <= ('Z' - 'A');
}

inline int l_islower(int c)
{
    return (unsigned int)(c - 'a') <= ('z' - 'a');
}

inline int l_toupper(int c)
{
    return l_islower(c) ? c - ('a' - 'A') : c;
}

inline int l_tolower(int c)
{
    return l_isupper(c) ? c + ('a' - 'A') : c;
}

inline long l_atol(const char *s)
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

inline int l_atoi(const char *s)
{
    return (int)l_atol(s);
}

inline unsigned long l_strtoul(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long acc = 0;
    int overflow = 0;
    int any = 0;

    while (l_isspace((unsigned char)*s))
        s++;

    if (*s == '+')
        s++;

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
    return acc;
}

inline long l_strtol(const char *nptr, char **endptr, int base)
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
        if (uval > (unsigned long)LONG_MAX + 1UL)
            return LONG_MIN;
        return -(long)uval;
    }
    if (uval > (unsigned long)LONG_MAX)
        return LONG_MAX;
    return (long)uval;
}

inline unsigned long long l_strtoull(const char *nptr, char **endptr, int base)
{
    const char *s = nptr;
    unsigned long long acc = 0;
    int overflow = 0;
    int any = 0;

    while (l_isspace((unsigned char)*s))
        s++;

    if (*s == '+')
        s++;

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
    return acc;
}

inline long long l_strtoll(const char *nptr, char **endptr, int base)
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
        if (uval > (unsigned long long)LLONG_MAX + 1ULL)
            return LLONG_MIN;
        return -(long long)uval;
    }
    if (uval > (unsigned long long)LLONG_MAX)
        return LLONG_MAX;
    return (long long)uval;
}

//function to reverse a string
inline void l_reverse(char str[], int length)
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

inline char* l_itoa(int num, char* str, int base)
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

inline void *l_memcpy(void *dst, const void *src, size_t len)
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

inline void *l_memchr(const void *s, int c, size_t n)
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

inline void *l_memrchr(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s + n;
    const unsigned char uc = (unsigned char)c;

    while (n--) {
        if (*--p == uc)
            return (void *)p;
    }
    return NULL;
}

inline size_t l_strnlen(const char *s, size_t maxlen)
{
    size_t len = 0;
    while (len < maxlen && s[len] != '\0')
        len++;
    return len;
}

inline void *l_memmem(const void *haystack, size_t haystacklen,
                      const void *needle,   size_t needlelen)
{
    const unsigned char *h = (const unsigned char *)haystack;
    const unsigned char *n = (const unsigned char *)needle;

    if (needlelen == 0)
        return (void *)haystack;
    if (needlelen > haystacklen)
        return NULL;

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

        int flag_minus = 0, flag_zero = 0;
        for (;;) {
            if      (*fmt == '-') { flag_minus = 1; fmt++; }
            else if (*fmt == '0') { flag_zero  = 1; fmt++; }
            else break;
        }

        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') width = width * 10 + (*fmt++ - '0');

        int prec = -1;
        if (*fmt == '.') {
            fmt++; prec = 0;
            while (*fmt >= '0' && *fmt <= '9') prec = prec * 10 + (*fmt++ - '0');
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
            int slen = (int)l_strlen(sv);
            if (prec >= 0 && prec < slen) slen = prec;
            int pad = width - slen; if (pad < 0) pad = 0;
            if (!flag_minus) for (int i = 0; i < pad; i++) L_SNPRINTF_EMIT(' ');
            for (int i = 0; i < slen; i++) L_SNPRINTF_EMIT(sv[i]);
            if ( flag_minus) for (int i = 0; i < pad; i++) L_SNPRINTF_EMIT(' ');
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

        /* prefix: "-", "+", or "0x" for pointers */
        const char *prefix = ""; int preflen = 0;
        if (neg)    { prefix = "-";  preflen = 1; }
        if (is_ptr) { prefix = "0x"; preflen = 2; }

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

inline int l_strcasecmp(const char *s1, const char *s2) {
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

inline int l_strncasecmp(const char *s1, const char *s2, size_t n) {
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

inline size_t l_strspn(const char *s, const char *accept) {
    size_t count = 0;
    for (; *s; s++) {
        const char *a = accept;
        int found = 0;
        for (; *a; a++) {
            if (*s == *a) { found = 1; break; }
        }
        if (!found) break;
        count++;
    }
    return count;
}

inline size_t l_strcspn(const char *s, const char *reject) {
    size_t count = 0;
    for (; *s; s++) {
        const char *r = reject;
        for (; *r; r++) {
            if (*s == *r) return count;
        }
        count++;
    }
    return count;
}

inline char *l_strpbrk(const char *s, const char *accept) {
    for (; *s; s++) {
        const char *a = accept;
        for (; *a; a++) {
            if (*s == *a) return (char *)s;
        }
    }
    return (char *)0;
}

inline char *l_strtok_r(char *str, const char *delim, char **saveptr) {
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

inline const char *l_basename(const char *path) {
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

inline char *l_dirname(const char *path, char *buf, size_t bufsize) {
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
#error "Supported architectures: linux x86_64, aarch64, arm, and windows. Paste relevant nolibc.h sections for more archs."
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
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#ifndef _WIN32
static char **l_envp;
#endif

noreturn inline void l_exit(int status)
{
    my_syscall1(__NR_exit, status & 255);
    while(1);
}

inline int l_chdir(const char *path)
{
    return my_syscall1(__NR_chdir, path);
}

inline char *l_getcwd(char *buf, size_t size)
{
#if defined(__x86_64__)
    long ret = my_syscall2(79 /*__NR_getcwd*/, buf, size);
#elif defined(__aarch64__)
    long ret = my_syscall2(17 /*__NR_getcwd*/, buf, size);
#elif defined(__arm__)
    long ret = my_syscall2(183 /*__NR_getcwd*/, buf, size);
#endif
    if (ret < 0) return (char *)0;
    return buf;
}

inline int l_close(L_FD fd)
{
    int ret = my_syscall1(__NR_close, fd);
    l_set_errno_from_ret(ret);
    return ret;
}

inline int l_dup(L_FD fd)
{
    return my_syscall1(__NR_dup, fd);
}

inline int l_pipe(L_FD fds[2])
{
    int tmp[2];
#if defined(__x86_64__)
    long ret = my_syscall2(293 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#elif defined(__aarch64__)
    long ret = my_syscall2(59 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#elif defined(__arm__)
    long ret = my_syscall2(359 /*__NR_pipe2*/, tmp, O_CLOEXEC);
#endif
    if (ret < 0) return -1;
    fds[0] = (L_FD)tmp[0];
    fds[1] = (L_FD)tmp[1];
    return 0;
}

inline int l_dup2(L_FD oldfd, L_FD newfd)
{
#if defined(__x86_64__)
    return (int)my_syscall2(33 /*__NR_dup2*/, oldfd, newfd);
#elif defined(__aarch64__)
    if (oldfd == newfd) {
        L_Stat st;
        return l_fstat(oldfd, &st) < 0 ? -1 : (int)oldfd;
    }
    return (int)my_syscall3(24 /*__NR_dup3*/, oldfd, newfd, 0);
#elif defined(__arm__)
    return (int)my_syscall2(63 /*__NR_dup2*/, oldfd, newfd);
#endif
}

inline L_PID l_fork(void)
{
#if defined(__x86_64__)
    return (L_PID)my_syscall5(56 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#elif defined(__aarch64__)
    return (L_PID)my_syscall5(220 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#elif defined(__arm__)
    return (L_PID)my_syscall5(120 /*__NR_clone*/, 17 /*SIGCHLD*/, 0, 0, 0, 0);
#endif
}

inline int l_execve(const char *path, char *const argv[], char *const envp[])
{
#if defined(__x86_64__)
    return (int)my_syscall3(59 /*__NR_execve*/, path, argv, envp);
#elif defined(__aarch64__)
    return (int)my_syscall3(221 /*__NR_execve*/, path, argv, envp);
#elif defined(__arm__)
    return (int)my_syscall3(11 /*__NR_execve*/, path, argv, envp);
#endif
}

inline L_PID l_waitpid(L_PID pid, int *status, int options)
{
#if defined(__x86_64__)
    return (L_PID)my_syscall4(61 /*__NR_wait4*/, pid, status, options, 0);
#elif defined(__aarch64__)
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

inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[])
{
    return l_spawn_stdio(path, argv, envp, L_SPAWN_INHERIT, L_SPAWN_INHERIT, L_SPAWN_INHERIT);
}

inline int l_wait(L_PID pid, int *exitcode)
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

inline off_t l_lseek(L_FD fd, off_t offset, int whence)
{
    return my_syscall3(__NR_lseek, fd, offset, whence);
}

inline int l_mkdir(const char *path, mode_t mode)
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

inline int l_unlink(const char *path)
{
#ifdef __NR_unlinkat
    return my_syscall3(__NR_unlinkat, AT_FDCWD, path, 0);
#elif defined(__NR_unlink)
    return my_syscall1(__NR_unlink, path);
#else
#error Neither __NR_unlinkat nor __NR_unlink defined, cannot implement l_unlink()
#endif
}

inline int l_rmdir(const char *path)
{
#ifdef __NR_unlinkat
    return my_syscall3(__NR_unlinkat, AT_FDCWD, path, AT_REMOVEDIR);
#elif defined(__NR_rmdir)
    return my_syscall1(__NR_rmdir, path);
#else
#error Neither __NR_unlinkat nor __NR_rmdir defined, cannot implement l_rmdir()
#endif
}

inline int l_rename(const char *oldpath, const char *newpath)
{
#if defined(__x86_64__)
    return (int)my_syscall4(264 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#elif defined(__aarch64__)
    return (int)my_syscall4(38 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#elif defined(__arm__)
    return (int)my_syscall4(329 /*__NR_renameat*/, AT_FDCWD, oldpath, AT_FDCWD, newpath);
#else
#error Unsupported architecture for l_rename
#endif
}

inline int l_access(const char *path, int mode)
{
#if defined(__x86_64__)
    return (int)my_syscall4(269 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#elif defined(__aarch64__)
    return (int)my_syscall4(48 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#elif defined(__arm__)
    return (int)my_syscall4(334 /*__NR_faccessat*/, AT_FDCWD, path, mode, 0);
#else
#error Unsupported architecture for l_access
#endif
}

inline int l_chmod(const char *path, mode_t mode)
{
#if defined(__x86_64__)
    return (int)my_syscall3(268 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#elif defined(__aarch64__)
    return (int)my_syscall3(53 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#elif defined(__arm__)
    return (int)my_syscall3(333 /*__NR_fchmodat*/, AT_FDCWD, path, mode);
#else
#error Unsupported architecture for l_chmod
#endif
}

inline int l_symlink(const char *target, const char *linkpath)
{
#ifdef __NR_symlinkat
    long ret = my_syscall3(__NR_symlinkat, target, AT_FDCWD, linkpath);
#elif defined(__NR_symlink)
    long ret = my_syscall2(__NR_symlink, target, linkpath);
#else
    /* symlinkat syscall numbers by arch */
#if defined(__x86_64__)
    long ret = my_syscall3(265 /*__NR_symlinkat*/, target, AT_FDCWD, linkpath);
#elif defined(__aarch64__)
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

inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz)
{
#ifdef __NR_readlinkat
    long ret = my_syscall4(__NR_readlinkat, AT_FDCWD, path, buf, bufsiz);
#elif defined(__NR_readlink)
    long ret = my_syscall3(__NR_readlink, path, buf, bufsiz);
#else
#if defined(__x86_64__)
    long ret = my_syscall4(267 /*__NR_readlinkat*/, AT_FDCWD, path, buf, bufsiz);
#elif defined(__aarch64__)
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

inline char *l_realpath(const char *path, char *resolved)
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
#elif defined(__aarch64__)
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
#elif defined(__aarch64__)
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

inline int l_stat(const char *path, L_Stat *st)
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
#elif defined(__aarch64__)
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

inline int l_fstat(L_FD fd, L_Stat *st)
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
#elif defined(__aarch64__)
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

inline int l_opendir(const char *path, L_Dir *dir)
{
    // Use openat syscall directly to avoid forward-declaration issues
#define L_O_RDONLY    0
#define L_AT_FDCWD    (-100)
    // O_DIRECTORY differs by arch: x86_64=0x10000, ARM/AArch64=0x4000
#if defined(__x86_64__)
#define L_O_DIRECTORY 0x10000
#elif defined(__aarch64__) || defined(__arm__)
#define L_O_DIRECTORY 0x4000
#endif
#if defined(__x86_64__)
    L_FD fd = (L_FD)my_syscall4(257 /*__NR_openat*/, L_AT_FDCWD, path, L_O_RDONLY | L_O_DIRECTORY, 0);
#elif defined(__aarch64__)
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
#elif defined(__aarch64__)
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

inline void l_closedir(L_Dir *dir)
{
    l_close(dir->fd);
}

inline void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset)
{
#if defined(__x86_64__)
    long ret = my_syscall6(9 /*__NR_mmap*/, addr, length, prot, flags, fd, offset);
#elif defined(__aarch64__)
    long ret = my_syscall6(222 /*__NR_mmap*/, addr, length, prot, flags, fd, offset);
#elif defined(__arm__)
    long ret = my_syscall6(192 /*__NR_mmap2*/, addr, length, prot, flags, fd, (long)(offset >> 12));
#else
#error Unsupported architecture for l_mmap
#endif
    if (ret < 0 && ret > -4096) return L_MAP_FAILED;
    return (void *)ret;
}

inline int l_munmap(void *addr, size_t length)
{
#if defined(__x86_64__)
    return (int)my_syscall2(11 /*__NR_munmap*/, addr, length);
#elif defined(__aarch64__)
    return (int)my_syscall2(215 /*__NR_munmap*/, addr, length);
#elif defined(__arm__)
    return (int)my_syscall2(91 /*__NR_munmap*/, addr, length);
#else
#error Unsupported architecture for l_munmap
#endif
}

inline L_FD l_open(const char *path, int flags, mode_t mode)
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

inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    long ret = my_syscall3(__NR_read, fd, buf, count);
    l_set_errno_from_ret(ret);
    return ret;
}

inline int l_sched_yield(void)
{
    return my_syscall0(__NR_sched_yield);
}

// Terminal and timing support for Unix
struct l_timespec {
    long tv_sec;
    long tv_nsec;
};

inline void l_sleep_ms(unsigned int ms)
{
    struct l_timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    my_syscall2(__NR_nanosleep, &ts, (void*)0);
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

inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
{
    // With VMIN=0, VTIME=0 set by l_term_raw, read returns immediately
    return my_syscall3(__NR_read, fd, buf, count);
}

#define L_TIOCGWINSZ 0x5413
struct l_winsize { unsigned short ws_row, ws_col, ws_xpixel, ws_ypixel; };

inline void l_term_size(int *rows, int *cols)
{
    struct l_winsize ws;
    long ret = my_syscall3(__NR_ioctl, L_STDIN, L_TIOCGWINSZ, &ws);
    if (ret < 0) { *rows = 24; *cols = 80; }
    else { *rows = ws.ws_row; *cols = ws.ws_col; }
}

inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    long ret = my_syscall3(__NR_write, fd, buf, count);
    l_set_errno_from_ret(ret);
    return ret;
}

#pragma GCC diagnostic pop

inline L_FD l_open_read(const char* file) {
    return l_open(file, O_RDONLY, 0);
}

inline L_FD l_open_write(const char* file) {
    return l_open(file, O_WRONLY | O_CREAT, 0644);
}

inline L_FD l_open_readwrite(const char* file) {
    return l_open(file, O_RDWR | O_CREAT, 0644);
}

inline L_FD l_open_append(const char* file) {
    return l_open(file, O_WRONLY | O_APPEND | O_CREAT, 0600);
}

inline L_FD l_open_trunc(const char* file) {
    return l_open(file, O_WRONLY | O_TRUNC | O_CREAT, 0644);
}

#else // Windows starts here

noreturn inline void l_exit(int status)
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

inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    DWORD written;
    HANDLE out = l_win_fd_handle(fd);
    BOOL result = WriteFile(out, buf, count, &written, NULL);
    if (result) { l_set_errno(0); return written; }
    l_set_errno(l_win_error_to_errno(GetLastError()));
    return -1;
}

inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    DWORD readden;
    HANDLE in = l_win_fd_handle(fd);
    BOOL result = ReadFile(in, buf, count, &readden, NULL);
    if (result) { l_set_errno(0); return readden; }
    l_set_errno(l_win_error_to_errno(GetLastError()));
    return -1;
}


inline int l_close(L_FD fd) {
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
inline void l_sleep_ms(unsigned int ms)
{
    Sleep(ms);
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

inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
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

inline void l_term_size(int *rows, int *cols)
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

inline int l_chdir(const char *path)
{
    wchar_t wpath[512];
    if (!l_utf8_to_wide(path, wpath, 512)) return -1;
    return SetCurrentDirectoryW(wpath) ? 0 : -1;
}

inline char *l_getcwd(char *buf, size_t size)
{
    wchar_t wbuf[512];
    DWORD len = GetCurrentDirectoryW(512, wbuf);
    if (len == 0 || len >= 512) return (char *)0;
    if (!l_wide_to_utf8(wbuf, buf, (int)size)) return (char *)0;
    return buf;
}

inline int l_pipe(L_FD fds[2])
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

inline int l_dup(L_FD fd)
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

inline int l_dup2(L_FD oldfd, L_FD newfd)
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

inline off_t l_lseek(L_FD fd, off_t offset, int whence)
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

inline int l_mkdir(const char *path, mode_t mode)
{
    wchar_t wpath[1024];
    (void)mode;
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return CreateDirectoryW(wpath, NULL) ? 0 : -1;
}

inline int l_sched_yield(void)
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

inline L_PID l_spawn(const char *path, char *const argv[], char *const envp[])
{
    return l_spawn_stdio(path, argv, envp, L_SPAWN_INHERIT, L_SPAWN_INHERIT, L_SPAWN_INHERIT);
}

inline int l_wait(L_PID pid, int *exitcode)
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

inline int l_unlink(const char *path) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return DeleteFileW(wpath) ? 0 : -1;
}

inline int l_rmdir(const char *path) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    return RemoveDirectoryW(wpath) ? 0 : -1;
}

inline int l_rename(const char *oldpath, const char *newpath) {
    wchar_t wold[1024], wnew[1024];
    if (!l_utf8_to_wide(oldpath, wold, 1024)) return -1;
    if (!l_utf8_to_wide(newpath, wnew, 1024)) return -1;
    return MoveFileExW(wold, wnew, 1 /*MOVEFILE_REPLACE_EXISTING*/) ? 0 : -1;
}

inline int l_access(const char *path, int mode) {
    wchar_t wpath[1024];
    if (!l_utf8_to_wide(path, wpath, 1024)) return -1;
    DWORD attr = GetFileAttributesW(wpath);
    if (attr == (DWORD)-1) return -1;
    if ((mode & L_W_OK) && (attr & 1 /*FILE_ATTRIBUTE_READONLY*/)) return -1;
    return 0;
}

inline int l_chmod(const char *path, mode_t mode) {
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

inline int l_symlink(const char *target, const char *linkpath) {
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

inline ptrdiff_t l_readlink(const char *path, char *buf, ptrdiff_t bufsiz) {
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

inline char *l_realpath(const char *path, char *resolved) {
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

inline int l_stat(const char *path, L_Stat *st) {
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

inline int l_fstat(L_FD fd, L_Stat *st) {
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

inline int l_opendir(const char *path, L_Dir *dir) {
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

inline void l_closedir(L_Dir *dir) {
    FindClose((HANDLE)dir->handle);
}

inline void *l_mmap(void *addr, size_t length, int prot, int flags, L_FD fd, long long offset)
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

inline int l_munmap(void *addr, size_t length)
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
inline L_FD l_open_read(const char* file) {
    return l_win_open_gen(file, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
}

inline L_FD l_open_write(const char* file) {
    return l_win_open_gen(file, GENERIC_WRITE, 0, CREATE_ALWAYS);
}

inline L_FD l_open_readwrite(const char* file) {
    return l_win_open_gen(file, GENERIC_READ | GENERIC_WRITE, 0, CREATE_ALWAYS);
}

inline L_FD l_open_append(const char* file) {
    return l_win_open_gen(file, FILE_APPEND_DATA, FILE_SHARE_READ, OPEN_ALWAYS);
}

inline L_FD l_open_trunc(const char* file) {
    return l_win_open_gen(file, GENERIC_WRITE, 0, CREATE_ALWAYS);
}

inline L_FD l_open(const char *path, int flags, mode_t mode) {
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
#endif

#ifdef __cplusplus
}
#endif

inline void l_exitif(bool condition, int code, char *message) {
    if(condition) {
        if(message) l_write(L_STDERR, message, l_strlen(message));
        l_exit(code);
    }
}

inline void puts(const char* s) {
  l_write(L_STDOUT, s, strlen(s));
}

// l_getenv: look up environment variable by name.
// On Unix: walks envp derived from argv (call l_getenv_init from main first).
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

#endif // L_OSH

// Provide non-inline linker symbols for memset/memcpy/memmove.
// Compilers (especially clang LTO on ARM) emit calls to these for
// large struct copies/zeroing even in freestanding mode.
#ifdef L_MAINFILE
#undef memset
#undef memcpy
#undef memmove
void *memset(void *dst, int b, size_t len) { return l_memset(dst, b, len); }
void *memcpy(void *dst, const void *src, size_t len) { return l_memcpy(dst, src, len); }
void *memmove(void *dst, const void *src, size_t len) { return l_memmove(dst, src, len); }
#ifndef L_DONTOVERRIDE
#define memset l_memset
#define memcpy l_memcpy
#define memmove l_memmove
#endif
#endif // L_MAINFILE
