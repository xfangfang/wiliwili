/**
 * wiliwili - PS5 native title libc additions.
 *
 * The bundled libraries were built against the payload SDK's FreeBSD libc, and
 * a title only sees the entry points the system modules export. Most of the
 * missing ones are supplied by locale_shims.c; this file holds the rest, which
 * are either absent from the console or named differently there.
 *
 * The SPIRIT of every entry here is the same: keep the call alive, fail softly,
 * and never let a feature the application does not use take the whole title
 * down. Anything that would silently return a wrong result (a regular
 * expression that never matches, a name resolution that never succeeds) is
 * marked with its consequence in the comment above it.
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <langinfo.h>
#include <limits.h>
#include <locale.h>
#include <net/if.h>
#include <pthread.h>
#include <pwd.h>
#include <regex.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <time.h>
#include <unistd.h>
#include <xlocale.h>

#ifndef LC_GLOBAL_LOCALE
#define LC_GLOBAL_LOCALE ((locale_t)-1)
#endif

/* mkstemps() lives in runtime_shims.c, the two flag taking variants are the
 * same function: this file system has no O_CLOEXEC to honour anyway. */
int mkstemps(char *template_name, int suffix_length);

int mkostemp(char *template_name, int flags)
{
    (void)flags;
    return mkstemps(template_name, 0);
}

int mkostemps(char *template_name, int suffix_length, int flags)
{
    (void)flags;
    return mkstemps(template_name, suffix_length);
}

/**
 * The console exports the signal mask aware entry points, the plain names are
 * the same routines in the FreeBSD libc the header comes from; keeping both
 * directions on one of the two pairs keeps them consistent.
 *
 * _setjmp() cannot be written as an ordinary wrapper: the address it saves has
 * to be the caller's, or the frame this function left behind would be popped a
 * second time when the jump lands. The call is a tail call and nothing of this
 * function survives it. _longjmp() needs no such care, the frame it discards is
 * the one that is supposed to go away.
 */
int _setjmp(jmp_buf environment)
{
    __attribute__((musttail)) return setjmp(environment);
}

__attribute__((noreturn)) void _longjmp(jmp_buf environment, int value)
{
    longjmp(environment, value);
}

/* Only reported through the ctype wide character paths, which the shims in
 * locale_shims.c answer for in ASCII as well. */
int ___mb_cur_max(void)
{
    return 1;
}

locale_t uselocale(locale_t locale)
{
    static int thread_locale;
    (void)locale;
    return (locale_t)&thread_locale;
}

char *nl_langinfo(nl_item item)
{
    switch (item) {
    case CODESET:
        return (char *)"UTF-8";
    default:
        return (char *)"";
    }
}

/**
 * OpenSSL asks for the file a code address belongs to when it looks for its
 * modules. A title has none, the failure is expected and handled by setting
 * the address based lookup aside.
 */
int dladdr(const void *address, Dl_info *info)
{
    (void)address;
    (void)info;
    return 0;
}

pid_t getpgrp(void)
{
    return 1;
}

/**
 * Nothing on the console has a password entry, getpwuid() is used by the
 * libraries to name a data directory they fall back from.
 */
int getpwuid_r(uid_t uid, struct passwd *password, char *buffer, size_t size,
               struct passwd **result)
{
    (void)uid;
    (void)password;
    (void)buffer;
    (void)size;
    if (result != NULL)
        *result = NULL;
    return 0;
}

/* The console offers the two conversions as system calls; going through them
 * in one direction covers gmtime(), and the broken down form is copied out
 * before another thread can reach it. */
static pthread_mutex_t wiliwili_time_lock = PTHREAD_MUTEX_INITIALIZER;

struct tm *gmtime_r(const time_t *timer, struct tm *result)
{
    pthread_mutex_lock(&wiliwili_time_lock);
    struct tm *shared = gmtime(timer);
    if (shared != NULL)
        *result = *shared;
    pthread_mutex_unlock(&wiliwili_time_lock);
    return shared != NULL ? result : NULL;
}

struct tm *localtime_r(const time_t *timer, struct tm *result)
{
    pthread_mutex_lock(&wiliwili_time_lock);
    struct tm *shared = localtime(timer);
    if (shared != NULL)
        *result = *shared;
    pthread_mutex_unlock(&wiliwili_time_lock);
    return shared != NULL ? result : NULL;
}

