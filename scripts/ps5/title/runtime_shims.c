/**
 * wiliwili - PS5 native title runtime shims.
 *
 * Adapted from ps5-opengl's native-app/runtime_shims.c and
 * ps5-native-app-boilerplate's ProsperoLight component
 * (Copyright (C) 2026 BlackBearReloaded, GPL-3.0-or-later).
 *
 * A native title links one static image: there is no dynamic loader that would
 * satisfy the libc entry points the bundled libraries expect, and returning from
 * main terminates the process instead of handing control back to a shell.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

extern int sceKernelUsleep(uint32_t microseconds);
extern uint64_t sceKernelGetProcessTime(void);

/* The console keeps its own splash picture up until the title draws something,
 * and the OpenGL runtime hides it as soon as the display opens. The screen stays
 * black until the fonts and the first page are ready; holding the splash longer
 * needs a call back into the launcher, which is not wired up yet. */

/**
 * Returning from main or calling exit() ends the process, and the console
 * reports the crash over the title. Keep the application suspended instead so
 * the log stays readable after a failure.
 */
__attribute__((noreturn)) void catchReturnFromMain(int status)
{
    printf("[wiliwili] main returned status=%d\n", status);
    fflush(NULL);
    for (;;)
        sceKernelUsleep(100000);
}

/**
 * A title's standard streams have no reader, so the whole application log is
 * moved to the directory wiliwili stores its configuration in. /download0 is the
 * title's private data area next to /app0 and is used if that directory cannot
 * be created.
 */
/**
 * The log lives on the console's storage and nothing ever deletes it: every
 * launch appends to the same file. Keep it bounded by moving the file aside
 * when it grows past this size, so the directory holds one recent log and one
 * previous one instead of a file that grows with every session.
 */
#define WILIWILI_LOG_ROTATE_BYTES (8 * 1024 * 1024)

/**
 * Where the console's own log would be read back.
 *
 * Kept at -1: a title cannot duplicate its console handle (the console answers
 * that call as an illegal function call and stops the process), and the kernel
 * log of this console cannot be read either way.
 */
int wiliwili_console_log = -1;

// Defined below; the log's own steps are marked so a session that stops inside
// them says which call it reached.
void wiliwili_mark(const char *stage);

static FILE *wiliwili_log_open(const char *directory)
{
    wiliwili_mark("log: mkdir");
    if (mkdir(directory, 0755) != 0 && errno != EEXIST)
        return NULL;
    wiliwili_mark("log: mkdir done");
    char path[256];
    if ((size_t)snprintf(path, sizeof(path), "%s/wiliwili-title.log", directory) >= sizeof(path))
        return NULL;
    {
        // An earlier diagnostic build wrote a second log next to this one.
        // Nothing writes it any more, and the console offers no way to delete
        // it from outside, so it only holds space on the card: drop it the next
        // time the title runs.
        char stale[256];
        if ((size_t)snprintf(stale, sizeof(stale), "%s/logprobe.log", directory) < sizeof(stale))
            (void)unlink(stale);
    }
    wiliwili_mark("log: unlink done");
    {
        struct stat info;
        if (stat(path, &info) == 0 && info.st_size >= WILIWILI_LOG_ROTATE_BYTES) {
            char previous[272];
            if ((size_t)snprintf(previous, sizeof(previous), "%s.1", path) < sizeof(previous))
                (void)rename(path, previous);  // replaces the older copy
        }
    }
    wiliwili_mark("log: stat done");
    // Append-only writes on both streams keep the file free of two-writer
    // offset races: every write lands at end-of-file, so lines never clobber
    // each other.
    FILE *stream = freopen(path, "a", stdout);
    wiliwili_mark(stream == NULL ? "log: stdout failed" : "log: stdout redirected");
    if (stream == NULL)
        return NULL;
    setvbuf(stream, NULL, _IOLBF, 0);
    if (freopen(path, "a", stderr) != NULL)
        setvbuf(stderr, NULL, _IOLBF, 0);
    wiliwili_mark("log: stderr redirected");
    return stream;
}

