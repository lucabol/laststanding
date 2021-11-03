#include <float.h>
#include <iso646.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* STARTUP */
#if !defined(_WIN32)

#include <asm/unistd.h>
#include <asm/ioctls.h>
#include <asm/errno.h>
#include <linux/fs.h>
#include <linux/loop.h>
#include <linux/time.h>

/* this way it will be removed if unused */
static int errno;

#ifndef NOLIBC_IGNORE_ERRNO
#define SET_ERRNO(v) do { errno = (v); } while (0)
#else
#define SET_ERRNO(v) do { } while (0)
#endif

/* errno codes all ensure that they will not conflict with a valid pointer
 * because they all correspond to the highest addressable memory page.
 */
#define MAX_ERRNO 4095

typedef   signed long       ssize_t;

/* for stat() */
typedef unsigned int          dev_t;
typedef unsigned long         ino_t;
typedef unsigned int         mode_t;
typedef   signed int          pid_t;
typedef unsigned int          uid_t;
typedef unsigned int          gid_t;
typedef unsigned long       nlink_t;
typedef   signed long         off_t;
typedef   signed long     blksize_t;
typedef   signed long      blkcnt_t;
typedef   signed long        time_t;

/* for poll() */
struct pollfd {
	int fd;
	short int events;
	short int revents;
};

/* for getdents64() */
struct linux_dirent64 {
	uint64_t       d_ino;
	int64_t        d_off;
	unsigned short d_reclen;
	unsigned char  d_type;
	char           d_name[];
};

/* commonly an fd_set represents 256 FDs */
#define FD_SETSIZE 256
typedef struct { uint32_t fd32[FD_SETSIZE/32]; } fd_set;

/* needed by wait4() */
struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
	long   ru_maxrss;
	long   ru_ixrss;
	long   ru_idrss;
	long   ru_isrss;
	long   ru_minflt;
	long   ru_majflt;
	long   ru_nswap;
	long   ru_inblock;
	long   ru_oublock;
	long   ru_msgsnd;
	long   ru_msgrcv;
	long   ru_nsignals;
	long   ru_nvcsw;
	long   ru_nivcsw;
};

/* stat flags (WARNING, octal here) */
#define S_IFDIR       0040000
#define S_IFCHR       0020000
#define S_IFBLK       0060000
#define S_IFREG       0100000
#define S_IFIFO       0010000
#define S_IFLNK       0120000
#define S_IFSOCK      0140000
#define S_IFMT        0170000

#define S_ISDIR(mode)  (((mode) & S_IFDIR) == S_IFDIR)
#define S_ISCHR(mode)  (((mode) & S_IFCHR) == S_IFCHR)
#define S_ISBLK(mode)  (((mode) & S_IFBLK) == S_IFBLK)
#define S_ISREG(mode)  (((mode) & S_IFREG) == S_IFREG)
#define S_ISFIFO(mode) (((mode) & S_IFIFO) == S_IFIFO)
#define S_ISLNK(mode)  (((mode) & S_IFLNK) == S_IFLNK)
#define S_ISSOCK(mode) (((mode) & S_IFSOCK) == S_IFSOCK)

#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK    10
#define DT_SOCK   12

/* all the *at functions */
#ifndef AT_FDCWD
#define AT_FDCWD             -100
#endif

/* lseek */
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2

/* reboot */
#define LINUX_REBOOT_MAGIC1         0xfee1dead
#define LINUX_REBOOT_MAGIC2         0x28121969
#define LINUX_REBOOT_CMD_HALT       0xcdef0123
#define LINUX_REBOOT_CMD_POWER_OFF  0x4321fedc
#define LINUX_REBOOT_CMD_RESTART    0x01234567
#define LINUX_REBOOT_CMD_SW_SUSPEND 0xd000fce2


/* The format of the struct as returned by the libc to the application, which
 * significantly differs from the format returned by the stat() syscall flavours.
 */
