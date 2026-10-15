#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <juce_core/juce_core.h>
#include "TwinParams.h"

/** HUMPIRE TWIN - DSP core.
    One shared mono delay line feeds up to four doubles. Each double is a Doppler pitch shifter
    (two taps cross-faded with a sin^2 window over a 10 ms cycle) with its own micro-detune, its own
    micro-delay (5-40 ms) and a slow random modulation (Humanize / Drift). The doubles are panned,
    summed to a stereo wet bus, then high-pass and low-pass filtered. No allocation in process(). */
namespace twin
{
    inline constexpr float kMinDelayMs = 5.0f;
    inline constexpr float kMaxDelayMs = 40.0f;

    /** Real-unit settings, pushed once per block. */
    struct DoublerSettings
    {
        int   voices   = 2;
        float detuneCt = 12.0f;    // maximum micro-detune of a double, cents
        float delayMs  = 18.0f;    // base micro-delay, 5..40 ms
        float humanize = 0.4f;     // 0..1, depth of the random modulation
        float driftHz  = 0.35f;    // average rate of the random modulation
        float width    = 0.8f;     // 0..1, stereo spread of the doubles
        float hpHz     = 120.0f;
        float lpHz     = 12000.0f;
    };

    /** Second-order RBJ biquad, Direct Form II transposed. */
    class Biquad
    {
    public:
        void reset() noexcept { z1 = 0.0f; z2 = 0.0f; }

        void setLowPass (double sr, double freq, double q) noexcept
        {
            const double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
            const double cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
            store ((1.0 - cw) * 0.5 / a0, (1.0 - cw) / a0, (1.0 - cw) * 0.5 / a0, -2.0 * cw / a0, (1.0 - alpha) / a0);
        }

        void setHighPass (double sr, double freq, double q) noexcept
        {
            const double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
            const double cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
            store ((1.0 + cw) * 0.5 / a0, -(1.0 + cw) / a0, (1.0 + cw) * 0.5 / a0, -2.0 * cw / a0, (1.0 - alpha) / a0);
        }

        float process (float x) noexcept
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

    private:
        void store (double nb0, double nb1, double nb2, double na1, double na2) noexcept
        {
            b0 = (float) nb0; b1 = (float) nb1; b2 = (float) nb2;
            a1 = (float) na1; a2 = (float) na2;
        }

        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f, z1 = 0.0f, z2 = 0.0f;
    };

    /** Small deterministic xorshift generator (no allocation, reproducible). */
    struct Random
    {
        std::uint32_t s = 0x9E3779B9u;

        void seed (std::uint32_t v) noexcept { s = v != 0u ? v : 0x9E3779B9u; }

        std::uint32_t next() noexcept
        {
            s ^= s << 13;
            s ^= s >> 17;
            s ^= s << 5;
            return s;
        }

        float unit() noexcept { return (float) (next() & 0xFFFFFFu) * (1.0f / 16777216.0f); }
        float bipolar() noexcept { return unit() * 2.0f - 1.0f; }
    };

    /** Slow random modulation: cosine interpolation between random targets, output in -1..1. */
    struct Drift
    {
        Random rng;
        float from = 0.0f, to = 0.0f, pos = 0.0f, inc = 0.0f;

        void start (std::uint32_t seed) noexcept
        {
            rng.seed (seed);
            from = 0.0f;
            to = rng.bipolar();
            pos = 0.0f;
            inc = 0.0f;
        }

        /** rateHz = average number of new random targets per second. */
        float next (float rateHz, double invSr) noexcept
        {
            if (inc <= 0.0f) inc = (float) ((double) rateHz * invSr);
            pos += inc;
            if (pos >= 1.0f)
            {
                pos -= 1.0f;
                from = to;
                to = rng.bipolar();
                inc = (float) ((double) rateHz * invSr * (0.6 + 0.8 * (double) rng.unit()));
            }
            const float w = 0.5f - 0.5f * std::cos (juce::MathConstants<float>::pi * pos);
            return from + (to - from) * w;
        }
    };

    /** Per-voice stereo pattern: detune and delay multipliers, and pan positions in -1..1. */
    inline constexpr float kDelayMul[kMaxVoices]  = {  1.00f,  0.72f,  1.28f,  0.86f };
    inline constexpr float kDetuneMul[kMaxVoices] = {  1.00f, -0.85f,  0.62f, -1.18f };
    inline constexpr float kPanPos[kMaxVoices]    = { -1.00f,  1.00f, -0.55f,  0.55f };

    class Doubler
    {
    public:
        /** Sizes the delay line and smoothers. Not real-time safe: call from prepareToPlay only. */
        void prepare (double sampleRate);

        /** Clears the delay line and snaps every smoother to its target. */
        void reset() noexcept;

        /** Pushes new settings (called once per block, no allocation). */
        void setSettings (const DoublerSettings& s) noexcept;

        /** Re-seeds the random modulation. */
        void reseed (std::uint32_t seed) noexcept;

