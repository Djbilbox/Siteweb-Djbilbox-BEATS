#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace rk
{

//==============================================================================
// Stereo ping-pong delay with feedback damping (one-pole LP) and dry/wet mix.
// Buffers are allocated in prepare() (never in the audio callback).
//
// The delay time glides towards its target instead of jumping: turning the TIME
// knob (or a tempo change under a synced delay) would otherwise cut the line at
// a new position on every block and click.
//==============================================================================
class Delay
{
public:
    void setSampleRate (double sr) noexcept { sampleRate = sr; }

    // Longest delay the line can hold. 4 bars at 80 bpm is 12 seconds, so the
    // synced divisions stay exact down to slow tempos.
    static constexpr int kMaxDelayMs = 12000;

    void prepare (int maxDelayMs = kMaxDelayMs)
    {
        const int maxSamples = (int) (sampleRate * 0.001 * maxDelayMs) + 4;
        if ((int) lineL.size() < maxSamples)
        {
            lineL.assign ((size_t) maxSamples, 0.0f);
            lineR.assign ((size_t) maxSamples, 0.0f);
        }
        reset();
    }

    void reset() noexcept
    {
        std::fill (lineL.begin(), lineL.end(), 0.0f);
        std::fill (lineR.begin(), lineR.end(), 0.0f);
        writePos = 0;
        lpL = 0.0f;
        lpR = 0.0f;
        primed = false;
        cleared = true;
    }

    void process (const float* inL, const float* inR,
                  float* outL, float* outR, int numSamples,
                  float timeMs, float feedback, float mix, float lpHz, bool active) noexcept
    {
        const int size = (int) lineL.size();

        if (! active || size < 8)
        {
            // Switched off: what is left in the line would come back as ghost
            // echoes of old audio the next time the delay is switched on.
            if (! cleared)
                reset();

            for (int i = 0; i < numSamples; ++i)
            {
                outL[i] = inL[i];
                outR[i] = inR[i];
            }
            return;
        }

        cleared = false;

        const float fb = std::min (0.95f, std::max (0.0f, feedback));
        const float m  = std::min (1.0f, std::max (0.0f, mix));

        // Longest reachable delay, in samples: two spare slots for the
        // interpolation.
        const double maxDelay = (double) (size - 3);
        const double target = std::min (maxDelay, std::max (1.0, (double) timeMs * 0.001 * sampleRate));
        if (! primed)
        {
            current = target;
            primed = true;
        }

        const double glide = 1.0 - std::exp (-1.0 / (0.05 * sampleRate));   // ~50 ms

        const float fc = std::min (0.48f * (float) sampleRate, std::max (100.0f, lpHz));
        const float k = 1.0f - std::exp (-6.28318530718f * fc / (float) sampleRate);

        for (int i = 0; i < numSamples; ++i)
        {
            const double gap = target - current;
            if (std::abs (gap) > 0.0)
                current = std::abs (gap) < 0.02 ? target : current + gap * glide;

            // Linear interpolation between the two samples around the delay.
            const int whole = (int) current;
            const float frac = (float) (current - (double) whole);

            int rp = writePos - whole;
            if (rp < 0) rp += size;
            int rp1 = rp - 1;
            if (rp1 < 0) rp1 += size;

            const float dryL = inL[i];
            const float dryR = inR[i];

            // ping-pong: read from the opposite line
            const float wetL = lineR[(size_t) rp] + (lineR[(size_t) rp1] - lineR[(size_t) rp]) * frac;
            const float wetR = lineL[(size_t) rp] + (lineL[(size_t) rp1] - lineL[(size_t) rp]) * frac;

            lpL += ((dryL + wetL * fb) - lpL) * k;
            lpR += ((dryR + wetR * fb) - lpR) * k;
            lineL[(size_t) writePos] = lpL;
            lineR[(size_t) writePos] = lpR;

            outL[i] = dryL * (1.0f - m) + wetL * m;
            outR[i] = dryR * (1.0f - m) + wetR * m;

            ++writePos;
            if (writePos >= size) writePos = 0;
        }
    }

private:
    double sampleRate = 48000.0;
    std::vector<float> lineL, lineR;
    int writePos = 0;
    float lpL = 0.0f;
    float lpR = 0.0f;
    double current = 0.0;      // delay time, in samples, as it is right now
    bool primed = false;       // false until the first block after a reset
    bool cleared = true;       // the line holds nothing
};

} // namespace rk