struct stat {
	dev_t     st_dev;     /* ID of device containing file */
	ino_t     st_ino;     /* inode number */
	mode_t    st_mode;    /* protection */
	nlink_t   st_nlink;   /* number of hard links */
	uid_t     st_uid;     /* user ID of owner */
	gid_t     st_gid;     /* group ID of owner */
	dev_t     st_rdev;    /* device ID (if special file) */
	off_t     st_size;    /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
	time_t    st_atime;   /* time of last access */
	time_t    st_mtime;   /* time of last modification */
	time_t    st_ctime;   /* time of last status change */
};

#define WEXITSTATUS(status)   (((status) & 0xff00) >> 8)
#define WIFEXITED(status)     (((status) & 0x7f) == 0)

/* for SIGCHLD */
#include <asm/signal.h>

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
	);                                                                    \
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
	);                                                                    \
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
	);                                                                    \
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
	);                                                                    \
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
	);                                                                    \
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
	);                                                                    \
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

/* fcntl / open */
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

/* The struct returned by the stat() syscall, equivalent to stat64(). The
 * syscall returns 116 bytes and stops in the middle of __unused.
 */
struct sys_stat_struct {
	unsigned long st_dev;
	unsigned long st_ino;
	unsigned long st_nlink;
	unsigned int  st_mode;
	unsigned int  st_uid;

	unsigned int  st_gid;
	unsigned int  __pad0;
	unsigned long st_rdev;
	long          st_size;
	long          st_blksize;

	long          st_blocks;
	unsigned long st_atime;
	unsigned long st_atime_nsec;
	unsigned long st_mtime;

	unsigned long st_mtime_nsec;
	unsigned long st_ctime;
	unsigned long st_ctime_nsec;
	long          __unused[3];
};

#else
#error "Just linux x86_64 and windows are supported. Paste relevant nolibc.h sections for more archs"
#endif

/* Below are the C functions used to declare the raw syscalls. They try to be
 * architecture-agnostic, and return either a success or -errno. Declaring them
 * static will lead to them being inlined in most cases, but it's still possible
 * to reference them by a pointer if needed.
 */
static __attribute__((unused))
void *sys_brk(void *addr)
{
	return (void *)my_syscall1(__NR_brk, addr);
}

static __attribute__((noreturn,unused))
void sys_exit(int status)
{
	my_syscall1(__NR_exit, status & 255);
	while(1); // shut the "noreturn" warnings.
}

static __attribute__((unused))
int sys_chdir(const char *path)
{
	return my_syscall1(__NR_chdir, path);
}

static __attribute__((unused))
int sys_chmod(const char *path, mode_t mode)
{
#ifdef __NR_fchmodat
	return my_syscall4(__NR_fchmodat, AT_FDCWD, path, mode, 0);
#elif defined(__NR_chmod)
	return my_syscall2(__NR_chmod, path, mode);
#else
#error Neither __NR_fchmodat nor __NR_chmod defined, cannot implement sys_chmod()
#endif
}

static __attribute__((unused))
int sys_chown(const char *path, uid_t owner, gid_t group)
{
#ifdef __NR_fchownat
	return my_syscall5(__NR_fchownat, AT_FDCWD, path, owner, group, 0);
#elif defined(__NR_chown)
	return my_syscall3(__NR_chown, path, owner, group);
#else
#error Neither __NR_fchownat nor __NR_chown defined, cannot implement sys_chown()
#endif
}

static __attribute__((unused))
int sys_chroot(const char *path)
{
	return my_syscall1(__NR_chroot, path);
}

static __attribute__((unused))
int sys_close(int fd)
{
	return my_syscall1(__NR_close, fd);
}

static __attribute__((unused))
int sys_dup(int fd)
{
	return my_syscall1(__NR_dup, fd);
}

#ifdef __NR_dup3
static __attribute__((unused))
int sys_dup3(int old, int new, int flags)
{
	return my_syscall3(__NR_dup3, old, new, flags);
}
#endif

static __attribute__((unused))
int sys_dup2(int old, int new)
{
#ifdef __NR_dup3
	return my_syscall3(__NR_dup3, old, new, 0);
#elif defined(__NR_dup2)
	return my_syscall2(__NR_dup2, old, new);
#else
#error Neither __NR_dup3 nor __NR_dup2 defined, cannot implement sys_dup2()
#endif
}

