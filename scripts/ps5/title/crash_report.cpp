/*
 * wiliwili - PS5 native title crash report.
 *
 * From ps5-native-app-boilerplate / ProsperoLight src/crash_report.cpp
 * (after ProsperoEden's crash report), Copyright (C) 2026 BlackBearReloaded,
 * GPL-3.0-or-later. What the handler runs uses only system calls, atomics and
 * the buffers below: the C library may be what broke.
 *
 * A title's output has no reader, so a fault is written next to the log as
 * <log dir>/crash-last.txt (the one before is kept as crash-prev.txt) and into
 * the log itself: what happened, where (as an offset into the build's
 * llvm-pie.elf) and the calls found on the stack. The system then ends the app
 * as it did without a handler. scripts/ps5/title/symbolize_crash.py names the
 * functions.
 */

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <initializer_list>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>

extern "C"
{
    int sceKernelDebugOutText(int channel, const char *text);
    int sceKernelUsleep(unsigned microseconds);
    std::uint64_t sceKernelGetProcessTime(void);
    void wiliwili_ps5_notify(const char *message);
    extern char wiliwili_last_stage[];
    // The app's code, from the link (scripts/ps5/title/ps5-pie.ld).

    extern const char __wiliwili_text_start[];
    extern const char __wiliwili_text_end[];
}

namespace
{

struct Kind
{
    int signal;
    const char *name;
    const char *meaning;
    bool fault; // the signal comes with the address that caused it
};
constexpr Kind kKinds[] = {
    {SIGSEGV, "SIGSEGV", "invalid memory access", true},
    {SIGBUS, "SIGBUS", "invalid memory access", true},
    {SIGILL, "SIGILL", "illegal instruction", true},
    {SIGFPE, "SIGFPE", "arithmetic error", true},
    {SIGABRT, "SIGABRT", "abort", false},
    {SIGSYS, "SIGSYS", "system call refused", false},
    {SIGTRAP, "SIGTRAP", "trap", true},
};

struct NamedThread
{
    std::atomic<std::uintptr_t> thread{0};
    const char *name = nullptr;
};
NamedThread g_threads[16];

std::atomic<int> g_reporting{0};
char g_report_path[200];
char g_previous_path[200];
int g_probe[2] = {-1, -1}; // memory that may be unreadable is read through it
int g_log_file = -1;

std::uintptr_t CodeStart()
{
    return reinterpret_cast<std::uintptr_t>(__wiliwili_text_start);
}
std::uintptr_t CodeEnd()
{
    return reinterpret_cast<std::uintptr_t>(__wiliwili_text_end);
}
bool InCode(std::uint64_t address)
{
    return address >= CodeStart() && address < CodeEnd();
}

// Text put together without the C library's formatting, which allocates.
struct Text
{
    char data[16 * 1024];
    std::size_t size;

