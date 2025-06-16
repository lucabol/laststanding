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

size_t l_wcslen(const wchar_t *s);
size_t l_strlen(const char *str);
char *l_strcpy(char *dst, const char *src);
char *l_strchr(const char *s, int c);
char *l_strrchr(const char *s, int c);
char *l_strstr(const char *s1, const char *s2);
int l_strncmp(const char *s1, const char *s2, size_t n);
void l_reverse(char str[], int length);

int l_isdigit(int c);
long l_atol(const char *s);
int l_atoi(const char *s);
char *l_itoa(int in, char* buffer, int radix);

void *l_memmove(void *dst, const void *src, size_t len);
void *l_memset(void *dst, int b, size_t len);
int l_memcmp(const void *s1, const void *s2, size_t n);
void *l_memcpy(void *dst, const void *src, size_t len);

// System interaction
noreturn void l_exit(int status);
L_FD l_open(const char *path, int flags, mode_t mode);
int l_close(L_FD fd);
ssize_t l_read(L_FD fd, void *buf, size_t count);
ssize_t l_write(L_FD fd, const void *buf, size_t count);
void l_puts(const char* s);
void l_exitif(bool condition, int code, char *message);

L_FD l_open_read(const char* file);
L_FD l_open_write(const char* file);
L_FD l_open_readwrite(const char* file);
L_FD l_open_append(const char* file);
L_FD l_open_trunc(const char* file);

#endif // L_WITHDEFS

#ifdef L_WITHSTART

#ifdef __unix__

/* startup code */
asm(".section .text\n"
    ".global _start\n"
    "_start:\n"
    "pop %rdi\n"                // argc   (first arg, %rdi)
    "mov %rsp, %rsi\n"          // argv[] (second arg, %rsi)
    "lea 8(%rsi,%rdi,8),%rdx\n" // then a NULL then envp (third arg, %rdx)
    "and $-16, %rsp\n"          // x86 ABI : esp must be 16-byte aligned when
    "sub $8, %rsp\n"            // entering the callee
    "call main\n"               // main() returns the status code, we'll exit with it.
    "movzb %al, %rdi\n"         // retrieve exit code from 8 lower bits
    "mov $60, %rax\n"           // NR_exit == 60
    "syscall\n"                 // really exit
    "hlt\n"                     // ensure it does not return
    "");

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

#  define isdigit l_isdigit
#  define atol l_atol
#  define atoi l_atoi
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
#  define execve l_execve
#  define fork l_fork
#  define mkdir l_mkdir
#  define chdir l_chdir
#  define sched_yield l_sched_yield