static __attribute__((unused))
int sys_execve(const char *filename, char *const argv[], char *const envp[])
{
	return my_syscall3(__NR_execve, filename, argv, envp);
}

static __attribute__((unused))
pid_t sys_fork(void)
{
#ifdef __NR_clone
	/* note: some archs only have clone() and not fork(). Different archs
	 * have a different API, but most archs have the flags on first arg and
	 * will not use the rest with no other flag.
	 */
	return my_syscall5(__NR_clone, SIGCHLD, 0, 0, 0, 0);
#elif defined(__NR_fork)
	return my_syscall0(__NR_fork);
#else
#error Neither __NR_clone nor __NR_fork defined, cannot implement sys_fork()
#endif
}

static __attribute__((unused))
int sys_fsync(int fd)
{
	return my_syscall1(__NR_fsync, fd);
}

static __attribute__((unused))
int sys_getdents64(int fd, struct linux_dirent64 *dirp, int count)
{
	return my_syscall3(__NR_getdents64, fd, dirp, count);
}

static __attribute__((unused))
pid_t sys_getpgid(pid_t pid)
{
	return my_syscall1(__NR_getpgid, pid);
}

static __attribute__((unused))
pid_t sys_getpgrp(void)
{
	return sys_getpgid(0);
}

static __attribute__((unused))
pid_t sys_getpid(void)
{
	return my_syscall0(__NR_getpid);
}

static __attribute__((unused))
int sys_gettimeofday(struct timeval *tv, struct timezone *tz)
{
	return my_syscall2(__NR_gettimeofday, tv, tz);
}

static __attribute__((unused))
int sys_ioctl(int fd, unsigned long req, void *value)
{
	return my_syscall3(__NR_ioctl, fd, req, value);
}

static __attribute__((unused))
int sys_kill(pid_t pid, int signal)
{
	return my_syscall2(__NR_kill, pid, signal);
}

static __attribute__((unused))
int sys_link(const char *old, const char *new)
{
#ifdef __NR_linkat
	return my_syscall5(__NR_linkat, AT_FDCWD, old, AT_FDCWD, new, 0);
#elif defined(__NR_link)
	return my_syscall2(__NR_link, old, new);
#else
#error Neither __NR_linkat nor __NR_link defined, cannot implement sys_link()
#endif
}

static __attribute__((unused))
off_t sys_lseek(int fd, off_t offset, int whence)
{
	return my_syscall3(__NR_lseek, fd, offset, whence);
}

static __attribute__((unused))
int sys_mkdir(const char *path, mode_t mode)
{
#ifdef __NR_mkdirat
	return my_syscall3(__NR_mkdirat, AT_FDCWD, path, mode);
#elif defined(__NR_mkdir)
	return my_syscall2(__NR_mkdir, path, mode);
#else
#error Neither __NR_mkdirat nor __NR_mkdir defined, cannot implement sys_mkdir()
#endif
}

static __attribute__((unused))
long sys_mknod(const char *path, mode_t mode, dev_t dev)
{
#ifdef __NR_mknodat
	return my_syscall4(__NR_mknodat, AT_FDCWD, path, mode, dev);
#elif defined(__NR_mknod)
	return my_syscall3(__NR_mknod, path, mode, dev);
#else
#error Neither __NR_mknodat nor __NR_mknod defined, cannot implement sys_mknod()
#endif
}

static __attribute__((unused))
int sys_mount(const char *src, const char *tgt, const char *fst,
	      unsigned long flags, const void *data)
{
	return my_syscall5(__NR_mount, src, tgt, fst, flags, data);
}

static __attribute__((unused))
int sys_open(const char *path, int flags, mode_t mode)
{
#ifdef __NR_openat
	return my_syscall4(__NR_openat, AT_FDCWD, path, flags, mode);
#elif defined(__NR_open)
	return my_syscall3(__NR_open, path, flags, mode);
#else
#error Neither __NR_openat nor __NR_open defined, cannot implement sys_open()
#endif
}