    void Put(char c)
    {
        if (size + 1 < sizeof(data))
            data[size++] = c;
        data[size] = '\0';
    }
    void Put(const char *text)
    {
        for (; text && *text; ++text)
            Put(*text);
    }
    void Hex(std::uint64_t value, int digits = 1)
    {
        char buffer[16];
        int count = 0;
        do
        {
            buffer[count++] = "0123456789abcdef"[value & 15];
            value >>= 4;
        } while (value != 0 || count < digits);
        while (count > 0)
            Put(buffer[--count]);
    }
    void Dec(std::uint64_t value)
    {
        char buffer[20];
        int count = 0;
        do
        {
            buffer[count++] = static_cast<char>('0' + value % 10);
            value /= 10;
        } while (value != 0);
        while (count > 0)
            Put(buffer[--count]);
    }
    // An offset into the app's code (its address in the build's
    // llvm-pie.elf), or the bare address of anything else.
    void Address(std::uint64_t address)
    {
        if (InCode(address))
        {
            Put("eboot+0x");
            Hex(address - CodeStart());
        }
        else
        {
            Put("0x");
            Hex(address, 16);
        }
    }
};
Text g_text;

// The kernel copies memory that may not be readable: a write into a pipe,
// read back. A bad address is an error there, where it would fault here.
bool Copy(const void *address, void *out, std::size_t bytes)
{
    if (g_probe[1] < 0 || bytes == 0 || bytes > 4096)
        return false;
    const ssize_t written = write(g_probe[1], address, bytes);
    std::size_t got = 0;
    auto *target = static_cast<char *>(out);
    while (written == static_cast<ssize_t>(bytes) && got < bytes)
    {
        const ssize_t count = read(g_probe[0], target + got, bytes - got);
        if (count > 0)
            got += static_cast<std::size_t>(count);
        else if (count == 0 || errno != EINTR)
            break;
    }
    if (got == bytes)
        return true;
    char rest[256];
    while (read(g_probe[0], rest, sizeof(rest)) > 0)
    {
    }
    return false;
}

// Whether the eight bytes that end at a code address end with a call.
bool AfterCall(const unsigned char *bytes)
{
    if (bytes[3] == 0xE8)
        return true; // call rel32
    for (const int length : {2, 3, 4, 6, 7})
    {
        const unsigned char *at = bytes + 8 - length; // FF /2: call through a register or memory
        if (at[0] == 0xFF && (at[1] & 0x38) == 0x10)
            return true;
        if (length >= 3 && (at[0] & 0xF0) == 0x40 && at[1] == 0xFF && (at[2] & 0x38) == 0x10)
            return true;
    }
    return false;
}

// The app's code addresses on the stack, most recent first. The console does
// not let its code be read, so values that cannot be checked against a call
// instruction carry a "?" and scripts/ps5/title/symbolize_crash.py checks them.
void PutCalls(std::uint64_t stack)
{
    constexpr std::size_t kBytes = 128 * 1024;
    constexpr unsigned kMost = 48;
    static std::uint64_t words[512];
    unsigned found = 0;
    std::uint64_t at = stack & ~std::uint64_t{7};
    for (std::size_t seen = 0; seen < kBytes && found < kMost;)
    {
        const std::size_t page = 4096 - (at & 4095);
        const std::size_t bytes = page < sizeof(words) ? page : sizeof(words);
        if (!Copy(reinterpret_cast<const void *>(at), words, bytes))
            break;
        for (std::size_t i = 0; i < bytes / 8 && found < kMost; ++i)
        {
            const std::uint64_t value = words[i];
            if (!InCode(value) || value < CodeStart() + 8)
                continue;
            unsigned char before[8];
            const bool checked = Copy(reinterpret_cast<const void *>(value - 8), before, 8);
            if (checked && !AfterCall(before))
                continue;
            g_text.Put("  ");
            g_text.Address(value);
            g_text.Put(checked ? "\n" : " ?\n");
            ++found;
        }
        at += bytes;
        seen += bytes;
    }
    if (found == 0)
        g_text.Put("  none found\n");
}

std::uint64_t GetFirstCaller(std::uint64_t stack)
{
    constexpr std::size_t kBytes = 32 * 1024;
    static std::uint64_t words[256];
    std::uint64_t at = stack & ~std::uint64_t{7};
    for (std::size_t seen = 0; seen < kBytes; at += sizeof(words), seen += sizeof(words))
    {
        const std::size_t page = 4096 - (at & 4095);
        const std::size_t bytes = page < sizeof(words) ? page : sizeof(words);
        if (!Copy(reinterpret_cast<const void *>(at), words, bytes))
            break;
        for (std::size_t i = 0; i < bytes / 8; ++i)
        {
            const std::uint64_t value = words[i];
            if (!InCode(value) || value < CodeStart() + 8)
                continue;
            unsigned char before[8];
            if (Copy(reinterpret_cast<const void *>(value - 8), before, 8) && AfterCall(before))
                return value;
        }
    }
    return 0;
}


const char *ThreadName(std::uintptr_t thread)
{
    for (const NamedThread &entry : g_threads)
    {
        if (entry.thread.load(std::memory_order_acquire) == thread)
            return entry.name;
    }
    return "not named";
}

void PutRegister(const char *before, const char *name, std::uint64_t value)
{
    g_text.Put(before);
    g_text.Put(name);
    g_text.Put(' ');
    g_text.Hex(value, 16);
}

void Write(const char *path)
{
    const int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0)
        return;
    std::size_t done = 0;
    while (done < g_text.size)
    {
        const ssize_t count = write(file, g_text.data + done, g_text.size - done);
        if (count <= 0)
            break;
        done += static_cast<std::size_t>(count);
    }
    close(file);
}