#  define exitif l_exitif
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
    size_t pos = (dst <= src) ? -1 : (long)len;
    void *ret = dst;

    while (len--) {
        pos += (dst <= src) ? 1 : -1;
        ((char *)dst)[pos] = ((char *)src)[pos];
    }
    return ret;
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
    char c1 = 0;

    while (ofs < n && !(c1 = ((char *)s1)[ofs] - ((char *)s2)[ofs])) {
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

inline char *l_strchr(const char *s, int c)
{
    while (*s) {
        if (*s == (char)c)
            return (char *)s;
        s++;
    }
    return NULL;
}

inline char *l_strrchr(const char *s, int c)
{
    const char *ret = NULL;

    while (*s) {
        if (*s == (char)c)
            ret = s;
        s++;
    }
    return (char *)ret;
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

inline char *l_strstr(const char *s1, const char *s2) {
    const size_t len = strlen (s2);
    while (*s1)
    {
        if (!l_memcmp (s1, s2, len))
            return (char *)s1;
        ++s1;
    }
    return (0);
}

inline int l_isdigit(int c)
{
    return (unsigned int)(c - '0') <= 9;
}

inline long l_atol(const char *s)
{
    unsigned long ret = 0;
    unsigned long d;
    int neg = 0;

    if (*s == '-') {
        neg = 1;
        s++;
    }

    while (1) {
        d = (*s++) - '0';
        if (d > 9)
            break;
        ret *= 10;
        ret += d;
    }

    return neg ? -ret : ret;
}

inline int l_atoi(const char *s)
{
    return l_atol(s);
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
    /*Handle 0 explicitly, 
      otherwise empty string is printed for 0 */
    if (num == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
    }
    else
    {
        // In library itoa function -ve numbers handled only with
        // base 10. SO here we are also following same concept
        if ((num < 0) && (base == 10))
        {
            isNegNum = 1;
            num = -num; // make num positive
        }
        // Process individual digits
        do
        {
            const int rem = (num % base);
            str[i++] = (rem > 9)? ((rem-10) + 'a') : (rem + '0');
            num = num/base;
        }
        while (num != 0);
        // If number is negative, append '-'
        if (isNegNum)
        {
            str[i++] = '-';
        }
        // Append string terminator
        str[i] = '\0';
        // Reverse the string
        l_reverse(str, i);
    }
    return str;
}

inline void *l_memcpy(void *dst, const void *src, size_t len)
{
    return l_memmove(dst, src, len);
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
         long _ret;                                                            \
         register long _num  asm("rax") = (num);                               \
         \
         asm volatile (                                                        \
                         "syscall\n"                                                   \
                         : "=a" (_ret)                                                 \
                         : "0"(_num)                                                   \
                         : "rcx", "r8", "r9", "r10", "r11", "memory", "cc"             \
                         )

#define my_syscall1(num, arg1)                                                \
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
                         )

#define my_syscall2(num, arg1, arg2)                                          \
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
                         )

#define my_syscall3(num, arg1, arg2, arg3)                                    \
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
                         )

#define my_syscall4(num, arg1, arg2, arg3, arg4)                              \
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
                         )

#define my_syscall5(num, arg1, arg2, arg3, arg4, arg5)                        \
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
                         )

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
#define O_DIRECTORY   0x10000

#else
#error "Just linux x86_64 and windows are supported. Paste relevant nolibc.h sections for more archs"
#endif // __x86_64__

noreturn inline void l_exit(int status)
{
    my_syscall1(__NR_exit, status & 255);
    while(1);
}

inline int l_chdir(const char *path)
{
    my_syscall1(__NR_chdir, path);
    return _ret;
}

inline int l_close(L_FD fd)
{
    my_syscall1(__NR_close, fd);
    return _ret;
}

inline int l_dup(L_FD fd)
{
    my_syscall1(__NR_dup, fd);
    return _ret;
}

inline off_t l_lseek(L_FD fd, off_t offset, int whence)
{
    my_syscall3(__NR_lseek, fd, offset, whence);
    return _ret;
}

inline int l_mkdir(const char *path, mode_t mode)
{
#ifdef __NR_mkdirat
    my_syscall3(__NR_mkdirat, AT_FDCWD, path, mode);
    return _ret;
#elif defined(__NR_mkdir)
    my_syscall2(__NR_mkdir, path, mode);
    return _ret;
#else
#error Neither __NR_mkdirat nor __NR_mkdir defined, cannot implement sys_mkdir()
#endif
}

inline L_FD l_open(const char *path, int flags, mode_t mode)
{
#ifdef __NR_openat
    my_syscall4(__NR_openat, AT_FDCWD, path, flags, mode);
    return _ret;
#elif defined(__NR_open)
    my_syscall3(__NR_open, path, flags, mode);
    return _ret;
#else
#error Neither __NR_openat nor __NR_open defined, cannot implement sys_open()
#endif
}

inline ssize_t l_read(L_FD fd, void *buf, size_t count)
{
    my_syscall3(__NR_read, fd, buf, count);
    return _ret;
}

inline int l_sched_yield(void)
{
    my_syscall0(__NR_sched_yield);
    return _ret;
}

inline ssize_t l_write(L_FD fd, const void *buf, size_t count)
{
    my_syscall3(__NR_write, fd, buf, count);
    return _ret;
}

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
    return l_win_open_gen(file, GENERIC_WRITE, 0, TRUNCATE_EXISTING | OPEN_ALWAYS);
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
#endif // L_OSH