/**
 * mktime() reads the fields as local time and the console answers in the same
 * zone, so running it twice on the same instant tells how far the zone is from
 * UTC, and that is all timegm() has to take off.
 */
time_t timegm(struct tm *broken_down)
{
    struct tm local = *broken_down;
    local.tm_isdst = 0;
    const time_t guess = mktime(&local);
    if (guess == (time_t)-1)
        return (time_t)-1;

    struct tm utc;
    if (gmtime_r(&guess, &utc) == NULL)
        return (time_t)-1;
    utc.tm_isdst = 0;
    const time_t shifted = mktime(&utc);
    if (shifted == (time_t)-1)
        return (time_t)-1;

    return guess - (shifted - guess);
}

/* Only the streaming interfaces the application never opens reach this. */
unsigned int if_nametoindex(const char *name)
{
    (void)name;
    errno = ENXIO;
    return 0;
}

int pipe2(int descriptors[2], int flags)
{
    if (pipe(descriptors) != 0)
        return -1;

    if ((flags & O_NONBLOCK) != 0) {
        for (int i = 0; i < 2; ++i) {
            const int mode = fcntl(descriptors[i], F_GETFL, 0);
            if (mode >= 0)
                fcntl(descriptors[i], F_SETFL, mode | O_NONBLOCK);
        }
    }
    /* O_CLOEXEC is accepted and dropped, a title starts no other program. */
    return 0;
}

/* Datagram TLS only, and the console has neither call. */
ssize_t recvmmsg(int socket, struct mmsghdr *restrict messages, size_t count, int flags,
                 const struct timespec *restrict timeout)
{
    (void)socket;
    (void)messages;
    (void)count;
    (void)flags;
    (void)timeout;
    errno = ENOSYS;
    return -1;
}

ssize_t sendmmsg(int socket, struct mmsghdr *messages, size_t count, int flags)
{
    (void)socket;
    (void)messages;
    (void)count;
    (void)flags;
    errno = ENOSYS;
    return -1;
}

/**
 * The console has no regular expression implementation. mpv reaches these only
 * for its --sub-filter-regex options, which wiliwili never sets: a compile
 * failure is reported on the log and the option is left out.
 */
int regcomp(regex_t *restrict compiled, const char *restrict pattern, int flags)
{
    (void)compiled;
    (void)pattern;
    (void)flags;
    return REG_BADPAT;
}

int regexec(const regex_t *restrict compiled, const char *restrict string,
            size_t matches, regmatch_t *restrict groups, int flags)
{
    (void)compiled;
    (void)string;
    (void)matches;
    (void)groups;
    (void)flags;
    return REG_NOMATCH;
}

size_t regerror(int error, const regex_t *restrict compiled, char *restrict buffer,
                size_t size)
{
    (void)compiled;
    static const char *const messages[] = {
        "no error",
        "regular expression not supported by the console",
    };
    const char *message = (error == REG_NOMATCH) ? "no match" : messages[1];
    if (size == 0)
        return strlen(message) + 1;
    snprintf(buffer, size, "%s", message);
    return strlen(buffer) + 1;
}

void regfree(regex_t *compiled)
{
    (void)compiled;
}

/**
 * zstd reads its tracing hooks through a weak reference, which is left
 * unresolved on every normal build: the SDK archives define neither. A title
 * may not import a symbol that no system module exports, so the two pointers
 * are defined here, null, which is the state that turns the tracing off.
 */
void (*ZSTD_trace_decompress_begin)(const void *trace) = NULL;
void (*ZSTD_trace_decompress_end)(const void *trace) = NULL;

/**
 * The remaining POSIX calls that only libkernel_sys, the kernel variant the
 * system software links against, offers. A title has no business pulling that
 * module in: libkernel provides open(), unlink(), chmod() and the rest of the
 * file calls, and every caller of these wrappers in the application passes
 * AT_FDCWD, so the directory argument is only ever checked.
 *
 * readlink(), symlink() and link() are not here: the console's own POSIX layer,
 * libScePosixForWebKit, exports those as __wrap_readlink() and its siblings,
 * which is what the linker is told to route them to (see build_title.sh).
 */