static __attribute__((unused))
int sys_pivot_root(const char *new, const char *old)
{
	return my_syscall2(__NR_pivot_root, new, old);
}

static __attribute__((unused))
int sys_poll(struct pollfd *fds, int nfds, int timeout)
{
#if defined(__NR_ppoll)
	struct timespec t;

	if (timeout >= 0) {
		t.tv_sec  = timeout / 1000;
		t.tv_nsec = (timeout % 1000) * 1000000;
	}
	return my_syscall4(__NR_ppoll, fds, nfds, (timeout >= 0) ? &t : NULL, NULL);
#elif defined(__NR_poll)
	return my_syscall3(__NR_poll, fds, nfds, timeout);
#else
#error Neither __NR_ppoll nor __NR_poll defined, cannot implement sys_poll()
#endif
}

static __attribute__((unused))
ssize_t sys_read(int fd, void *buf, size_t count)
{
	return my_syscall3(__NR_read, fd, buf, count);
}

static __attribute__((unused))
ssize_t sys_reboot(int magic1, int magic2, int cmd, void *arg)
{
	return my_syscall4(__NR_reboot, magic1, magic2, cmd, arg);
}

static __attribute__((unused))
int sys_sched_yield(void)
{
	return my_syscall0(__NR_sched_yield);
}

static __attribute__((unused))
int sys_select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
#if defined(__ARCH_WANT_SYS_OLD_SELECT) && !defined(__NR__newselect)
	struct sel_arg_struct {
		unsigned long n;
		fd_set *r, *w, *e;
		struct timeval *t;
	} arg = { .n = nfds, .r = rfds, .w = wfds, .e = efds, .t = timeout };
	return my_syscall1(__NR_select, &arg);
#elif defined(__ARCH_WANT_SYS_PSELECT6) && defined(__NR_pselect6)
	struct timespec t;

	if (timeout) {
		t.tv_sec  = timeout->tv_sec;
		t.tv_nsec = timeout->tv_usec * 1000;
	}
	return my_syscall6(__NR_pselect6, nfds, rfds, wfds, efds, timeout ? &t : NULL, NULL);
#elif defined(__NR__newselect) || defined(__NR_select)
#ifndef __NR__newselect
#define __NR__newselect __NR_select
#endif
	return my_syscall5(__NR__newselect, nfds, rfds, wfds, efds, timeout);
#else
#error None of __NR_select, __NR_pselect6, nor __NR__newselect defined, cannot implement sys_select()
#endif
}

static __attribute__((unused))
int sys_setpgid(pid_t pid, pid_t pgid)
{
	return my_syscall2(__NR_setpgid, pid, pgid);
}

static __attribute__((unused))
pid_t sys_setsid(void)
{
	return my_syscall0(__NR_setsid);
}

static __attribute__((unused))
int sys_stat(const char *path, struct stat *buf)
{
	struct sys_stat_struct stat;
	long ret;

#ifdef __NR_newfstatat
	/* only solution for arm64 */
	ret = my_syscall4(__NR_newfstatat, AT_FDCWD, path, &stat, 0);
#elif defined(__NR_stat)
	ret = my_syscall2(__NR_stat, path, &stat);
#else
#error Neither __NR_newfstatat nor __NR_stat defined, cannot implement sys_stat()
#endif
	buf->st_dev     = stat.st_dev;
	buf->st_ino     = stat.st_ino;
	buf->st_mode    = stat.st_mode;
	buf->st_nlink   = stat.st_nlink;
	buf->st_uid     = stat.st_uid;
	buf->st_gid     = stat.st_gid;
	buf->st_rdev    = stat.st_rdev;
	buf->st_size    = stat.st_size;
	buf->st_blksize = stat.st_blksize;
	buf->st_blocks  = stat.st_blocks;
	buf->st_atime   = stat.st_atime;
	buf->st_mtime   = stat.st_mtime;
	buf->st_ctime   = stat.st_ctime;
	return ret;
}


