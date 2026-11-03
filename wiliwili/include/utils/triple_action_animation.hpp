#pragma once

#include <algorithm>
#include <array>
#include <cmath>

// Time is supplied by the caller. This keeps the three-second gesture exact
// under frame stalls and makes interruption and API latency deterministic.
class TripleActionAnimation {
public:
    enum class Phase { Idle, Charging, Pending, Compressing, Bursting, Fading, Dispersing };
    struct Particle {
        float x = 0, y = 0, radius = 0, alpha = 0;
    };
    struct Image {
        std::array<Particle, 64> particles{};
        size_t count = 0;
        float energy = 0, contraction = 0, wave = 0, waveAlpha = 0, targetAlpha = 0;
    };
    static constexpr double holdSeconds       = 3;
    static constexpr double activationSeconds = .10;
    static constexpr size_t sparkCount        = 32;

    Phase phase() const { return state; }
    bool active() const { return state != Phase::Idle; }
    bool charging() const { return state == Phase::Charging; }
    bool pending() const { return state == Phase::Pending; }
    float progress(double now) const { return std::clamp(float((now - started) / holdSeconds), 0.f, 1.f); }
    unsigned gathered(double now) const {
        unsigned result = 0;
        for (size_t i = 0; i < sparkCount; ++i) result += now - started >= arrival(i);
        return result;
    }

    bool begin(double now) {
        if (state != Phase::Idle && state != Phase::Dispersing) return false;
        const auto previous = sample(now);
        tails               = previous.particles;
        tailCount           = previous.count;
        carry               = previous.energy;
        tailStarted         = now;
        started = phaseStarted = now;
        state                  = Phase::Charging;
        return true;
    }

    // True only once, when a continuous held gesture reaches three seconds.
    bool advance(double now) {
        if (state == Phase::Charging && now - started >= holdSeconds) {
            state        = Phase::Pending;
            phaseStarted = now;
            return true;
        }
        if (state == Phase::Compressing && now - phaseStarted >= .14) {
            phaseStarted += .14;
            state = Phase::Bursting;
        }
        if (state == Phase::Bursting && now - phaseStarted >= .56) {
            phaseStarted += .56;
            state = Phase::Fading;
        }
        if ((state == Phase::Fading && now - phaseStarted >= .70) ||
            (state == Phase::Dispersing && now - phaseStarted >= .52))
            reset();
        return false;
    }

    void cancel(double now) {
        if (state != Phase::Charging && state != Phase::Pending) return;
        const auto previous = sample(now);
        if (previous.count == 0 && previous.energy <= .001f) {
            reset();
            return;
        }
        tails       = previous.particles;
        tailCount   = previous.count;
        carry       = previous.energy;
        tailStarted = phaseStarted = now;
        // Gathered light also breaks apart, even if every spark has arrived.
        for (size_t i = 0; i < 16 && tailCount < tails.size(); ++i) {
            if (carry <= .001f) break;
            const float angle  = float(i) * 2.3999632f;
            tails[tailCount++] = {float(std::cos(angle) * .12f), float(std::sin(angle) * .12f), .08f, carry * .85f};
        }
        state = Phase::Dispersing;
    }

    bool resolve(bool success, double now) {
        if (state != Phase::Pending) return false;
        if (!success) {
            cancel(now);
            return true;
        }
        const auto previous = sample(now);
        tails               = previous.particles;
        tailCount           = previous.count;
        state               = Phase::Compressing;
        phaseStarted        = now;
        return true;
    }

    void reset() {
        state     = Phase::Idle;
        tailCount = 0;
        carry     = 0;
    }

