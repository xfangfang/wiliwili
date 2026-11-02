/*
    wiliwili - the watch over the interface's main loop.
*/

#include "utils/stage.hpp"

#include <borealis/core/logger.hpp>
#include <borealis/core/thread.hpp>
#include <borealis/core/time.hpp>

#include <chrono>
#include <thread>

namespace stage
{

std::atomic<const char*> current {""};
std::atomic<uint64_t> beat {0};

void startWatch()
{
#ifdef PS5
    std::thread([]() {
        uint64_t last    = 0;
        int64_t  lastLog = 0;
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            uint64_t now = beat.load(std::memory_order_relaxed);
            int64_t  at  = brls::getCPUTimeUsec();
            if (now != last) {
                last    = now;
                lastLog = at;
                continue;
            }
            // The interface did not move for a second. Say where it stopped,
            // but not on every second: a title that is stuck says it once and
            // then every ten seconds.
            if (at - lastLog > 10 * 1000000) {
                lastLog = at;
                const char* where = current.load(std::memory_order_relaxed);
                brls::Logger::error("the interface stopped in: {}", where && *where ? where : "(nowhere)");
            }
        }
    }).detach();
#endif
}

}  // namespace stage