void Handler(int signal, siginfo_t *info, void *context)
{
    const Kind *kind = nullptr;
    for (const Kind &candidate : kKinds)
    {
        if (candidate.signal == signal)
            kind = &candidate;
    }
    int idle = 0;
    if (g_reporting.compare_exchange_strong(idle, 1))
    {
        // The console's context: the machine context after 64 bytes.
        const auto *m = static_cast<const std::uint64_t *>(context) + 8;
        const std::uintptr_t self = reinterpret_cast<std::uintptr_t>(pthread_self());
        Text &t = g_text;
        t.size = 0;
        t.Put("wiliwili crash report (code size 0x");
        t.Hex(CodeEnd() - CodeStart());
        t.Put(")\nwhat: ");
        t.Put(kind ? kind->name : "signal");
        t.Put(" (");
        t.Put(kind ? kind->meaning : "unknown");
        t.Put(")");
        if (kind && kind->fault && info)
        {
            t.Put(", code ");
            t.Dec(static_cast<std::uint64_t>(info->si_code));
            t.Put(", address 0x");
            t.Hex(reinterpret_cast<std::uintptr_t>(info->si_addr), 16);
        }
        t.Put("\nafter: ");
        t.Dec(sceKernelGetProcessTime() / 1000u);
        t.Put(" ms\nwhere: ");
        t.Address(m[20]);
        if (!InCode(m[20]))
            t.Put(" (outside the app's code: a system library)");
        t.Put("\nthread: ");
        t.Put(ThreadName(self));
        t.Put(" (0x");
        t.Hex(self);
        t.Put(")\nregisters:");
        PutRegister("\n  ", "rip", m[20]);
        PutRegister("  ", "rsp", m[23]);
        PutRegister("  ", "rbp", m[9]);
        PutRegister("\n  ", "rax", m[7]);
        PutRegister("  ", "rbx", m[8]);
        PutRegister("  ", "rcx", m[4]);
        PutRegister("\n  ", "rdx", m[3]);
        PutRegister("  ", "rsi", m[2]);
        PutRegister("  ", "rdi", m[1]);
        PutRegister("\n  ", "r8 ", m[5]);
        PutRegister("  ", "r9 ", m[6]);
        PutRegister("  ", "r10", m[10]);
        PutRegister("\n  ", "r11", m[11]);
        PutRegister("  ", "r12", m[12]);
        PutRegister("  ", "r13", m[13]);
        PutRegister("\n  ", "r14", m[14]);
        PutRegister("  ", "r15", m[15]);
        t.Put("\ncode: 0x");
        t.Hex(CodeStart(), 16);
        t.Put(" to 0x");
        t.Hex(CodeEnd(), 16);
        t.Put("\ncalls found on the stack, most recent first (\"?\": not checked):\n");
        PutCalls(m[23]);
        rename(g_report_path, g_previous_path);
        Write(g_report_path);
        // The same text in the log, and one line in the kernel log. Not through
        // the log's buffer: its thread may not run again.
        if (g_log_file >= 0)
            (void)write(g_log_file, g_text.data, g_text.size);
        (void)sceKernelDebugOutText(0, "[wiliwili] crash report written to crash-last.txt\n");

        char toast[256];
        const std::uint64_t rip = m[20];
        const std::uint64_t caller = GetFirstCaller(m[23]);
        if (InCode(rip))
        {
            std::snprintf(toast, sizeof(toast), "wiliwili: crash %s (sig %d)\nat eboot+0x%llx\nstage: %s",
                          kind ? kind->name : "signal", signal,
                          static_cast<unsigned long long>(rip - CodeStart()),
                          wiliwili_last_stage);
        }
        else
        {
            std::snprintf(toast, sizeof(toast), "wiliwili: crash %s (sig %d)\nsys: 0x%llx\ncaller: eboot+0x%llx\nstage: %s",
                          kind ? kind->name : "signal", signal,
                          static_cast<unsigned long long>(rip),
                          caller ? static_cast<unsigned long long>(caller - CodeStart()) : 0ULL,
                          wiliwili_last_stage);
        }
        wiliwili_ps5_notify(toast);

        g_reporting.store(2);
    }

    else
    {
        // Another thread is writing the report: let it finish.
        for (int i = 0; i < 200 && g_reporting.load() == 1; ++i)
            sceKernelUsleep(10000);
    }
    // The system ends the app as it did before: the fault happens again
    // without a handler.
    struct sigaction plain
    {
    };
    plain.sa_handler = SIG_DFL;
    sigemptyset(&plain.sa_mask);
    sigaction(signal, &plain, nullptr);
}