    Image sample(double now) const {
        Image image;
        if (state == Phase::Idle) return image;
        const auto append = [&](Particle p) {
            if (p.alpha > .001f && p.radius > .001f && image.count < image.particles.size())
                image.particles[image.count++] = p;
        };
        if (state == Phase::Charging || state == Phase::Pending) {
            const double elapsed = std::max(0., now - started);
            image.energy         = float(gathered(started + elapsed)) / float(sparkCount);
            image.energy += carry * std::exp(-8.f * float(now - tailStarted)) * (1 - image.energy);
            for (size_t i = 0; i < sparkCount; ++i) {
                const double age = elapsed - spawn(i);
                if (age < 0) continue;
                // Reuse a bounded stream through the three-second deadline and
                // for as long as the real request remains in flight.
                constexpr double cycle = double(sparkCount) * .067;
                const float p          = float(std::fmod(age, cycle) / duration(i));
                if (p >= 1) continue;
                const float angle = float(i) * 2.3999632f + float(std::floor(age / cycle)) * .85f + .4f * p * (1 - p);
                const float distance = (2.8f + noise(i) * 2.2f) * (1 - p * p * p);
                append({float(std::cos(angle) * distance), float(std::sin(angle) * distance), .055f + noise(i + 7) * .065f,
                        smooth(std::min(p * 8, 1.f)) * .85f});
            }
        } else if (state == Phase::Compressing) {
            image.energy      = 1;
            const float p     = std::clamp(float((now - phaseStarted) / .14), 0.f, 1.f);
            image.contraction = p * p;
            for (size_t i = 0; i < tailCount; ++i) {
                auto particle = tails[i];
                particle.x *= 1 - image.contraction;
                particle.y *= 1 - image.contraction;
                particle.radius *= 1 - p * .8f;
                particle.alpha *= 1 - p * .4f;
                append(particle);
            }
        } else if (state == Phase::Bursting || state == Phase::Fading) {
            const bool fading = state == Phase::Fading;
            const float p     = std::clamp(float((now - phaseStarted) / (fading ? .70 : .56)), 0.f, 1.f);
            image.wave        = fading ? 1 + smooth(p) * .12f : 1 - std::pow(1 - p, 3.f);
            image.waveAlpha   = fading ? .18f * (1 - p) * (1 - p) : .95f - .77f * p;
            image.energy      = fading ? .30f * (1 - p) * (1 - p) : .30f + .35f * (1 - p) * (1 - p);
            image.targetAlpha = fading ? .40f * (1 - p) * (1 - p) : .45f - .05f * p;
            for (size_t i = 0; i < 24; ++i) {
                const float angle    = float(i) * 2.3999632f;
                const float distance = 6 * image.wave * (.6f + noise(i) * .4f);
                append({float(std::cos(angle) * distance), float(std::sin(angle) * distance),
                        (.04f + noise(i + 11) * .06f) * (fading ? 1 - p : 1.f), image.waveAlpha * .6f});
            }
        } else if (state == Phase::Dispersing) {
            const float p = std::clamp(float((now - phaseStarted) / .52), 0.f, 1.f);
            image.energy  = carry * (1 - p) * (1 - p);
        }
        if (state == Phase::Charging || state == Phase::Dispersing) {
            const float p = std::clamp(float((now - tailStarted) / .52), 0.f, 1.f);
            for (size_t i = 0; i < tailCount; ++i) {
                auto particle        = tails[i];
                const float angle    = std::atan2(particle.y, particle.x) + noise(i) * .7f;
                const float distance = (1.8f + noise(i + 3) * 2.8f) * (1 - std::pow(1 - p, 2.f));
                particle.x += std::cos(angle) * distance;
                particle.y += std::sin(angle) * distance;
                particle.alpha *= (1 - p) * (1 - p);
                particle.radius *= 1 - p * .7f;
                append(particle);
            }
        }
        return image;
    }

private:
    Phase state    = Phase::Idle;
    double started = 0, phaseStarted = 0, tailStarted = 0;
    std::array<Particle, 64> tails{};
    size_t tailCount = 0;
    float carry      = 0;
    static float noise(size_t i) { return float((i * 73 + 19) % 101) / 100; }
    static double spawn(size_t i) { return activationSeconds + double(i) * .067; }
    static double duration(size_t i) { return .52 + noise(i) * .30; }
    static double arrival(size_t i) { return spawn(i) + duration(i); }
    static float smooth(float p) { return p * p * (3 - 2 * p); }
};
