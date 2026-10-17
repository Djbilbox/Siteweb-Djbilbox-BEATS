#pragma once
#include <algorithm>
#include <cmath>
#include <vector>
#include "SaturnParams.h"

/** HUMPIRE SATURN - two-tap pitch-shift kernel, adapted from HUMPIRE SOLAR's PitchShifter.

    One delay line is shared by all voices. Each voice reads two taps half a grain apart and
    crossfades them with sin^2 windows. The grain length W is an even multiple of the input period,
    so the two taps read the same waveform phase and the crossfade does not cancel.
    The mean read delay of the taps is the announced latency, which keeps the dry signal and
    every harmony voice time-aligned. Nothing is allocated outside prepare(). */
namespace saturn
{
    /** Shared delay line with three channels: 0 = left, 1 = right, 2 = mono sum (used by the voices). */
    class SharedDelayLine
    {
    public:
        static constexpr int channels = 3;

        void prepare (int minLength)
        {
            int sz = 2;
            while (sz < minLength) sz <<= 1;
            size = sz;
            mask = sz - 1;
            for (auto& b : buf) b.assign ((size_t) sz, 0.0f);
            wr = 0;
        }

        void reset() noexcept
        {
            for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
            wr = 0;
        }

        /** Stores one frame (channels values) at the write position. */
        void write (const float* frame) noexcept
        {
            for (int c = 0; c < channels; ++c)
                buf[(size_t) c][(size_t) wr] = frame[c];
        }

        /** Moves the write position forward. Call once per sample, after all reads. */
        void advance() noexcept { wr = (wr + 1) & mask; }

        /** Cubic Hermite read `delay` samples behind the frame just written (delay >= 2, delay < size). */
        float read (int c, double delay) const noexcept
        {
            const double rp = (double) wr - delay;
            const int i0 = (int) std::floor (rp);
            const float f = (float) (rp - (double) i0);
            const auto& b = buf[(size_t) c];
            const int base = size;                       // keeps every index positive
            const float y0 = b[(size_t) ((i0 - 1 + base) & mask)];
            const float y1 = b[(size_t) ((i0 + base) & mask)];
            const float y2 = b[(size_t) ((i0 + 1 + base) & mask)];
            const float y3 = b[(size_t) ((i0 + 2 + base) & mask)];
            const float c1 = 0.5f * (y2 - y0);
            const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
            const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
            return ((c3 * f + c2) * f + c1) * f + y1;
        }

    private:
        std::vector<float> buf[channels];
        int size = 2, mask = 1, wr = 0;
    };

    /** Pitch-synchronous grain length. W = 2 * j * period, so the tap separation W/2 is a whole number of
        input periods. Grain bounds: 11-22 ms at 48 kHz (scaled with the sample rate). Inputs below about
        90 Hz get the largest allowed grain, where coherence is no longer guaranteed.
        The whole number j is kept while the grain stays in 0.85 * 11 ms .. 22 ms (hysteresis), so vibrato around a
        threshold does not make the grain length flip between two very different values. */
    class GrainWindow
    {
    public:
        void prepare (double sampleRate)
        {
            sr = sampleRate;
            wLo = 0.011 * sr;
            wHi = 0.022 * sr;
            W = Wtarget = 0.5 * (wLo + wHi);
            wSlew = 1.0 / (0.05 * sr);     // 50 ms glide of the grain length
            j = 0.0;
        }

        /** Input period in samples (ignored when not positive). */
        void setPeriod (double periodSamples) noexcept
        {
            if (! (periodSamples > 1.0)) return;
            const double twoT = 2.0 * periodSamples;
            const double wMin = 0.85 * wLo;
            double w = j * twoT;
            if (j < 1.0 || w < wMin || w > wHi)
            {
                j = std::max (1.0, std::ceil (wLo / twoT));
                w = j * twoT;
            }
            Wtarget = std::min (wHi, std::max (wMin, w));
        }

        void tick() noexcept { W += (Wtarget - W) * wSlew; }

        double length() const noexcept { return W; }

        /** Announced latency in samples: the mean read delay of the taps. Constant, so it never changes. */
        int latencySamples() const noexcept { return (int) std::ceil (0.5 * wHi) + 4; }

    private:
        double sr = 44100.0, wLo = 0.0, wHi = 0.0, W = 0.0, Wtarget = 0.0, wSlew = 0.0, j = 0.0;
    };

    /** Read state of one voice: phase of tap A in grains (0..1), tap B half a grain later, sin^2 crossfade. */
    struct TwoTap
    {
        double phase = 0.0, pB = 0.5;
        float gA = 0.0f, gB = 1.0f;

        void reset (double initialPhase) noexcept
        {
            phase = initialPhase;
            updateGains();
        }

        /** rate = 1 - ratio (phase advances by rate / W per sample). homeStep = largest phase move per sample at rest. */
        void step (double rate, double W, double homeStep) noexcept
        {
            if (std::abs (rate) < 5e-4)
            {
                // at rest: drift to phase 0 or 0.5, where a single tap is audible (no comb filtering)
                const double home = std::round (phase * 2.0) * 0.5;
                phase += std::max (-homeStep, std::min (homeStep, home - phase));
            }
            else
                phase += rate / W;

            phase -= std::floor (phase);
            updateGains();
        }

    private:
        void updateGains() noexcept
        {
            pB = phase + 0.5;
            pB -= std::floor (pB);
            const double sA = std::sin (kPi * phase), sB = std::sin (kPi * pB);
            gA = (float) (sA * sA);
            gB = (float) (sB * sB);
        }
    };
}
