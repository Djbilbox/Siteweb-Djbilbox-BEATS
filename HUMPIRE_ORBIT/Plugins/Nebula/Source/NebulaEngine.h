#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <juce_core/juce_core.h>

/** HUMPIRE NEBULA - shared-delay-line unison engine (1 to 32 voices).

    Cost model (per sample, stereo output):
      - ONE mono delay line is written per sample and shared by every voice;
      - each voice reads it through two Hermite-interpolated taps (a crossfaded pair
        whose read speed is set by the voice detune, so the detune is a real pitch offset);
      - each voice adds one LFO lookup (shared rate, per-voice phase) and two window lookups;
      - the summed wet signal goes through one 2-pole tone low-pass shared by all voices.
    No voice owns a pitch shifter, no allocation happens after prepare(), no locks are used.

    Detune: a voice whose read head drifts at rate r = 2^(cents/1200) is pitch shifted by r.
    The drift is kept bounded by a sawtooth window of kWindowMs; the two taps are offset by
    half a window and crossfaded with sin^2 / cos^2 so the wrap-around is inaudible. */
class NebulaEngine
{
public:
    static constexpr int maxVoices = 32;

    struct Settings
    {
        int   voices      = 8;
        float detuneCents = 12.0f;   // voices spread evenly from -detune to +detune
        float delayMs     = 14.0f;   // base delay of the ensemble (2 .. 40 ms)
        float staggerMs   = 4.0f;    // extra delay per voice, spread quasi-randomly (0 .. 30 ms)
        float spread      = 0.8f;    // stereo spread of the voices, 0 = mono, 1 = full width
        float modRateHz   = 0.45f;   // LFO rate shared by all voices (each voice has its own phase)
        float modDepth    = 0.3f;    // 0 .. 1 maps to 0 .. kMaxModMs of delay wobble
        float tone        = 0.7f;    // 0 .. 1 : wet low-pass cutoff, ~0.9 kHz (dark) to ~22 kHz (open)
    };

    NebulaEngine()
    {
        constexpr double pi = juce::MathConstants<double>::pi;
        for (int k = 0; k < kTableSize; ++k)
        {
            const double ph = 2.0 * pi * (double) k / (double) kTableSize;
            sinTab[(size_t) k] = (float) std::sin (ph);
            const double s = std::sin (pi * (double) k / (double) kTableSize);
            winTab[(size_t) k] = (float) (s * s);                 // sin^2(pi x), x = k / N
        }
        for (int i = 0; i < maxVoices; ++i)
        {
            auto& v = voices[(size_t) i];
            v.x         = frac ((double) i * 0.5698402910);       // initial window phase
            v.spreadU   = (float) frac ((double) i * 0.6180339887);
            v.lfoOff    = (int) (frac ((double) i * 0.7548776662) * (double) kTableSize) & (kTableSize - 1);
        }
    }

    /** Allocates the shared delay line for the given sample rate. Not real-time safe: call from prepareToPlay only. */
    void prepare (double sampleRate)
    {
        sr = sampleRate > 1000.0 ? sampleRate : 44100.0;
        const int needed = (int) std::ceil (0.25 * sr);            // 250 ms of history
        size = 1;
        while (size < needed) size <<= 1;
        mask = size - 1;
        delayLine.assign ((size_t) size, 0.0f);
        writeIdx = 0;

        winSamples = juce::jmax (64.0, 0.025 * sr);                // 25 ms detune window
        kSmooth = 1.0f - std::exp ((float) (-1.0 / (0.030 * sr)));  // 30 ms smoothing of scalar params
        kPan    = 1.0f - std::exp ((float) (-1.0 / (0.020 * sr)));  // 20 ms voice pan / gain ramps
        kFade   = 1.0f - std::exp ((float) (-1.0 / (0.020 * sr)));  // 20 ms voice on/off fades

        updateTargets();
        reset();
        snapToTargets();
    }

