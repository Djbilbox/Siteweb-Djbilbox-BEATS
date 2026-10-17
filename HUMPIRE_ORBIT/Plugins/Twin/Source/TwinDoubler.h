#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <juce_core/juce_core.h>
#include "TwinParams.h"

/** HUMPIRE TWIN - DSP core.

    One shared mono delay line feeds up to four doubles. Each double is a delay-line pitch shifter
    with its own micro-detune, its own micro-delay (5-40 ms) and a slow random modulation
    (Humanize / Drift). The doubles are panned, summed to a stereo wet bus, then high-pass and
    low-pass filtered. Nothing is allocated in process().

    Pitch shifting: a read tap whose delay changes by (1 - ratio) samples per sample plays the input
    at `ratio` times its speed. The delay can not drift forever, so every so often the tap is
    cross-faded (10 ms) to a second tap at a different delay. A plain cross-fade of two arbitrary
    taps turns the shifter into a frequency shifter (a constant number of Hz instead of a constant
    number of cents) because the two taps are not in phase. To keep the shift a true pitch ratio the
    new tap is placed by a short correlation search: its delay is chosen so that the waveform it
    plays matches the waveform the old tap has just played (a whole number of pitch periods away).
    The search runs on a 12 kHz decimated copy of the input plus a full-rate refinement, and only
    when a cross-fade is due, so its cost is small and independent of the sample rate. */
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

    /** Normalised biquad coefficients (a0 = 1). */
    struct BiquadCoeffs
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    };

    /** RBJ low-pass. */
    inline BiquadCoeffs makeLowPass (double sr, double freq, double q) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        const double cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
        BiquadCoeffs c;
        c.b0 = (1.0 - cw) * 0.5 / a0;  c.b1 = (1.0 - cw) / a0;  c.b2 = c.b0;
        c.a1 = -2.0 * cw / a0;         c.a2 = (1.0 - alpha) / a0;
        return c;
    }

    /** RBJ high-pass. */
    inline BiquadCoeffs makeHighPass (double sr, double freq, double q) noexcept
    {
        const double w0 = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        const double cw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
        BiquadCoeffs c;
        c.b0 = (1.0 + cw) * 0.5 / a0;  c.b1 = -(1.0 + cw) / a0;  c.b2 = c.b0;
        c.a1 = -2.0 * cw / a0;         c.a2 = (1.0 - alpha) / a0;
        return c;
    }

    /** Second-order section, Direct Form II transposed, double precision state (a 20 Hz high-pass
        is too close to z = 1 for a float state). */
    class Biquad
    {
    public:
        void reset() noexcept { z1 = 0.0; z2 = 0.0; }
        void setCoeffs (const BiquadCoeffs& c) noexcept { k = c; }

        float process (float x) noexcept
        {
            const double in = (double) x;
            const double y = k.b0 * in + z1;
            z1 = k.b1 * in - k.a1 * y + z2;
            z2 = k.b2 * in - k.a2 * y;
            return (float) y;
        }

    private:
        BiquadCoeffs k;
        double z1 = 0.0, z2 = 0.0;
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

    /** Slow random modulation: smooth interpolation between random targets, output in -1..1. */
    struct Drift
    {
        Random rng;
        float from = 0.0f, to = 0.0f, scale = 1.0f;
        double pos = 0.0;

        void start (std::uint32_t seed) noexcept
        {
            rng.seed (seed);
            from = 0.0f;
            to = rng.bipolar();
            pos = 0.0;
            scale = 1.0f;
        }

        /** rateHz = average number of new random targets per second. The rate is read every sample,
            so a Drift automation takes effect at once. */
        float next (float rateHz, double invSr) noexcept
        {
            pos += (double) rateHz * invSr * (double) scale;
            if (pos >= 1.0)
            {
                pos -= 1.0;
                from = to;
                to = rng.bipolar();
                scale = 0.6f + 0.8f * rng.unit();
            }
            const float p = (float) pos;
            const float w = p * p * (3.0f - 2.0f * p);   // smoothstep: zero slope at every target
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
        /** Sizes the delay lines and smoothers. Not real-time safe: call from prepareToPlay only. */
        void prepare (double sampleRate);

        /** Clears the delay lines and snaps every smoother to its target. */
        void reset() noexcept;

        /** Pushes new settings (called once per block, no allocation). */
        void setSettings (const DoublerSettings& s) noexcept;

        /** Re-seeds the random modulation. */
        void reseed (std::uint32_t seed) noexcept;

        /** One sample: mono in, stereo wet out (already filtered). Silence before prepare(). */
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
            // read taps: delay = centre delay + offset (samples). Tap A is audible, tap B takes over
            // after a cross-fade.
            float offA = 0.0f, offB = 0.0f, fadePos = 0.0f;
            bool fading = false;
            Drift delayDrift, detuneDrift;
        };

        static constexpr int kMaxCorr = 512;     // longest correlation window, decimated samples
        static constexpr int kMaxSeg  = 1536;    // longest candidate segment, decimated samples

        float readAt (float delaySamples) const noexcept;
        float sampleAt (int delay) const noexcept;
        float decAt (int index) const noexcept;
        float clampTap (float delay) const noexcept { return juce::jlimit (2.0f, maxTapSamples, delay); }

        void  stepVoice (Voice& v, float& sumL, float& sumR) noexcept;
        void  maybeJump (Voice& v, float eps, float centre) noexcept;
        void  startJump (Voice& v, float centre, float offLo, float offHi, float offNominal, bool search) noexcept;
        float findAlignedDelay (float dCur, float dLo, float dHi, float dNom) const noexcept;
        void  updateFilters (bool force) noexcept;

        double sr = 48000.0, invSr = 1.0 / 48000.0;
        float samplesPerMs = 48.0f;

        // full-rate delay line
        int bufLen = 0, mask = 0, writeIdx = 0;
        std::vector<float> buffer;
        float maxTapSamples = 1.0f;

        // decimated copy of the input (box-filtered) used by the alignment search
        int decM = 4, decLen = 0, decCount = 0, corrLenDec = 0, corrLenFull = 0;
        unsigned decMask = 0u, decWrite = 0u;
        float decInvM = 0.25f, decAcc = 0.0f;
        std::vector<float> dec;

        // cross-fade / range constants, in samples
        float fadeInc = 0.01f, maxRangeSamples = 0.0f, minSpanSamples = 0.0f, minMarginSamples = 0.0f, staticLimitSamples = 0.0f;

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
    /** |1 - ratio| below this (about 0.9 cent) the double is treated as "not detuned". */
    inline constexpr float kStaticEps = 5.0e-4f;
    /** Pitch error allowed while an offset left over from a detuned period is walked back to zero
        (samples per sample, about 0.5 cent). */
    inline constexpr float kRecentreSlew = 3.0e-4f;

    inline void Doubler::prepare (double newSr)
    {
        sr = juce::jlimit (8000.0, 384000.0, newSr);
        invSr = 1.0 / sr;
        samplesPerMs = (float) (sr * 0.001);

        // History needed: longest tap (40 ms + 14 ms range) plus the correlation window (16 ms)
        const int need = (int) std::ceil (0.1 * sr) + 64;
        int len = 1;
        while (len < need) len <<= 1;
        bufLen = len;
        mask = len - 1;
        buffer.assign ((size_t) len, 0.0f);
        writeIdx = 0;
        maxTapSamples = (float) (len - 8);

        decM = juce::jmax (1, (int) std::lround (sr / 12000.0));
        decInvM = 1.0f / (float) decM;
        const double decRate = sr / (double) decM;
        const int decNeed = (int) std::ceil (0.1 * decRate) + 32;
        int dl = 1;
        while (dl < decNeed) dl <<= 1;
        decLen = dl;
        decMask = (unsigned) (dl - 1);
        dec.assign ((size_t) dl, 0.0f);
        corrLenDec = juce::jlimit (8, kMaxCorr, (int) std::lround (0.016 * decRate));
        corrLenFull = juce::jmax (16, (int) std::lround (0.006 * sr));

        fadeInc = (float) (1.0 / juce::jmax (16.0, 0.010 * sr));   // 10 ms cross-fade
        maxRangeSamples = (float) (0.014 * sr);                      // offsets stay within +-14 ms
        minSpanSamples = (float) (0.016 * sr);                       // the offset range is at least 16 ms wide
        minMarginSamples = (float) (0.002 * sr);                     // a tap never gets closer than 2 ms
        staticLimitSamples = (float) (0.003 * sr);

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
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        std::fill (dec.begin(), dec.end(), 0.0f);
        writeIdx = 0;
        decWrite = 0u;
        decCount = 0;
        decAcc = 0.0f;
        for (int k = 0; k < kMaxVoices; ++k)
        {
            Voice& v = voices[(size_t) k];
            v.delayMs = v.targetDelayMs;
            v.detuneCt = v.targetDetuneCt;
            v.pan = v.targetPan;
            v.gainL = v.targetGainL;
            v.gainR = v.targetGainR;
            v.offA = 0.0f;
            v.offB = 0.0f;
            v.fadePos = 0.0f;
            v.fading = false;
            v.delayDrift.start (0x1234567u + 0x9E3779B9u * (std::uint32_t) (k + 1));
            v.detuneDrift.start (0x7654321u + 0x85EBCA6Bu * (std::uint32_t) (k + 1));
        }
        normSm = normTarget;
        hpLog = hpTargetLog;
        lpLog = lpTargetLog;
        hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset();
        filterCounter = 0u;
        updateFilters (true);
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
            v.targetDetuneCt = juce::jlimit (-100.0f, 100.0f, s.detuneCt * kDetuneMul[k]);
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

    /** Cubic (Catmull-Rom) read `delaySamples` (>= 2) behind the most recent input sample. Linear
        interpolation would low-pass the top octave by a varying amount as the fractional delay of a
        detuned tap sweeps, which shows up as a roughness at ~ the detune rate. */
    inline float Doubler::readAt (float delaySamples) const noexcept
    {
        const float pos = (float) (writeIdx + bufLen - 1) - delaySamples;   // positive: delay < bufLen
        const int i0 = (int) pos;
        const float x = pos - (float) i0;
        const float y0 = buffer[(size_t) ((i0 - 1) & mask)];
        const float y1 = buffer[(size_t) (i0 & mask)];
        const float y2 = buffer[(size_t) ((i0 + 1) & mask)];
        const float y3 = buffer[(size_t) ((i0 + 2) & mask)];
        return y1 + 0.5f * x * (y2 - y0 + x * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3 + x * (3.0f * (y1 - y2) + y3 - y0)));
    }

    /** Input sample `delay` samples ago (0 = the most recent one). */
    inline float Doubler::sampleAt (int delay) const noexcept
    {
        return buffer[(size_t) ((writeIdx + bufLen - 1 - delay) & mask)];
    }

    /** Decimated sample number `index` counted back from the newest complete one. */
    inline float Doubler::decAt (int index) const noexcept
    {
        return dec[(size_t) ((decWrite - 1u - (unsigned) index) & decMask)];
    }

    inline void Doubler::process (float x, float& outL, float& outR) noexcept
    {
        if (bufLen == 0) { outL = 0.0f; outR = 0.0f; return; }   // not prepared

        buffer[(size_t) writeIdx] = x;
        writeIdx = (writeIdx + 1) & mask;

        decAcc += x;
        if (++decCount >= decM)
        {
            dec[(size_t) (decWrite & decMask)] = decAcc * decInvM;
            ++decWrite;
            decAcc = 0.0f;
            decCount = 0;
        }

        float sumL = 0.0f, sumR = 0.0f;
        for (auto& v : voices)
            stepVoice (v, sumL, sumR);

        normSm += (normTarget - normSm) * coefGain;
        sumL *= normSm;
        sumR *= normSm;

        if ((++filterCounter & 31u) == 0u) updateFilters (false);
        outL = lpL.process (hpL.process (sumL));
        outR = lpR.process (hpR.process (sumR));
    }

    /** Advances one double by one sample and adds its panned output to the wet bus. */
    inline void Doubler::stepVoice (Voice& v, float& sumL, float& sumR) noexcept
    {
        const int index = (int) (&v - voices.data());
        if (index >= activeVoices && std::abs (v.gainL) < 1.0e-6f && std::abs (v.gainR) < 1.0e-6f)
            return;   // switched off and faded out: nothing to compute

        // Smoothed live values: targets plus slow random modulation (Humanize)
        const float dMod = v.delayDrift.next (driftHz, invSr);
        const float cMod = v.detuneDrift.next (driftHz, invSr);
        const float delayGoal = juce::jlimit (kMinDelayMs, kMaxDelayMs, v.targetDelayMs + depthDelayMs * dMod);
        v.delayMs  += (delayGoal - v.delayMs) * coefDelay;
        v.detuneCt += (v.targetDetuneCt + depthDetuneCt * cMod - v.detuneCt) * coefDetune;
        v.pan      += (v.targetPan - v.pan) * coefPan;
        v.gainL    += (v.targetGainL - v.gainL) * coefGain;
        v.gainR    += (v.targetGainR - v.gainR) * coefGain;

        // A tap whose delay grows by eps = 1 - ratio per sample plays the input at `ratio` speed.
        const float eps = 1.0f - std::exp2 (v.detuneCt * (1.0f / 1200.0f));
        const float centre = v.delayMs * samplesPerMs;
        v.offA += eps;
        if (v.fading) v.offB += eps;

        if (! v.fading)
        {
            if (std::abs (eps) < kStaticEps)
            {
                // Not detuned: walk any leftover offset back to zero so that the delay is the one asked for
                const float step = juce::jmin (std::abs (v.offA), kRecentreSlew);
                v.offA -= v.offA > 0.0f ? step : -step;
            }
            maybeJump (v, eps, centre);
        }

        float y = readAt (clampTap (centre + v.offA));
        if (v.fading)
        {
            v.fadePos += fadeInc;
            if (v.fadePos >= 1.0f)
            {
                y = readAt (clampTap (centre + v.offB));
                v.offA = v.offB;
                v.fading = false;
            }
            else
            {
                const float p = v.fadePos;
                const float w = p * p * (3.0f - 2.0f * p);   // amplitude-complementary: fades sum to 1
                y += (readAt (clampTap (centre + v.offB)) - y) * w;
            }
        }

        sumL += v.gainL * y;
        sumR += v.gainR * y;
    }

    /** Decides whether the audible tap has drifted far enough to be handed over to a new tap.
        The tap offset lives in [-lowCap, +highCap] around the voice delay. The range is at least
        16 ms wide so that a jump can always span a whole pitch period of a low male voice; for
        delays below ~10 ms that means the average delay of a detuned double sits near 10 ms. */
    inline void Doubler::maybeJump (Voice& v, float eps, float centre) noexcept
    {
        const float lowCap = juce::jmin (maxRangeSamples, centre - minMarginSamples);   // centre >= 5 ms
        if (std::abs (eps) < kStaticEps)
        {
            const float limit = juce::jmin (staticLimitSamples, lowCap);
            if (std::abs (v.offA) > limit)
                startJump (v, centre, -limit, limit, 0.0f, true);
            return;
        }

        const float lo = -lowCap;
        const float hi = juce::jmax (lowCap, minSpanSamples - lowCap);
        const float width = hi - lo, mid = 0.5f * (hi + lo);
        const float sgn = eps > 0.0f ? 1.0f : -1.0f;
        const float trigger = mid + sgn * 0.25f * width;      // the tap has travelled half-way across the range
        const float nominal = mid - sgn * 0.25f * width;      // ... and comes back to the opposite quarter point

        if (sgn * (v.offA - trigger) >= 0.0f)
        {
            const float minJump = 0.15f * width;
            if (sgn > 0.0f) startJump (v, centre, lo, trigger - minJump, nominal, true);
            else            startJump (v, centre, trigger + minJump, hi, nominal, true);
        }
        else if (v.offA > hi + 2.0f || v.offA < lo - 2.0f)
            startJump (v, centre, lo, hi, nominal, false);   // out of range (the delay was changed fast)
    }

    inline void Doubler::startJump (Voice& v, float centre, float offLo, float offHi, float offNominal, bool search) noexcept
    {
        float newOff = offNominal;
        if (search)
        {
            const float dCur = clampTap (centre + v.offA);
            newOff = findAlignedDelay (dCur, centre + offLo, centre + offHi, centre + offNominal) - centre;
        }
        v.offB = juce::jlimit (offLo, offHi, newOff);
        v.fadePos = 0.0f;
        v.fading = true;
    }

    /** Looks for the delay in [dLo, dHi] whose recent past (the 16 ms before the tap) looks most like
        the recent past of a tap at `dCur`. Normalised correlation on the decimated copy, then a
        full-rate refinement with parabolic interpolation. Ties and noise-like input fall back to
        `dNom`. Returns `dNom` when there is nothing to align (silence). */
    inline float Doubler::findAlignedDelay (float dCur, float dLo, float dHi, float dNom) const noexcept
    {
        const int Ld = corrLenDec;
        const float origin = (float) decCount + 0.5f * (float) (decM - 1);   // delay of decimated sample 0
        auto toIndex = [&] (float d)
        {
            return juce::jlimit (0, decLen - Ld - 2, (int) std::lround ((d - origin) * decInvM));
        };

        const int ic = toIndex (dCur);
        const int iLo = toIndex (dLo);
        const int iHi = juce::jlimit (iLo, iLo + kMaxSeg - Ld - 1, toIndex (dHi));
        const int iNom = toIndex (dNom);

        // Contiguous copies keep the inner loops simple and fast
        float cur[kMaxCorr], seg[kMaxSeg];
        for (int k = 0; k < Ld; ++k) cur[k] = decAt (ic + k);
        const int segLen = iHi - iLo + Ld;
        for (int k = 0; k < segLen; ++k) seg[k] = decAt (iLo + k);

        double ec = 0.0;
        for (int k = 0; k < Ld; ++k) ec += (double) cur[k] * (double) cur[k];
        if (ec < 1.0e-8) return dNom;

        const float halfSpan = (float) juce::jmax (1, (iHi - iLo) / 2);
        float bestScore = -1.0e9f, bestRho = 0.0f;
        int bestIndex = iNom;
        for (int i = iLo; i <= iHi; ++i)
        {
            const float* s = seg + (i - iLo);
            double c = 0.0, e = 0.0;
            for (int k = 0; k < Ld; ++k) { c += (double) cur[k] * (double) s[k]; e += (double) s[k] * (double) s[k]; }
            const float rho = (float) (c / std::sqrt (ec * e + 1.0e-18));
            const float score = rho - 0.15f * std::abs ((float) (i - iNom)) / halfSpan;   // slight bias to the nominal delay
            if (score > bestScore) { bestScore = score; bestRho = rho; bestIndex = i; }
        }
        if (bestRho < 0.2f) return dNom;   // unvoiced / noise: no meaningful alignment

        // Full-rate refinement around the best decimated candidate
        const int dCurI = (int) std::lround (dCur);
        const float frac = dCur - (float) dCurI;
        const int Lf = corrLenFull;
        const int dMin = 1, dMax = bufLen - Lf - 8;
        const int d0 = juce::jlimit (dMin + decM, dMax - decM, (int) std::lround (origin + (float) bestIndex * (float) decM));

        double ecf = 0.0;
        for (int k = 0; k < Lf; ++k) { const double a = (double) sampleAt (juce::jlimit (0, bufLen - 1, dCurI + k)); ecf += a * a; }
        if (ecf < 1.0e-8) return (float) d0 + frac;

        float rho[2 * 32 + 3] {};
        const int span = juce::jmin (decM, 32);
        int bestK = 0;
        for (int j = -span; j <= span; ++j)
        {
            double c = 0.0, e = 0.0;
            for (int k = 0; k < Lf; ++k)
            {
                const double a = (double) sampleAt (dCurI + k), b = (double) sampleAt (d0 + j + k);
                c += a * b;
                e += b * b;
            }
            rho[j + span] = (float) (c / std::sqrt (ecf * e + 1.0e-18));
            if (rho[j + span] > rho[bestK]) bestK = j + span;
        }

        float delta = 0.0f;
        if (bestK > 0 && bestK < 2 * span)
        {
            const float a = rho[bestK - 1], b = rho[bestK], g = rho[bestK + 1];
            const float denom = a - 2.0f * b + g;
            if (denom < -1.0e-6f) delta = juce::jlimit (-0.5f, 0.5f, 0.5f * (a - g) / denom);
        }
        return (float) (d0 + bestK - span) + delta + frac;
    }

    inline void Doubler::updateFilters (bool force) noexcept
    {
        const float hpDiff = hpTargetLog - hpLog, lpDiff = lpTargetLog - lpLog;
        if (! force && std::abs (hpDiff) < 1.0e-4f && std::abs (lpDiff) < 1.0e-4f)
            return;   // settled: keep the current coefficients

        hpLog += hpDiff * coefFilter;
        lpLog += lpDiff * coefFilter;
        const double hz = std::exp2 ((double) hpLog);
        const double lz = juce::jmin (std::exp2 ((double) lpLog), 0.45 * sr);
        const auto hp = makeHighPass (sr, hz, 0.7071);
        const auto lp = makeLowPass (sr, lz, 0.7071);
        hpL.setCoeffs (hp); hpR.setCoeffs (hp);
        lpL.setCoeffs (lp); lpR.setCoeffs (lp);
    }
}