static __attribute__((unused))
int sys_symlink(const char *old, const char *new)
{
#ifdef __NR_symlinkat
	return my_syscall3(__NR_symlinkat, old, AT_FDCWD, new);
#elif defined(__NR_symlink)
	return my_syscall2(__NR_symlink, old, new);
#else
#error Neither __NR_symlinkat nor __NR_symlink defined, cannot implement sys_symlink()
#endif
}

static __attribute__((unused))
mode_t sys_umask(mode_t mode)
{
	return my_syscall1(__NR_umask, mode);
}

static __attribute__((unused))
int sys_umount2(const char *path, int flags)
{
	return my_syscall2(__NR_umount2, path, flags);
}

static __attribute__((unused))
int sys_unlink(const char *path)
{
#ifdef __NR_unlinkat
	return my_syscall3(__NR_unlinkat, AT_FDCWD, path, 0);
#elif defined(__NR_unlink)
	return my_syscall1(__NR_unlink, path);
#else
#error Neither __NR_unlinkat nor __NR_unlink defined, cannot implement sys_unlink()
#endif
}

static __attribute__((unused))
pid_t sys_wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	return my_syscall4(__NR_wait4, pid, status, options, rusage);
}

static __attribute__((unused))
pid_t sys_waitpid(pid_t pid, int *status, int options)
{
	return sys_wait4(pid, status, options, 0);
}

static __attribute__((unused))
pid_t sys_wait(int *status)
{
	return sys_waitpid(-1, status, 0);
}

static __attribute__((unused))
ssize_t sys_write(int fd, const void *buf, size_t count)
{
	return my_syscall3(__NR_write, fd, buf, count);
}


/* Below are the libc-compatible syscalls which return x or -1 and set errno.
 * They rely on the functions above. Similarly they're marked static so that it
 * is possible to assign pointers to them if needed.
 */

static __attribute__((unused))
int brk(void *addr)
{
	void *ret = sys_brk(addr);

	if (!ret) {
		SET_ERRNO(ENOMEM);
		return -1;
	}
	return 0;
}

static __attribute__((noreturn,unused))
void exit(int status)
{
	sys_exit(status);
}