    /** Clears the delay line and the filter state (keeps the parameter targets). */
    void reset() noexcept
    {
        std::fill (delayLine.begin(), delayLine.end(), 0.0f);
        yL1 = yL2 = yR1 = yR2 = 0.0f;
    }

    /** Sets the target parameters. Cheap (a few dozen trig calls); called once per audio block. */
    void setSettings (const Settings& s) noexcept
    {
        cur = s;
        if (size > 0) updateTargets();
    }

    /** Jumps every smoothed value to its target (used at start-up so the first block is not ramped). */
    void snapToTargets() noexcept
    {
        base = baseTgt; stagger = staggerTgt; depth = depthTgt; toneA = toneTgt;
        for (auto& v : voices) { v.gL = v.tgtL; v.gR = v.tgtR; v.fade = v.fadeTgt; }
    }

    /** Processes one mono input sample and returns the stereo wet signal (dry mixing is done by the caller). */
    void processSample (float input, float& wetL, float& wetR) noexcept
    {
        delayLine[(size_t) writeIdx] = input;

        base    += (baseTgt    - base)    * kSmooth;
        stagger += (staggerTgt - stagger) * kSmooth;
        depth   += (depthTgt   - depth)   * kSmooth;
        toneA   += (toneTgt    - toneA)   * kSmooth;

        lfoPhase += lfoInc;                                        // 32-bit phase accumulator
        const unsigned lfoIdx = lfoPhase >> 20;                    // top 12 bits = table index

        float sumL = 0.0f, sumR = 0.0f;
        for (auto& v : voices)
        {
            v.x += v.dx;                                           // window phase, 0 .. 1
            if (v.x >= 1.0) v.x -= 1.0;
            else if (v.x < 0.0) v.x += 1.0;

            v.gL   += (v.tgtL - v.gL) * kPan;
            v.gR   += (v.tgtR - v.gR) * kPan;
            v.fade += (v.fadeTgt - v.fade) * kFade;
            if (v.fade < 1.0e-4f && v.fadeTgt < 0.5f) { v.fade = 0.0f; continue; }   // silent voice: skip work

            const int li   = (int) ((lfoIdx + (unsigned) v.lfoOff) & (unsigned) (kTableSize - 1));
            const float mod = depth * sinTab[(size_t) li];
            const float d0  = base + stagger * v.spreadU + mod;

            const double xB = (v.x < 0.5) ? v.x + 0.5 : v.x - 0.5;  // second tap, half a window away
            const float dA = juce::jlimit (2.0f, (float) (size - 8), d0 + (float) (winSamples * v.x));
            const float dB = juce::jlimit (2.0f, (float) (size - 8), d0 + (float) (winSamples * xB));

            const int xi  = (int) (v.x * (double) kTableSize) & (kTableSize - 1);
            const int xbi = (xi + kTableSize / 2) & (kTableSize - 1);
            const float vo = (readTap (dA) * winTab[(size_t) xi] + readTap (dB) * winTab[(size_t) xbi]) * v.fade;

            sumL += vo * v.gL;
            sumR += vo * v.gR;
        }

        // shared tone filter: two cascaded one-pole low-passes (-12 dB/oct)
        yL1 += toneA * (sumL - yL1);  yL2 += toneA * (yL1 - yL2);
        yR1 += toneA * (sumR - yR1);  yR2 += toneA * (yR1 - yR2);
        wetL = yL2;
        wetR = yR2;

        writeIdx = (writeIdx + 1) & mask;
    }

private:
    static constexpr int kTableSize = 4096;     // power of two
    static constexpr float kMaxModMs = 3.0f;

    struct Voice
    {
        double x = 0.0;          // window phase 0..1 (sawtooth of the detune drift)
        double dx = 0.0;         // window phase increment per sample (block constant)
        float  spreadU = 0.0f;   // quasi-random 0..1 delay stagger of this voice
        int    lfoOff = 0;       // LFO phase offset of this voice (table index)
        float  tgtL = 0.0f, tgtR = 0.0f, gL = 0.0f, gR = 0.0f;   // pan gains, target and smoothed
        float  fadeTgt = 0.0f, fade = 0.0f;                        // on/off ramp
    };

