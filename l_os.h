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

// This is not part of the C freestanding definition, but it is available on gcc, mingw & msvc
// For: mode_t, off_t, ssize_t. If not present on your system, define these types.
#include <sys/types.h>

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
#define   L_STDIN         0
#define   L_STDOUT        1
#define   L_STDERR        2

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

#ifdef __unix__
// Unix-only functions
/// Changes the current working directory
int l_chdir(const char *path);
/// Duplicates a file descriptor
int l_dup(L_FD fd);
/// Repositions the file offset of fd
off_t l_lseek(L_FD fd, off_t offset, int whence);
/// Creates a directory with the given permissions
int l_mkdir(const char *path, mode_t mode);
/// Yields the processor to other threads
int l_sched_yield(void);
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
#  define itoa l_itoa

#  define memmove l_memmove
#  define memset l_memset
#  define memcmp l_memcmp
#  define memcpy l_memcpy

#  define exit l_exit
#  define close l_close
#  define read l_read
#  define write l_write
#  define puts l_puts
#  define lseek l_lseek
#  define dup l_dup
#  define mkdir l_mkdir
#  define chdir l_chdir
#  define sched_yield l_sched_yield

#  define exitif l_exitif
#  define getenv l_getenv
#  define open_read l_open_read
#  define open_write l_open_write
#  define open_readwrite l_open_readwrite
#  define open_append l_open_append
#  define open_trunc l_open_trunc

#define STDIN    L_STDIN
#define STDOUT   L_STDOUT
#define STDERR   L_STDERR

#endif // L_DONT_OVERRIDE

inline size_t l_wcslen(const wchar_t *s) {
    size_t len = 0;
    while(s[len] != L'\0') ++len;
    return len;
}

inline size_t l_strlen(const char *str)
{
    size_t len;

    for (len = 0; str[len]; len++);
    return len;
}

inline void *l_memmove(void *dst, const void *src, size_t len)
{
    char *d = (char *)dst;
    const char *s = (const char *)src;

    if (d <= s) {
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
    char *p = dst;

    while (len--)
        *(p++) = b;
    return dst;
}

inline int l_memcmp(const void *s1, const void *s2, size_t n)
{
    size_t ofs = 0;
    int c1 = 0;

    while (ofs < n && !(c1 = ((unsigned char *)s1)[ofs] - ((unsigned char *)s2)[ofs])) {
        ofs++;
    }
    return c1;
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
    if (len > slen) return (0);
    const char *end = s1 + slen - len;
    while (*s1 && s1 <= end)
    {
        if (!l_memcmp(s1, s2, len))
            return (char *)s1;
        ++s1;
    }
    return (0);
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

    while (len--)
        *d++ = *s++;
    return dst;
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
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

noreturn inline void l_exit(int status)
{
    my_syscall1(__NR_exit, status & 255);
    while(1);
}

inline int l_chdir(const char *path)
{
    return my_syscall1(__NR_chdir, path);
}

inline int l_close(L_FD fd)
{
    return my_syscall1(__NR_close, fd);
}

inline int l_dup(L_FD fd)
{
    return my_syscall1(__NR_dup, fd);
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

inline L_FD l_open(const char *path, int flags, mode_t mode)
{
#ifdef __NR_openat
    return my_syscall4(__NR_openat, AT_FDCWD, path, flags, mode);
#elif defined(__NR_open)
    return my_syscall3(__NR_open, path, flags, mode);
#else
#error Neither __NR_openat nor __NR_open defined, cannot implement sys_open()
#endif
}

inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    return my_syscall3(__NR_read, fd, buf, count);
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
    return my_syscall3(__NR_write, fd, buf, count);
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

inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    DWORD written;
    HANDLE out;
    if(fd == L_STDOUT) {
      out = GetStdHandle(STD_OUTPUT_HANDLE);
    } else {
        out = (HANDLE)fd;
    }
    BOOL result = WriteFile(out, buf, count, &written, NULL);
    RETURN_CHECK(written);
}

inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    DWORD readden;
    HANDLE in;
    if(fd == L_STDIN) {
        in = GetStdHandle(STD_INPUT_HANDLE);
    } else {
        in = (HANDLE)fd;
    }
    BOOL result = ReadFile(in, buf, count, &readden, NULL);
    RETURN_CHECK(readden);
}


inline int l_close(L_FD fd) {
    BOOL result = CloseHandle((HANDLE)fd);
    if(result) return 0; else return -1;
}

// Terminal and timing support for Windows
inline void l_sleep_ms(unsigned int ms)
{
    Sleep(ms);
}

static DWORD l_saved_console_mode;

static inline unsigned long l_term_raw(void)
{
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(in, &l_saved_console_mode);
    DWORD raw = l_saved_console_mode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT);
    SetConsoleMode(in, raw);
    // Enable ANSI escape sequences on stdout
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD out_mode;
    GetConsoleMode(out, &out_mode);
    SetConsoleMode(out, out_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    return l_saved_console_mode;
}

static inline void l_term_restore(unsigned long old_mode)
{
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(in, (DWORD)old_mode);
}

inline ssize_t l_read_nonblock(L_FD fd, void *buf, size_t count)
{
    (void)fd; (void)count;
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    INPUT_RECORD ir;
    DWORD events;
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
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(out, &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *rows = 24; *cols = 80;
    }
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

    if(!result) return -1;

    HANDLE hFile = CreateFileW(buffer,        // file to open
                               desired,          // open for reading
                               shared,       // share for reading
                               NULL,                  // default security
                               dispo,         // existing file only
                               FILE_ATTRIBUTE_NORMAL, // normal file
                               NULL);                 // no attr. template

    if(hFile == INVALID_HANDLE_VALUE) return -1;
    else return (L_FD)hFile;
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

static char **l_envp;

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