static __attribute__((unused))
int chdir(const char *path)
{
	int ret = sys_chdir(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int chmod(const char *path, mode_t mode)
{
	int ret = sys_chmod(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int chown(const char *path, uid_t owner, gid_t group)
{
	int ret = sys_chown(path, owner, group);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int chroot(const char *path)
{
	int ret = sys_chroot(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int close(int fd)
{
	int ret = sys_close(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int dup(int fd)
{
	int ret = sys_dup(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int dup2(int old, int new)
{
	int ret = sys_dup2(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

#ifdef __NR_dup3
static __attribute__((unused))
int dup3(int old, int new, int flags)
{
	int ret = sys_dup3(old, new, flags);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}
#endif

static __attribute__((unused))
int execve(const char *filename, char *const argv[], char *const envp[])
{
	int ret = sys_execve(filename, argv, envp);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t fork(void)
{
	pid_t ret = sys_fork();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int fsync(int fd)
{
	int ret = sys_fsync(fd);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int getdents64(int fd, struct linux_dirent64 *dirp, int count)
{
	int ret = sys_getdents64(fd, dirp, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t getpgid(pid_t pid)
{
	pid_t ret = sys_getpgid(pid);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t getpgrp(void)
{
	pid_t ret = sys_getpgrp();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t getpid(void)
{
	pid_t ret = sys_getpid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	int ret = sys_gettimeofday(tv, tz);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int ioctl(int fd, unsigned long req, void *value)
{
	int ret = sys_ioctl(fd, req, value);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int kill(pid_t pid, int signal)
{
	int ret = sys_kill(pid, signal);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int link(const char *old, const char *new)
{
	int ret = sys_link(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
off_t lseek(int fd, off_t offset, int whence)
{
	off_t ret = sys_lseek(fd, offset, whence);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int mkdir(const char *path, mode_t mode)
{
	int ret = sys_mkdir(path, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int mknod(const char *path, mode_t mode, dev_t dev)
{
	int ret = sys_mknod(path, mode, dev);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int mount(const char *src, const char *tgt,
	  const char *fst, unsigned long flags,
	  const void *data)
{
	int ret = sys_mount(src, tgt, fst, flags, data);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int open(const char *path, int flags, mode_t mode)
{
	int ret = sys_open(path, flags, mode);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int pivot_root(const char *new, const char *old)
{
	int ret = sys_pivot_root(new, old);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int poll(struct pollfd *fds, int nfds, int timeout)
{
	int ret = sys_poll(fds, nfds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
ssize_t read(int fd, void *buf, size_t count)
{
	ssize_t ret = sys_read(fd, buf, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int reboot(int cmd)
{
	int ret = sys_reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, cmd, 0);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
void *sbrk(intptr_t inc)
{
	void *ret;

	/* first call to find current end */
	if ((ret = sys_brk(0)) && (sys_brk(ret + inc) == ret + inc))
		return ret + inc;

	SET_ERRNO(ENOMEM);
	return (void *)-1;
}

static __attribute__((unused))
int sched_yield(void)
{
	int ret = sys_sched_yield();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, struct timeval *timeout)
{
	int ret = sys_select(nfds, rfds, wfds, efds, timeout);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int setpgid(pid_t pid, pid_t pgid)
{
	int ret = sys_setpgid(pid, pgid);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t setsid(void)
{
	pid_t ret = sys_setsid();

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
unsigned int sleep(unsigned int seconds)
{
	struct timeval my_timeval = { seconds, 0 };

	if (sys_select(0, 0, 0, 0, &my_timeval) < 0)
		return my_timeval.tv_sec + !!my_timeval.tv_usec;
	else
		return 0;
}

static __attribute__((unused))
int msleep(unsigned int msecs)
{
	struct timeval my_timeval = { msecs / 1000, (msecs % 1000) * 1000 };

	if (sys_select(0, 0, 0, 0, &my_timeval) < 0)
		return (my_timeval.tv_sec * 1000) +
			(my_timeval.tv_usec / 1000) +
			!!(my_timeval.tv_usec % 1000);
	else
		return 0;
}

static __attribute__((unused))
int stat(const char *path, struct stat *buf)
{
	int ret = sys_stat(path, buf);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int symlink(const char *old, const char *new)
{
	int ret = sys_symlink(old, new);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int tcsetpgrp(int fd, pid_t pid)
{
	return ioctl(fd, TIOCSPGRP, &pid);
}

static __attribute__((unused))
mode_t umask(mode_t mode)
{
	return sys_umask(mode);
}

static __attribute__((unused))
int umount2(const char *path, int flags)
{
	int ret = sys_umount2(path, flags);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
int unlink(const char *path)
{
	int ret = sys_unlink(path);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t wait4(pid_t pid, int *status, int options, struct rusage *rusage)
{
	pid_t ret = sys_wait4(pid, status, options, rusage);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t waitpid(pid_t pid, int *status, int options)
{
	pid_t ret = sys_waitpid(pid, status, options);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
pid_t wait(int *status)
{
	pid_t ret = sys_wait(status);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

static __attribute__((unused))
ssize_t write(int fd, const void *buf, size_t count)
{
	ssize_t ret = sys_write(fd, buf, count);

	if (ret < 0) {
		SET_ERRNO(-ret);
		ret = -1;
	}
	return ret;
}

/* some size-optimized reimplementations of a few common str* and mem*
 * functions. They're marked static, except memcpy() and raise() which are used
 * by libgcc on ARM, so they are marked weak instead in order not to cause an
 * error when building a program made of multiple files (not recommended).
 */

static __attribute__((unused))
void *memmove(void *dst, const void *src, size_t len)
{
	ssize_t pos = (dst <= src) ? -1 : (long)len;
	void *ret = dst;

	while (len--) {
		pos += (dst <= src) ? 1 : -1;
		((char *)dst)[pos] = ((char *)src)[pos];
	}
	return ret;
}

static __attribute__((unused))
void *memset(void *dst, int b, size_t len)
{
	char *p = dst;

	while (len--)
		*(p++) = b;
	return dst;
}

static __attribute__((unused))
int memcmp(const void *s1, const void *s2, size_t n)
{
	size_t ofs = 0;
	char c1 = 0;

	while (ofs < n && !(c1 = ((char *)s1)[ofs] - ((char *)s2)[ofs])) {
		ofs++;
	}
	return c1;
}

static __attribute__((unused))
char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++));
	return ret;
}

static __attribute__((unused))
char *strchr(const char *s, int c)
{
	while (*s) {
		if (*s == (char)c)
			return (char *)s;
		s++;
	}
	return NULL;
}

static __attribute__((unused))
char *strrchr(const char *s, int c)
{
	const char *ret = NULL;

	while (*s) {
		if (*s == (char)c)
			ret = s;
		s++;
	}
	return (char *)ret;
}

static __attribute__((unused))
int isdigit(int c)
{
	return (unsigned int)(c - '0') <= 9;
}

static __attribute__((unused))
long atol(const char *s)
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

static __attribute__((unused))
int atoi(const char *s)
{
	return atol(s);
}

static __attribute__((unused))
const char *ltoa(long in)
{
	/* large enough for -9223372036854775808 */
	static char buffer[21];
	char       *pos = buffer + sizeof(buffer) - 1;
	int         neg = in < 0;
	unsigned long n = neg ? -in : in;

	*pos-- = '\0';
	do {
		*pos-- = '0' + n % 10;
		n /= 10;
		if (pos < buffer)
			return pos + 1;
	} while (n);

	if (neg)
		*pos-- = '-';
	return pos + 1;
}

__attribute__((weak,unused))
void *memcpy(void *dst, const void *src, size_t len)
{
	return memmove(dst, src, len);
}

/* needed by libgcc for divide by zero */
__attribute__((weak,unused))
int raise(int signal)
{
	return kill(getpid(), signal);
}

/* Here come a few helper functions */

static __attribute__((unused))
void FD_ZERO(fd_set *set)
{
	memset(set, 0, sizeof(*set));
}

static __attribute__((unused))
void FD_SET(int fd, fd_set *set)
{
	if (fd < 0 || fd >= FD_SETSIZE)
		return;
	set->fd32[fd / 32] |= 1 << (fd & 31);
}

/* WARNING, it only deals with the 4096 first majors and 256 first minors */
static __attribute__((unused))
dev_t makedev(unsigned int major, unsigned int minor)
{
	return ((major & 0xfff) << 8) | (minor & 0xff);
}

#else
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shellapi.h>

// Windows startup code here
void __main() {}

int main(int argc, char* argv[]);

LPTSTR * parseCommandLine(LPTSTR szCmdLine, int * argc)
{
    int arg_count = 0;
    int char_count = 0;
    TCHAR break_char = ' ';
    TCHAR * c;
    LPTSTR * ret;
    LPTSTR ret_str;

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

    ret = LocalAlloc( LMEM_FIXED, (arg_count * sizeof(LPTSTR)) + (char_count + arg_count) * sizeof(TCHAR));

    ret_str = (LPTSTR)(ret + arg_count);

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

int WINAPI
mainCRTStartup(void)
{
   LPTSTR *szArglist;
   int nArgs;
   int i;

   szArglist = parseCommandLine(GetCommandLine(), &nArgs);
   if( NULL == szArglist )
   {
      return 0;
   }
   else {
	   i = main(nArgs, szArglist);
   }

   LocalFree(szArglist);

   return(i);	
}

static __attribute__((unused))
int write(int fd, const void *buf, size_t count)
{
	DWORD written;
	WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, strlen(buf), &written, NULL);
	return written;
}
#endif

size_t strlen(const char *str)
{
	size_t len;

	for (len = 0; str[len]; len++);
	return len;
}

#define strlen(str) ({                          \
	__builtin_constant_p((str)) ?           \
		__builtin_strlen((str)) :       \
		strlen((str));           \
})