/* not declared by the SDK headers, the kernel uses another name for it */
int openat(int directory, const char *path, int flags, ...);

int openat(int directory, const char *path, int flags, ...)
{
    va_list arguments;
    va_start(arguments, flags);
    const mode_t mode = (mode_t)va_arg(arguments, int);
    va_end(arguments);

    if (directory != AT_FDCWD && path != NULL && path[0] != '/') {
        errno = ENOTSUP;
        return -1;
    }
    return open(path, flags, mode);
}

int unlinkat(int directory, const char *path, int flags)
{
    if (directory != AT_FDCWD && path != NULL && path[0] != '/') {
        errno = ENOTSUP;
        return -1;
    }
    return (flags & AT_REMOVEDIR) != 0 ? rmdir(path) : unlink(path);
}

int fchmodat(int directory, const char *path, mode_t mode, int flags)
{
    /* AT_SYMLINK_NOFOLLOW asks for the link itself, and the console exposes no
     * symbolic links for a title to find. */
    (void)flags;
    if (directory != AT_FDCWD && path != NULL && path[0] != '/') {
        errno = ENOTSUP;
        return -1;
    }
    return chmod(path, mode);
}

long pathconf(const char *path, int name)
{
    (void)path;
    switch (name) {
    case _PC_LINK_MAX:
        return 1;
    case _PC_NAME_MAX:
        return 255;
    case _PC_PATH_MAX:
        return 1024;
    case _PC_CHOWN_RESTRICTED:
    case _PC_NO_TRUNC:
        return 1;
    case _PC_VDISABLE:
        return 0;
    default:
        errno = EINVAL;
        return -1;
    }
}

/* The loader starts the application in a session of its own. */
pid_t setsid(void)
{
    return getpid();
}

/* ---- fcntl on sockets ---------------------------------------------------------------------- */

/* In a sandboxed title the console's libc refuses fcntl on sockets with
 * EINVAL. curl then fails every transfer ("fcntl set CLOEXEC: Invalid
 * argument") and its sockets stay blocking. Linked with --wrap=fcntl, every
 * fcntl call in the application and its archives arrives here; only a call
 * the real libc refuses is treated as a socket request. */
extern int __real_fcntl(int descriptor, int command, ...);

/* The console's own non-blocking socket option. */
enum { SOCKET_OPTION_NONBLOCKING = 0x1200 };

static int socket_flags(int descriptor, int command, int value)
{
    switch (command)
    {
    case F_GETFD:
    case F_SETFD:
        return 0; /* a title never execs another program, no descriptor flags needed */
    case F_SETFL:
    {
        const int nonblocking = (value & O_NONBLOCK) != 0;
        return setsockopt(descriptor, SOL_SOCKET, SOCKET_OPTION_NONBLOCKING,
                          &nonblocking, sizeof(nonblocking));
    }
    case F_GETFL:
    {
        int nonblocking = 0;
        socklen_t length = sizeof(nonblocking);
        if (getsockopt(descriptor, SOL_SOCKET, SOCKET_OPTION_NONBLOCKING,
                       &nonblocking, &length) < 0)
            return -1;
        return O_RDWR | (nonblocking ? O_NONBLOCK : 0);
    }
    default:
        errno = EINVAL;
        return -1;
    }
}

int __wrap_fcntl(int descriptor, int command, ...)
{
    va_list arguments;
    va_start(arguments, command);
    /* Every command this application and its libraries use carries an int
     * or a pointer as the optional argument. */
    const intptr_t argument = va_arg(arguments, intptr_t);
    va_end(arguments);
    const int result = __real_fcntl(descriptor, command, argument);
    if (result >= 0 || errno != EINVAL)
        return result;
    if (command == F_GETFD || command == F_SETFD || command == F_SETFL ||
        command == F_GETFL)
        return socket_flags(descriptor, command, (int)argument);
    return result;
}


/* A title shares its address space with the libraries it runs, there is no
 * second copy of it to create. */
pid_t fork(void)
{
    errno = ENOSYS;
    return -1;
}

int fstatfs(int descriptor, struct statfs *buffer)
{
    (void)descriptor;
    (void)buffer;
    errno = ENOSYS;
    return -1;
}
