/*
    wiliwili - the stage the interface is in, and a watch over it.

    A title that stops answering does not write anything: the thread that
    stopped is the one that writes the log. The watch keeps the last stage
    the interface was in, and a thread of its own says so when the interface
    stops moving.

    The stage is set around the work that can block - a picture that is put on
    the screen, a font that is drawn, a picture that is fetched - so that the
    log names the place a title got stuck at.
*/

#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace stage
{

/// The last place the interface was in.
extern std::atomic<const char*> current;

/// Counts the turns of the interface's main loop.
extern std::atomic<uint64_t> beat;

/// Starts the watch. It writes a line every second when the interface stops
/// moving, naming the place it stopped at.
void startWatch();

/// Says that the interface moved on.
inline void tick()
{
    beat.fetch_add(1, std::memory_order_relaxed);
}

/**
 * Names the place the interface is in.
 *
 * The name stays when the place is left: a title that stops says where it was
 * last, which is the closest hint a watch can give.
 */
class Marker
{
  public:
    explicit Marker(const char* name) { current.store(name, std::memory_order_relaxed); }

    Marker(const Marker&)            = delete;
    Marker& operator=(const Marker&) = delete;
};

}  // namespace stage