void NameThread(const char *name)
{
    const std::uintptr_t self = reinterpret_cast<std::uintptr_t>(pthread_self());
    for (NamedThread &entry : g_threads)
    {
        std::uintptr_t empty = 0;
        if (entry.thread.load(std::memory_order_acquire) == self)
            return;
        if (entry.thread.compare_exchange_strong(empty, self))
        {
            entry.name = name;
            return;
        }
    }
}

// Once, early in a constructor. log_file also receives the report (-1: only
// the report file).
void Install(const char *logs_dir, int log_file)
{
    g_log_file = log_file;
    std::snprintf(g_report_path, sizeof(g_report_path), "%s/crash-last.txt", logs_dir);
    std::snprintf(g_previous_path, sizeof(g_previous_path), "%s/crash-prev.txt", logs_dir);
    if (pipe(g_probe) == 0)
    {
        // The read end must not block, and the first write on a new pipe
        // can fail: prime it once.
        fcntl(g_probe[0], F_SETFL, fcntl(g_probe[0], F_GETFL) | O_NONBLOCK);
        char byte = 0;
        if (write(g_probe[1], &byte, 1) == 1)
            (void)read(g_probe[0], &byte, 1);
    }

    static uint8_t g_alt_stack[64 * 1024] __attribute__((aligned(16)));
    stack_t ss{};
    ss.ss_sp = g_alt_stack;
    ss.ss_size = sizeof(g_alt_stack);
    ss.ss_flags = 0;
    sigaltstack(&ss, nullptr);

    struct sigaction action
    {
    };
    action.sa_sigaction = Handler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&action.sa_mask);
    int installed = 0;
    for (const Kind &kind : kKinds)
        installed += sigaction(kind.signal, &action, nullptr) == 0 ? 1 : 0;
    NameThread("main");

    std::set_terminate([]() {
        char buffer[256] = "wiliwili: terminate called";
        try
        {
            auto eptr = std::current_exception();
            if (eptr)
                std::rethrow_exception(eptr);
        }
        catch (const std::exception &e)
        {
            std::snprintf(buffer, sizeof(buffer), "wiliwili terminate:\n%s\nstage: %s", e.what(), wiliwili_last_stage);
        }
        catch (...)
        {
            std::snprintf(buffer, sizeof(buffer), "wiliwili terminate: unknown exception\nstage: %s", wiliwili_last_stage);
        }
        wiliwili_ps5_notify(buffer);
        if (g_log_file >= 0)
        {
            (void)write(g_log_file, buffer, std::strlen(buffer));
            (void)write(g_log_file, "\n", 1);
        }
        (void)sceKernelDebugOutText(0, buffer);
        abort();
    });

    // The code range answers where the loader put the image: the offsets in
    // llvm-pie.elf start at __wiliwili_text_start.
    std::printf("[wiliwili] crash report: %d handlers, probe=%d, code 0x%llx to 0x%llx\n",
                installed, g_probe[1] >= 0 ? 1 : 0,
                static_cast<unsigned long long>(CodeStart()),
                static_cast<unsigned long long>(CodeEnd()));
}


/*
 * The log file is opened by the constructor in runtime_shims.c, which this
 * object is linked after, so stdout already points at wiliwili-title.log. The
 * directory is chosen the same way there.
 */
__attribute__((constructor)) void wiliwili_install_crash_report(void)
{
    const char *directories[] = {"/data/wiliwili", "/download0", nullptr};
    for (std::size_t i = 0; directories[i] != nullptr; ++i)
    {
        if (mkdir(directories[i], 0755) == 0 || errno == EEXIST)
        {
            Install(directories[i], fileno(stdout));
            return;
        }
    }
}

} // namespace