        /** One sample: mono in, stereo wet out (already filtered). */
        void process (float monoIn, float& wetL, float& wetR) noexcept;

        int   getActiveVoices() const noexcept { return activeVoices; }
        float getVoiceDelayMs (int k) const noexcept { return voices[(size_t) k].delayMs; }
        float getVoiceDetuneCt (int k) const noexcept { return voices[(size_t) k].detuneCt; }
        float getVoicePan (int k) const noexcept { return voices[(size_t) k].pan; }

    private:
        struct Voice
        {
            // targets, written by setSettings()
            float targetDelayMs = 18.0f, targetDetuneCt = 0.0f, targetPan = 0.0f;
            float targetGainL = 0.0f, targetGainR = 0.0f;
            // smoothed state, advanced per sample
            float delayMs = 18.0f, detuneCt = 0.0f, pan = 0.0f;
            float gainL = 0.0f, gainR = 0.0f;
            float phase = 0.0f;          // Doppler sawtooth, 0..windowSamples
            Drift delayDrift, detuneDrift;
        };

        float readAt (float delaySamples) const noexcept;
        void updateFilters() noexcept;

        double sr = 48000.0, invSr = 1.0 / 48000.0;
        float samplesPerMs = 48.0f, windowSamples = 480.0f;
        int bufLen = 0, mask = 0, writeIdx = 0;
        std::vector<float> buffer;

        std::array<Voice, kMaxVoices> voices;
        DoublerSettings target;
        int activeVoices = 2;
        float depthDelayMs = 0.0f, depthDetuneCt = 0.0f, driftHz = 0.35f;
        float normTarget = 1.0f, normSm = 1.0f;
        float hpTargetLog = 0.0f, lpTargetLog = 0.0f, hpLog = 0.0f, lpLog = 0.0f;
        float coefDelay = 0.0f, coefDetune = 0.0f, coefPan = 0.0f, coefGain = 0.0f, coefFilter = 0.0f;
        Biquad hpL, hpR, lpL, lpR;
        unsigned filterCounter = 0u;
    };

    // ------------------------------------------------------------------------------------------
    inline void Doubler::prepare (double newSr)
    {
        sr = juce::jlimit (8000.0, 384000.0, newSr);
        invSr = 1.0 / sr;
        samplesPerMs = (float) (sr * 0.001);
        windowSamples = (float) (0.010 * sr);            // 10 ms Doppler cycle

        const int need = (int) std::ceil (0.06 * sr) + 64;   // longest tap is ~45 ms
        int len = 1;
        while (len < need) len <<= 1;
        bufLen = len;
        mask = len - 1;
        buffer.assign ((size_t) len, 0.0f);
        writeIdx = 0;

        coefDelay  = (float) (1.0 - std::exp (-1.0 / (0.06 * sr)));
        coefDetune = (float) (1.0 - std::exp (-1.0 / (0.04 * sr)));
        coefPan    = (float) (1.0 - std::exp (-1.0 / (0.03 * sr)));
        coefGain   = (float) (1.0 - std::exp (-1.0 / (0.03 * sr)));
        coefFilter = (float) (1.0 - std::exp (-32.0 / (0.05 * sr)));   // filters update every 32 samples

        const DoublerSettings current = target;
        setSettings (current);
        reset();
    }

    inline void Doubler::reset() noexcept
    {
        if (! buffer.empty()) std::fill (buffer.begin(), buffer.end(), 0.0f);
        writeIdx = 0;
        for (int k = 0; k < kMaxVoices; ++k)
        {
            Voice& v = voices[(size_t) k];
            v.delayMs = v.targetDelayMs;
            v.detuneCt = v.targetDetuneCt;
            v.pan = v.targetPan;
            v.gainL = v.targetGainL;
            v.gainR = v.targetGainR;
            v.phase = 0.0f;
            v.delayDrift.start (0x1234567u + 0x9E3779B9u * (std::uint32_t) (k + 1));
            v.detuneDrift.start (0x7654321u + 0x85EBCA6Bu * (std::uint32_t) (k + 1));
        }
        normSm = normTarget;
        hpLog = hpTargetLog;
        lpLog = lpTargetLog;
        hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset();
        updateFilters();
    }

