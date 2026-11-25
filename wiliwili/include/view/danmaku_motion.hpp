//
// Smooth danmaku timing and subpixel NanoVG text submission.
//
// Header-only VSync frame clock and subpixel NanoVG text helpers shared by
// DanmakuCore and LiveDanmakuCore.
//

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

#include <nanovg.h>

namespace wiliwili {

// Phase-locked VSync frame clock for scrolling danmaku.
//
// Why raw wall-clock timestamps (getCPUTimeUsec / system_clock::now) stutter:
// 1. Multi-buffer FIFO swapchains (Vulkan FIFO, DXGI flip model on Windows/Xbox,
//    triple-buffered OpenGL/EGL) decouple CPU command submission from scanout. When
//    a free swapchain image is available, consecutive frames can run 0.5 ms
//    apart on the CPU and block ~32 ms on the next frame, even though the panel
//    presents them on uniform 16.67 ms (60 Hz) or 8.33 ms (120 Hz) VBlanks.
// 2. Borealis runs MPVCore::on_update in performSyncTasks() after SwapBuffers;
//    on a 24/30 fps video at 60/120 Hz, CPU work before DanmakuCore::draw()
//    alternates every frame, turning 60 Hz motion into a 30 Hz sawtooth.
//
// DanmakuMotionClock estimates both the refresh period and the centroid VSync
// phase over a 12-frame window (divisible by 2, 3, 4, and 6, cancelling
// swapchain burst pairs/triplets and 30/24 fps pulldown jitter) and applies a
// bounded phase-lock step so every presented frame advances uniformly without
// cumulative drift across 60 Hz, 120 Hz, VRR, pause/resume, or seeks.
class DanmakuMotionClock {
public:
    static constexpr size_t kHistoryCapacity    = 13;      // up to 12 intervals
    static constexpr int64_t kDiscontinuityUsec = 200000;  // 200 ms

    void reset() {
        this->initialized      = false;
        this->wasAdvancing     = false;
        this->intervalSeeded   = false;
        this->pacedUsec        = 0.0;
        this->rawToPacedOffset = 0.0;
        this->lastRawUsec      = 0;
        this->intervalUsec     = 16666.67;
        this->historyCount     = 0;
        this->historyCursor    = 0;
    }

    int64_t step(int64_t rawNowUsec, bool advancing = true) {
        if (!this->initialized) {
            this->initialized      = true;
            this->wasAdvancing     = advancing;
            this->pacedUsec        = static_cast<double>(rawNowUsec);
            this->rawToPacedOffset = 0.0;
            this->lastRawUsec      = rawNowUsec;
            this->clearAndPushHistory(rawNowUsec);
            return rawNowUsec;
        }

        const int64_t rawDt = rawNowUsec - this->lastRawUsec;
        this->lastRawUsec   = rawNowUsec;

        if (!advancing) {
            this->wasAdvancing = false;
            this->historyCount = 0;
            return static_cast<int64_t>(std::llround(this->pacedUsec));
        }

        if (!this->wasAdvancing) {
            this->wasAdvancing = true;
            this->pacedUsec += this->intervalUsec;
            this->rawToPacedOffset = static_cast<double>(rawNowUsec) - this->pacedUsec;
            this->clearAndPushHistory(rawNowUsec);
            return static_cast<int64_t>(std::llround(this->pacedUsec));
        }

        if (rawDt <= 0 || rawDt > kDiscontinuityUsec) {
            if (rawDt > kDiscontinuityUsec) {
                this->pacedUsec = static_cast<double>(rawNowUsec) - this->rawToPacedOffset;
            } else {
                this->rawToPacedOffset = static_cast<double>(rawNowUsec) - this->pacedUsec;
            }
            this->clearAndPushHistory(rawNowUsec);
            return static_cast<int64_t>(std::llround(this->pacedUsec));
        }

        this->pushHistory(rawNowUsec);

        // Use an even number of frame intervals (>= 4, up to 12) so 2-frame and
        // 3-frame swapchain bursts cancel symmetrically during warmup and steady state.
        if (this->historyCount >= 5) {
            size_t spanFrames = this->historyCount - 1;
            if ((spanFrames & 1U) != 0U) {
                spanFrames -= 1;
            }
            const size_t startIdx  = (this->historyCursor + kHistoryCapacity - 1 - spanFrames) % kHistoryCapacity;
            const double spanAvgDt = static_cast<double>(rawNowUsec - this->history[startIdx]) /
                                     static_cast<double>(spanFrames);
            const double clampedAvg = std::clamp(spanAvgDt, 3500.0, 45000.0);
            if (!this->intervalSeeded) {
                this->intervalUsec   = clampedAvg;
                this->intervalSeeded = true;
                double sum = 0.0;
                for (size_t m = 0; m < spanFrames; ++m) {
                    const size_t idx = (this->historyCursor + kHistoryCapacity - 1 - m) % kHistoryCapacity;
                    sum += static_cast<double>(this->history[idx]) + static_cast<double>(m) * this->intervalUsec;
                }
                this->rawToPacedOffset = (sum / static_cast<double>(spanFrames)) - (this->pacedUsec + this->intervalUsec);
            } else {
                const double alpha = (std::abs(clampedAvg - this->intervalUsec) > 0.25 * this->intervalUsec)
                                         ? 0.25
                                         : 0.08;
                this->intervalUsec += alpha * (clampedAvg - this->intervalUsec);
                this->intervalUsec = std::clamp(this->intervalUsec, 3500.0, 45000.0);
            }
        }

        // Estimate jitter-cancelled centroid target time across an even window W (up to 12):
        //   t_hat = (1/W) * sum_{m=0..W-1} (t_{n-m} + m * intervalUsec) - rawToPacedOffset
        size_t phaseWindow = (this->historyCount >= 12) ? 12 : (this->historyCount & ~size_t(1));
        double targetPaced = static_cast<double>(rawNowUsec) - this->rawToPacedOffset;
        if (phaseWindow >= 2) {
            double sum = 0.0;
            for (size_t m = 0; m < phaseWindow; ++m) {
                const size_t idx = (this->historyCursor + kHistoryCapacity - 1 - m) % kHistoryCapacity;
                sum += static_cast<double>(this->history[idx]) + static_cast<double>(m) * this->intervalUsec;
            }
            targetPaced = (sum / static_cast<double>(phaseWindow)) - this->rawToPacedOffset;
        }

        const double nextNominal = this->pacedUsec + this->intervalUsec;
        const double phaseError  = targetPaced - nextNominal;
        if (std::abs(phaseError) > 4.0 * this->intervalUsec) {
            this->pacedUsec = targetPaced;
            this->clearAndPushHistory(rawNowUsec);
        } else {
            const double maxAdjust  = 0.04 * this->intervalUsec;
            const double correction = std::clamp(phaseError * 0.05, -maxAdjust, maxAdjust);
            this->pacedUsec         = nextNominal + correction;
        }

        return static_cast<int64_t>(std::llround(this->pacedUsec));
    }