/**
 * Says how far the start-up got.
 *
 * A title that the system stops before it writes anything leaves no log at
 * all: the log file is only reached once the application prints its first
 * line. This writes to a file of its own as soon as the first stage is set, so
 * a session that ends in the first moments still says which step it reached.
 */
char wiliwili_last_stage[64] = "boot";

void wiliwili_mark(const char *stage)
{
    if (stage && *stage)
        snprintf(wiliwili_last_stage, sizeof(wiliwili_last_stage), "%s", stage);

    static int handle = -2;  // -2: not opened yet, -1: nowhere to write
    static struct timespec first;
    if (handle == -2) {
        clock_gettime(CLOCK_MONOTONIC, &first);
        handle = open("/data/wiliwili/startup.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (handle < 0)
            handle = open("/download0/startup.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (handle < 0)
            handle = -1;
    }
    if (handle < 0 || stage == NULL)
        return;
    // The elapsed time separates "stopped at this call" from "the system
    // stopped the title while it was here".
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    const long ms = (now.tv_sec - first.tv_sec) * 1000 + (now.tv_nsec - first.tv_nsec) / 1000000;
    char line[192];
    const int length = snprintf(line, sizeof(line), "%6ld ms  %s\n", ms, stage);
    if (length <= 0)
        return;
    (void)!write(handle, line, (size_t)length < sizeof(line) ? (size_t)length : sizeof(line) - 1);
}


/**
 * The console's hardware video decoder is a system module that is not loaded
 * by default. It has to be there before the application asks for full
 * access, so it is loaded here, in the constructor that runs before main().
 */
extern int wiliwili_videodec2_prepare(void);
extern int wiliwili_ps5_decoder_prepare(void);
static void wiliwili_report_memory(void);

extern int sceKernelSendNotificationRequest(uint32_t device, void *request, size_t size, int blocking);

struct WiliwiliNotificationRequest {
    uint8_t reserved[45];
    char message[3075];
};

void wiliwili_ps5_notify(const char *message)
{
    if (!message)
        return;
    struct WiliwiliNotificationRequest request;
    memset(&request, 0, sizeof(request));
    snprintf(request.message, sizeof(request.message), "%s", message);
    (void)sceKernelSendNotificationRequest(0, &request, sizeof(request), 0);
}

#define EBOOT_BASE 0x400000ULL

static void wiliwili_check_imports(void)
{
    FILE *f = fopen("/app0/imports.txt", "r");
    if (!f)
        f = fopen("imports.txt", "r");
    if (!f)
        return;

    char name[128];
    unsigned long offset;
    int checked = 0, missing = 0;

    while (fscanf(f, "%lx %127s", &offset, name) == 2) {
        checked++;
        if (*(const uint64_t *)(EBOOT_BASE + offset) == 0) {
            fprintf(stderr, "[wiliwili] unresolved import: %s (offset 0x%lx)\n", name, offset);
            missing++;
        }
    }
    fclose(f);
    fprintf(stderr, "[wiliwili] imports check: %d of %d unresolved\n", missing, checked);
}

__attribute__((constructor)) static void wiliwili_open_log(void)
{
    wiliwili_mark("crt");
    const char *directories[] = {"/data/wiliwili", "/download0", NULL};
    const char *opened = "log: none";
    for (size_t i = 0; directories[i] != NULL; ++i) {
        if (wiliwili_log_open(directories[i]) != NULL) {
            opened = directories[i];
            break;
        }
    }
    wiliwili_mark(opened);

    wiliwili_check_imports();

    char line[64];
    (void)snprintf(line, sizeof(line), "decoder: %d", wiliwili_videodec2_prepare());
    wiliwili_mark(line);
    wiliwili_report_memory();
    wiliwili_mark("crt done");
}

/**
 * The console has 16 GiB, but a title runs in a sandbox and only gets a
 * slice of it, sized by the system's policy for its content id. These are
 * the pools the application may actually use.
 */
static void wiliwili_report_memory(void)
{    extern int sceKernelConfiguredFlexibleMemorySize(size_t *configured);
    extern int sceKernelAvailableFlexibleMemorySize(size_t *available);
    extern int sceKernelAllocateDirectMemory(int64_t, int64_t, size_t, size_t, int, int64_t *);
    extern int sceKernelReleaseDirectMemory(int64_t, size_t);

    size_t configured = 0, available = 0;
    if (sceKernelConfiguredFlexibleMemorySize(&configured) == 0) {
        if (sceKernelAvailableFlexibleMemorySize(&available) != 0)
            available = 0;
        fprintf(stderr, "[wiliwili] flexible memory: %zu MiB free of %zu MiB\n",
                available >> 20, configured >> 20);
    }

    /* The direct memory is the pool the graphics chip reads from: measure
     * how much of it the title may have by taking it and giving it back.
     * The second argument is the limit the console reports for the process. */
    extern int64_t sceKernelGetDirectMemorySize(void);
    const int64_t direct_limit = sceKernelGetDirectMemorySize();
    const size_t chunk = 4u << 20;
    size_t total = 0;
    for (int i = 0; i < 1024; i++) {
        int64_t direct = -1;
        if (sceKernelAllocateDirectMemory(0, direct_limit, chunk, 0x4000, 12, &direct) != 0)
            break;
        total += chunk;
        if (sceKernelReleaseDirectMemory(direct, chunk) != 0)
            break;
    }
    fprintf(stderr, "[wiliwili] direct memory: %zu MiB available of %lld MiB\n",
            total >> 20, (long long)(direct_limit >> 20));
    fflush(stderr);
}

__attribute__((noreturn)) void __assert(const char *function, const char *file, int line,
                                        const char *expression)
{
    char toast[256];
    snprintf(toast, sizeof(toast), "wiliwili assert failed:\n%s\n(%s:%d)", expression ? expression : "?",
             file ? file : "?", line);
    wiliwili_ps5_notify(toast);
    fprintf(stderr, "[wiliwili] assertion failed: %s (%s:%d, %s)\n", expression, file, line,
            function);
    abort();
}


/* The SDK binds these to libScePosixForWebKit, a system library a title does not
 * load: without a definition the call jumps to nothing and the application stops
 * at its first temporary file or socket pair. */
int mkstemps(char *template_name, int suffix_length)
{
    static const char letters[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    static unsigned counter;
    const size_t length = template_name ? strlen(template_name) : 0;

    if (suffix_length < 0 || length < (size_t)suffix_length + 6u) {
        errno = EINVAL;
        return -1;
    }
    char *name = template_name + length - (size_t)suffix_length - 6u;
    for (int i = 0; i < 6; ++i) {
        if (name[i] != 'X') {
            errno = EINVAL;
            return -1;
        }
    }
    for (int attempt = 0; attempt < 100; ++attempt) {
        uint64_t value = sceKernelGetProcessTime() +
                         (uint64_t)__atomic_add_fetch(&counter, 1u, __ATOMIC_RELAXED) *
                             UINT64_C(0x9E3779B97F4A7C15);
        for (int i = 0; i < 6; ++i) {
            name[i] = letters[value % 36u];
            value /= 36u;
        }
        const int file = open(template_name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (file >= 0 || errno != EEXIST)
            return file;
    }
    errno = EEXIST;
    return -1;
}

int mkstemp(char *template_name)
{
    return mkstemps(template_name, 0);
}

/* Bound to the same library, and nothing under a title is a terminal. */
int isatty(int descriptor)
{
    (void)descriptor;
    errno = ENOTTY;
    return 0;
}

FILE *popen(const char *command, const char *mode)
{
    (void)command;
    (void)mode;
    errno = ENOSYS;
    return NULL;
}

int pclose(FILE *stream)
{
    (void)stream;
    errno = ENOSYS;
    return -1;
}