    static double frac (double x) noexcept { return x - std::floor (x); }

    /** Hermite-interpolated read of the shared delay line, `delaySamples` behind the write head (>= 2). */
    float readTap (float delaySamples) const noexcept
    {
        const int di = (int) delaySamples;
        const float t = 1.0f - (delaySamples - (float) di);
        const int i2 = (writeIdx - di + size) & mask;
        const float y0 = delayLine[(size_t) ((i2 + size - 2) & mask)];
        const float y1 = delayLine[(size_t) ((i2 + size - 1) & mask)];
        const float y2 = delayLine[(size_t) i2];
        const float y3 = delayLine[(size_t) ((i2 + 1) & mask)];
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
        return ((c3 * t + c2) * t + c1) * t + y1;
    }

    /** Recomputes every per-voice and per-ensemble target from `cur`. */
    void updateTargets() noexcept
    {
        const int n = juce::jlimit (1, maxVoices, cur.voices);
        const double norm = 1.0 / std::sqrt ((double) n);       // keeps loudness roughly constant with voice count
        constexpr double pi = juce::MathConstants<double>::pi;

        for (int i = 0; i < maxVoices; ++i)
        {
            auto& v = voices[(size_t) i];
            const bool active = i < n;
            const double u = (active && n > 1) ? (2.0 * (double) i / (double) (n - 1) - 1.0) : 0.0;   // -1 .. +1

            const double rate = std::exp2 ((double) juce::jlimit (0.0f, 50.0f, cur.detuneCents) * u / 1200.0);
            v.dx = (1.0 - rate) / winSamples;                    // d(delay)/dn = 1 - rate  =>  pitch ratio = rate

            const double theta = (u * (double) juce::jlimit (0.0f, 1.0f, cur.spread) + 1.0) * (pi * 0.25);
            v.tgtL = (float) (std::cos (theta) * norm);
            v.tgtR = (float) (std::sin (theta) * norm);
            v.fadeTgt = active ? 1.0f : 0.0f;
        }

        baseTgt    = juce::jlimit (2.0f, 40.0f, cur.delayMs) * (float) sr * 0.001f;
        staggerTgt = juce::jlimit (0.0f, 30.0f, cur.staggerMs) * (float) sr * 0.001f;
        depthTgt   = juce::jlimit (0.0f, 1.0f, cur.modDepth) * kMaxModMs * (float) sr * 0.001f;
        lfoInc     = (std::uint32_t) (juce::jlimit (0.01f, 20.0f, cur.modRateHz) / sr * 4294967296.0);

        const double fc = 900.0 * std::pow (22000.0 / 900.0, (double) juce::jlimit (0.0f, 1.0f, cur.tone));
        toneTgt = (float) (1.0 - std::exp (-2.0 * juce::MathConstants<double>::pi * fc / sr));
    }

    Settings cur;
    std::array<Voice, maxVoices> voices {};
    std::array<float, kTableSize> sinTab {};
    std::array<float, kTableSize> winTab {};

    std::vector<float> delayLine;
    int size = 16, mask = 15, writeIdx = 0;
    double sr = 44100.0;
    double winSamples = 1200.0;

    float baseTgt = 0.0f, base = 0.0f;
    float staggerTgt = 0.0f, stagger = 0.0f;
    float depthTgt = 0.0f, depth = 0.0f;
    float toneTgt = 1.0f, toneA = 1.0f;
    float kSmooth = 0.0f, kPan = 0.0f, kFade = 0.0f;
    std::uint32_t lfoInc = 0, lfoPhase = 0;

    float yL1 = 0.0f, yL2 = 0.0f, yR1 = 0.0f, yR2 = 0.0f;
};