    int64_t currentUsec(int64_t fallbackRawNowUsec) const {
        return this->initialized ? static_cast<int64_t>(std::llround(this->pacedUsec)) : fallbackRawNowUsec;
    }

    double estimatedIntervalUsec() const { return this->intervalUsec; }

private:
    void clearAndPushHistory(int64_t rawNowUsec) {
        this->historyCount  = 0;
        this->historyCursor = 0;
        this->pushHistory(rawNowUsec);
    }

    void pushHistory(int64_t rawNowUsec) {
        this->history[this->historyCursor] = rawNowUsec;
        this->historyCursor                = (this->historyCursor + 1) % kHistoryCapacity;
        if (this->historyCount < kHistoryCapacity) {
            this->historyCount++;
        }
    }

    bool initialized        = false;
    bool wasAdvancing       = false;
    bool intervalSeeded     = false;
    double pacedUsec        = 0.0;
    double rawToPacedOffset = 0.0;
    int64_t lastRawUsec     = 0;
    double intervalUsec     = 16666.67;
    int64_t history[kHistoryCapacity]{};
    size_t historyCount  = 0;
    size_t historyCursor = 0;
};

// Submits single-line danmaku text with continuous subpixel horizontal
// translation while keeping vertical baseline snapped to physical pixel rows.
//
// Passing floating-point x directly to nvgText() routes x*scale into Fontstash's
// fons__getQuad(), which executes `rx = floorf(*x + xoff)` and quantizes every
// glyph to integer pixels (producing 2px/3px or 1px/2px frame-step beats and
// inter-glyph kerning wobble). Translating via state->xform lays out glyphs at
// constant local integer offsets from (0, 0) while nvgTransformPoint() shifts
// quad vertices continuously in float, engaging hardware GL_LINEAR filtering
// across Fontstash's 2px glyph padding border.
inline float snapVerticalPixel(float y, float devicePxRatio) {
    if (devicePxRatio <= 0.0f) return y;
    return std::floor(y * devicePxRatio + 0.5f) / devicePxRatio;
}

inline void drawSubpixelText(NVGcontext* vg, float x, float y, float devicePxRatio,
                             const char* str, const char* end = nullptr) {
    nvgSave(vg);
    nvgTranslate(vg, x, snapVerticalPixel(y, devicePxRatio));
    nvgText(vg, 0.0f, 0.0f, str, end);
    nvgRestore(vg);
}

}  // namespace wiliwili
