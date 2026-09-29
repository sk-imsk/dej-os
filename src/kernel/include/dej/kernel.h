#pragma once
// global macros and functions
// that are used throughout the kernel
//

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <stdatomic.h>

#define likely(x) __builtin_expect(!!(x), true)
#define unlikely(x) __builtin_expect(!!(x), false)
#define always_inline __attribute__((always_inline))
#define __unused __attribute__((unused))

#define KiB(x) (x) * 1024
#define KB(x) (x) * 1000
#define MB(x) KB(x) * 1000


/*errno */
#define EPERM		1	/* Operation not permitted */
#define ENOENT		2	/* No such file or directory */
#define ESRCH		3	/* No such process */
#define EINTR		4	/* Interrupted system call */
#define EIO		5	/* Input/output error */
#define ENXIO		6	/* Device not configured */
#define E2BIG		7	/* Argument list too long */
#define ENOEXEC		8	/* Exec format error */
#define EBADF		9	/* Bad file descriptor */
#define ECHILD		10	/* No child processes */
#define EDEADLK		11	/* Resource deadlock avoided */
				/* 11 was EAGAIN */
#define ENOMEM		12	/* Cannot allocate memory */
#define EACCES		13	/* Permission denied */
#define EFAULT		14	/* Bad address */
#define EBUSY		16	/* Device busy */
#define EEXIST		17	/* File exists */
#define EXDEV		18	/* Cross-device link */
#define ENODEV		19	/* Operation not supported by device */
#define ENOTDIR		20	/* Not a directory */
#define EISDIR		21	/* Is a directory */
#define EINVAL		22	/* Invalid argument */
#define ENFILE		23	/* Too many open files in system */
#define EMFILE		24	/* Too many open files */
#define ENOTTY		25	/* Inappropriate ioctl for device */
#define ETXTBSY		26	/* Text file busy */
#define EFBIG		27	/* File too large */
#define ENOSPC		28	/* No space left on device */
#define ESPIPE		29	/* Illegal seek */
#define EROFS		30	/* Read-only file system */
#define EMLINK		31	/* Too many links */
#define EPIPE		32	/* Broken pipe */

/* math software */
#define EDOM		33	/* Numerical argument out of domain */
#define ERANGE		34	/* Result too large */

/* non-blocking and interrupt i/o */
#define EAGAIN		35	/* Resource temporarily unavailable */
#define EWOULDBLOCK	EAGAIN	/* Operation would block */
#define EINPROGRESS	36	/* Operation now in progress */
#define EALREADY	37	/* Operation already in progress */
#define ETIMEDOUT	60	/* Operation timed out */
#define ECONNREFUSED	61	/* Connection refused */
#define ELOOP		62	/* Too many levels of symbolic links */
#define ENAMETOOLONG	63	/* File name too long */
#define EHOSTUNREACH	65	/* No route to host */
#define ENOTEMPTY	66	/* Directory not empty */
#define ENOLCK		77	/* No locks available */
#define ENOSYS		78	/* Function not implemented */
#define EILSEQ		84	/* Illegal byte sequence */
#define EOVERFLOW	87	/* Value too large to be stored in data type */
#define ECANCELED	88	/* Operation canceled */
#define EIDRM		89	/* Identifier removed */
#define ENOMSG		90	/* No message of desired type */
#define ENOTSUP		91	/* Not supported */
#define EBADMSG		92	/* Bad message */
#define ENOTRECOVERABLE	93	/* State not recoverable */
#define EOWNERDEAD	94	/* Previous owner died */
#define EPROTO		95	/* Protocol error */

typedef struct {
    _Atomic bool held;
    int holding_cpu;
} lock_t;

void aquire_lock(lock_t * lock);
void unlock_lock(lock_t * lock);