    inline void Doubler::setSettings (const DoublerSettings& s) noexcept
    {
        target = s;
        target.voices = juce::jlimit (1, kMaxVoices, s.voices);
        activeVoices = target.voices;

        const float humanizeAmount = juce::jlimit (0.0f, 1.0f, s.humanize);
        const float stereoWidth = juce::jlimit (0.0f, 1.0f, s.width);
        const float baseDelay = juce::jlimit (kMinDelayMs, kMaxDelayMs, s.delayMs);
        depthDelayMs  = humanizeAmount * 8.0f;     // +-8 ms at 100 %
        depthDetuneCt = humanizeAmount * 6.0f;     // +-6 cents at 100 %
        driftHz       = juce::jlimit (0.02f, 4.0f, s.driftHz);
        normTarget    = 1.0f / std::sqrt ((float) target.voices);
        hpTargetLog   = std::log2 (juce::jlimit (20.0f, 800.0f, s.hpHz));
        lpTargetLog   = std::log2 (juce::jlimit (1000.0f, 20000.0f, s.lpHz));

        for (int k = 0; k < kMaxVoices; ++k)
        {
            Voice& v = voices[(size_t) k];
            const bool on = k < target.voices;
            const float angle = (stereoWidth * kPanPos[k] + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
            v.targetDelayMs  = juce::jlimit (kMinDelayMs, kMaxDelayMs, baseDelay * kDelayMul[k]);
            v.targetDetuneCt = s.detuneCt * kDetuneMul[k];
            v.targetPan      = stereoWidth * kPanPos[k];
            v.targetGainL    = on ? std::cos (angle) : 0.0f;   // constant-power pan law
            v.targetGainR    = on ? std::sin (angle) : 0.0f;
        }
    }

    inline void Doubler::reseed (std::uint32_t seed) noexcept
    {
        for (int k = 0; k < kMaxVoices; ++k)
        {
            Voice& v = voices[(size_t) k];
            const auto kk = (std::uint32_t) (k + 1);
            v.delayDrift.start (seed * 2654435761u + kk * 977u);
            v.detuneDrift.start (seed * 40503u + kk * 7919u + 12345u);
        }
    }

    /** Linear-interpolated read `delaySamples` behind the most recent input sample. */
    inline float Doubler::readAt (float delaySamples) const noexcept
    {
        const float pos = (float) (writeIdx + bufLen - 1) - delaySamples;   // positive: delay < bufLen
        const int i0 = (int) pos;
        const float frac = pos - (float) i0;
        const auto a = (size_t) (i0 & mask);
        const auto b = (size_t) ((i0 + 1) & mask);
        return buffer[a] + (buffer[b] - buffer[a]) * frac;
    }

    inline void Doubler::process (float x, float& outL, float& outR) noexcept
    {
        buffer[(size_t) writeIdx] = x;
        writeIdx = (writeIdx + 1) & mask;

        const float halfW = 0.5f * windowSamples;
        const float winInc = juce::MathConstants<float>::twoPi / windowSamples;
        float sumL = 0.0f, sumR = 0.0f;

        for (int k = 0; k < kMaxVoices; ++k)
        {
            Voice& v = voices[(size_t) k];

            // Smoothed live values: targets plus slow random modulation (Humanize)
            const float dMod = v.delayDrift.next (driftHz, invSr);
            const float cMod = v.detuneDrift.next (driftHz, invSr);
            const float delayGoal = juce::jlimit (kMinDelayMs, kMaxDelayMs, v.targetDelayMs + depthDelayMs * dMod);
            v.delayMs  += (delayGoal - v.delayMs) * coefDelay;
            v.detuneCt += (v.targetDetuneCt + depthDetuneCt * cMod - v.detuneCt) * coefDetune;
            v.pan      += (v.targetPan - v.pan) * coefPan;
            v.gainL    += (v.targetGainL - v.gainL) * coefGain;
            v.gainR    += (v.targetGainR - v.gainR) * coefGain;

            // Doppler pitch shifter: the read head runs at `ratio` against the writer,
            // so its delay grows by (1 - ratio) per sample; the sawtooth wraps every window.
            const float ratio = std::exp2 (v.detuneCt / 1200.0f);
            v.phase += 1.0f - ratio;
            if (v.phase >= windowSamples)  v.phase -= windowSamples;
            else if (v.phase < 0.0f)       v.phase += windowSamples;

            const float phaseB = v.phase >= halfW ? v.phase - halfW : v.phase + halfW;
            const float wA = 0.5f - 0.5f * std::cos (winInc * v.phase);   // sin^2 window, wA + wB = 1
            const float dBase = juce::jmax (1.0f, v.delayMs * samplesPerMs - halfW);
            const float y = wA * readAt (dBase + v.phase) + (1.0f - wA) * readAt (dBase + phaseB);

            sumL += v.gainL * y;
            sumR += v.gainR * y;
        }

        normSm += (normTarget - normSm) * coefGain;
        sumL *= normSm;
        sumR *= normSm;

        if ((++filterCounter & 31u) == 0u) updateFilters();
        outL = lpL.process (hpL.process (sumL));
        outR = lpR.process (hpR.process (sumR));
    }

    inline void Doubler::updateFilters() noexcept
    {
        hpLog += (hpTargetLog - hpLog) * coefFilter;
        lpLog += (lpTargetLog - lpLog) * coefFilter;
        const double hz = std::exp2 ((double) hpLog);
        const double lz = juce::jmin (std::exp2 ((double) lpLog), 0.45 * sr);
        hpL.setHighPass (sr, hz, 0.7071);
        hpR.setHighPass (sr, hz, 0.7071);
        lpL.setLowPass (sr, lz, 0.7071);
        lpR.setLowPass (sr, lz, 0.7071);
    }
}
