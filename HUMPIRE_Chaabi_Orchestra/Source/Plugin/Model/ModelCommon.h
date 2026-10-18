#pragma once
// ============================================================================
//  Shared helpers for the instrument renderers (pure C++17, std only, header only).
//  Everything here is deterministic and re-entrant (no static mutable state).
//  Author contract: see ModelLibrary.h.  All functions live in humpire::model.
//
//  Contents
//    conversions, Rng (PCG32), OnePole, Biquad, DcBlocker, Resonator (modal),
//    DelayLine (Lagrange-3), KarplusString (tuned waveguide string),
//    level helpers (peak / loudness / normalise), fades, tail trimming, sanitising,
//    pitch detection (detectPitchHz) + pitch correction by resampling (correctPitch),
//    loop helpers (findLoopPoints / crossfadeLoop / finishLoop),
//    key spans (makeKeySpans) and alias zones (addKeyAliases), ProgressTracker, naming.
// ============================================================================
#include "ModelLibrary.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

namespace humpire::model
{
inline constexpr double kPi = 3.14159265358979323846;
inline constexpr double kTwoPi = 2.0 * kPi;

// ------------------------------------------------------------------------------ conversions
inline double midiToHz (double note) { return 440.0 * std::pow (2.0, (note - 69.0) / 12.0); }
inline double hzToMidi (double hz) { return 69.0 + 12.0 * std::log2 (hz / 440.0); }
inline double dbToGain (double db) { return std::pow (10.0, db / 20.0); }
inline double gainToDb (double g) { return 20.0 * std::log10 (g > 1.0e-12 ? g : 1.0e-12); }
inline double centsBetween (double hz, double refHz) { return 1200.0 * std::log2 (hz / refHz); }
inline double clampd (double v, double lo, double hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline int clampi (int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
inline double lerp (double a, double b, double t) { return a + (b - a) * t; }
inline double smoothstep (double t) { t = clampd (t, 0.0, 1.0); return t * t * (3.0 - 2.0 * t); }
inline int secondsToFrames (double seconds, double sr) { return (int) std::lround (seconds * sr); }

// ------------------------------------------------------------------------------ deterministic RNG
// PCG32.  Seed from the instrument / zone index, never from time or addresses.
class Rng
{
public:
    explicit Rng (std::uint64_t seed = 1u) { reseed (seed); }

    void reseed (std::uint64_t seed)
    {
        state = 0u;
        inc = (mix64 (seed ^ 0x9E3779B97F4A7C15ull) << 1u) | 1u;
        next();
        state += mix64 (seed);
        next();
        haveSpare = false;
    }

    // Stream unique to (instrument, zone, salt).
    static Rng forZone (int instrument, int zone, int salt = 0)
    {
        return Rng (mix64 (((std::uint64_t) (instrument + 1) * 0x100000001B3ull) ^ mix64 ((std::uint64_t) (zone + 1) * 0x9E3779B1ull + (std::uint64_t) (salt + 7))));
    }

    std::uint32_t next()
    {
        const std::uint64_t old = state;
        state = old * 6364136223846793005ull + inc;
        const std::uint32_t xs = (std::uint32_t) (((old >> 18u) ^ old) >> 27u);
        const std::uint32_t rot = (std::uint32_t) (old >> 59u);
        return (xs >> rot) | (xs << ((0u - rot) & 31u));
    }
    double uniform() { return (double) (next() >> 8) * (1.0 / 16777216.0); }          // [0,1)
    double bipolar() { return uniform() * 2.0 - 1.0; }                                // [-1,1)
    float uniformF() { return (float) uniform(); }
    float bipolarF() { return (float) bipolar(); }
    double range (double lo, double hi) { return lo + (hi - lo) * uniform(); }
    int rangeInt (int lo, int hiInclusive) { return lo + (int) (next() % (std::uint32_t) (hiInclusive - lo + 1)); }
    double gaussian()                                                                  // N(0,1), Box-Muller
    {
        if (haveSpare) { haveSpare = false; return spare; }
        double u1 = uniform();
        const double u2 = uniform();
        if (u1 < 1.0e-12) u1 = 1.0e-12;
        const double r = std::sqrt (-2.0 * std::log (u1));
        spare = r * std::sin (kTwoPi * u2);
        haveSpare = true;
        return r * std::cos (kTwoPi * u2);
    }

    static std::uint64_t mix64 (std::uint64_t z)
    {
        z += 0x9E3779B97F4A7C15ull;
        z = (z ^ (z >> 30u)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27u)) * 0x94D049BB133111EBull;
        return z ^ (z >> 31u);
    }

private:
    std::uint64_t state = 0u, inc = 1u;
    double spare = 0.0;
    bool haveSpare = false;
};

// ------------------------------------------------------------------------------ filters
// One-pole low-pass (y += (1-a)(x-y)).  highpass() = x - lowpass(x).
struct OnePole
{
    double a = 0.0, z = 0.0;
    void setLowpass (double fc, double sr) { a = std::exp (-kTwoPi * clampd (fc, 1.0, sr * 0.49) / sr); }
    void reset() { z = 0.0; }
    double lowpass (double x) { z = (1.0 - a) * x + a * z; return z; }
    double highpass (double x) { return x - lowpass (x); }
};

// RBJ audio-EQ-cookbook biquad, transposed direct form II, double precision.
struct Biquad
{
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0, z1 = 0.0, z2 = 0.0;

    void reset() { z1 = z2 = 0.0; }
    double process (double x)
    {
        const double y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void processBuffer (std::vector<float>& v) { for (auto& s : v) s = (float) process ((double) s); }

    void set (double nb0, double nb1, double nb2, double a0, double na1, double na2)
    {
        b0 = nb0 / a0; b1 = nb1 / a0; b2 = nb2 / a0; a1 = na1 / a0; a2 = na2 / a0;
    }
    static Biquad lowpass (double fc, double q, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        f.set ((1 - c) / 2, 1 - c, (1 - c) / 2, 1 + al, -2 * c, 1 - al); return f;
    }
    static Biquad highpass (double fc, double q, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        f.set ((1 + c) / 2, -(1 + c), (1 + c) / 2, 1 + al, -2 * c, 1 - al); return f;
    }
    // Band-pass, constant 0 dB peak gain.
    static Biquad bandpass (double fc, double q, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        f.set (al, 0, -al, 1 + al, -2 * c, 1 - al); return f;
    }
    static Biquad notch (double fc, double q, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        f.set (1, -2 * c, 1, 1 + al, -2 * c, 1 - al); return f;
    }
    static Biquad peaking (double fc, double q, double gainDb, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        const double A = std::pow (10.0, gainDb / 40.0);
        f.set (1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A); return f;
    }
    static Biquad lowShelf (double fc, double gainDb, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, 0.7071, sr, w, c, s, al);
        const double A = std::pow (10.0, gainDb / 40.0), t = 2.0 * std::sqrt (A) * al;
        f.set (A * ((A + 1) - (A - 1) * c + t), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - t),
               (A + 1) + (A - 1) * c + t, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - t); return f;
    }
    static Biquad highShelf (double fc, double gainDb, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, 0.7071, sr, w, c, s, al);
        const double A = std::pow (10.0, gainDb / 40.0), t = 2.0 * std::sqrt (A) * al;
        f.set (A * ((A + 1) + (A - 1) * c + t), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - t),
               (A + 1) - (A - 1) * c + t, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - t); return f;
    }
    static Biquad allpass (double fc, double q, double sr)
    {
        Biquad f; double w, c, s, al; prep (fc, q, sr, w, c, s, al);
        f.set (1 - al, -2 * c, 1 + al, 1 + al, -2 * c, 1 - al); return f;
    }

private:
    static void prep (double fc, double q, double sr, double& w, double& c, double& s, double& al)
    {
        fc = clampd (fc, 5.0, sr * 0.499);
        q = q < 0.05 ? 0.05 : q;
        w = kTwoPi * fc / sr; c = std::cos (w); s = std::sin (w); al = s / (2.0 * q);
    }
};

// First-order DC blocker  y[n] = x[n] - x[n-1] + R y[n-1]  (R ~ 0.995 -> corner ~ 35 Hz at 44.1 kHz).
struct DcBlocker
{
    double r = 0.995, x1 = 0.0, y1 = 0.0;
    explicit DcBlocker (double fc = 10.0, double sr = 44100.0) { r = clampd (1.0 - kTwoPi * fc / sr, 0.9, 0.99999); }
    double process (double x) { const double y = x - x1 + r * y1; x1 = x; y1 = y; return y; }
};

// Two-pole resonator (one vibrating mode).  Impulse response of unit amplitude:
//   y[n] = r^n sin(w (n+1))  with  r = 10^(-3 / (t60 * sr)).  Excite with an impulse / short burst.
struct Resonator
{
    double b0 = 0.0, a1 = 0.0, a2 = 0.0, y1 = 0.0, y2 = 0.0;
    void set (double freqHz, double t60Sec, double sr)
    {
        freqHz = clampd (freqHz, 1.0, sr * 0.49);
        const double r = std::pow (10.0, -3.0 / (std::max (t60Sec, 1.0e-4) * sr));
        const double w = kTwoPi * freqHz / sr;
        a1 = 2.0 * r * std::cos (w);
        a2 = -r * r;
        b0 = std::sin (w);
        y1 = y2 = 0.0;
    }
    double process (double x)
    {
        const double y = b0 * x + a1 * y1 + a2 * y2;
        y2 = y1; y1 = y;
        return y;
    }
};

// ------------------------------------------------------------------------------ delay line
// Power-of-two ring buffer with integer taps and 3rd-order Lagrange fractional read.
class DelayLine
{
public:
    void init (int maxDelaySamples)
    {
        std::size_t n = 8;
        while (n < (std::size_t) std::max (maxDelaySamples, 4) + 8) n <<= 1;
        buf.assign (n, 0.f);
        mask = n - 1;
        pos = 0;
    }
    void clear() { std::fill (buf.begin(), buf.end(), 0.f); pos = 0; }
    void push (float x) { pos = (pos + 1) & mask; buf[pos] = x; }       // x becomes tap(0)
    float tap (int delay) const { return buf[(pos - (std::size_t) delay) & mask]; }
    // Fractional read, delay >= 1.0 (in samples; tap(0) is the newest sample).
    float read (double delay) const
    {
        const int i = (int) delay;
        const double u = (delay - (double) i) + 1.0;           // position inside the 4 nodes [i-1 .. i+2], u in [1,2)
        const double y0 = tap (i - 1), y1 = tap (i), y2 = tap (i + 1), y3 = tap (i + 2);
        const double h0 = -(u - 1.0) * (u - 2.0) * (u - 3.0) / 6.0;
        const double h1 = u * (u - 2.0) * (u - 3.0) / 2.0;
        const double h2 = -u * (u - 1.0) * (u - 3.0) / 2.0;
        const double h3 = u * (u - 1.0) * (u - 2.0) / 6.0;
        return (float) (h0 * y0 + h1 * y1 + h2 * y2 + h3 * y3);
    }
    std::size_t capacity() const { return buf.size(); }

private:
    std::vector<float> buf;
    std::size_t mask = 0, pos = 0;
};

// ------------------------------------------------------------------------------ waveguide string
// Tuned Karplus-Strong / digital-waveguide string:
//   loop = integer delay + 2-tap damping filter [1-b, b] + loop gain + first-order all-pass (fractional delay).
// The total loop phase delay at f0 is solved numerically, so the pitch is exact (within ~1 cent) for any f0 < sr/6.
class KarplusString
{
public:
    // freqHz: fundamental.  t60: seconds for the fundamental to decay by 60 dB.
    // bright 0..1: 0 = dull (strong averaging filter), 1 = bright (almost no loop filtering).
    void init (double freqHz, double sampleRate, double t60, double bright)
    {
        sr = sampleRate;
        const double w0 = kTwoPi * freqHz / sr;
        const double T = sr / freqHz;
        b = clampd (0.5 * (1.0 - clampd (bright, 0.0, 1.0)), 0.02, 0.5);
        const double firDelay = std::atan2 (b * std::sin (w0), (1.0 - b) + b * std::cos (w0)) / w0;
        N = std::max (2, (int) std::floor (T - firDelay - 1.0));
        const double rem = T - firDelay - (double) N;           // wanted all-pass phase delay at w0, ~[1,2)
        // bisection on eta in (-0.9, 0.9): phase delay of (eta + z^-1)/(1 + eta z^-1) is monotonic in eta
        double lo = -0.9, hi = 0.9;
        for (int it = 0; it < 40; ++it)
        {
            const double mid = 0.5 * (lo + hi);
            if (allpassPhaseDelay (mid, w0) > rem) lo = mid; else hi = mid;   // larger eta -> smaller delay
        }
        eta = 0.5 * (lo + hi);
        const double magFir = std::sqrt ((1.0 - b) * (1.0 - b) + b * b + 2.0 * b * (1.0 - b) * std::cos (w0));
        gain = clampd (std::pow (10.0, -3.0 / (std::max (t60, 0.01) * freqHz)) / magFir, 0.0, 0.99995);
        line.init (N + 4);
        line.clear();
        x1 = 0.0; apX1 = 0.0; apY1 = 0.0;
    }
    int loopDelay() const { return N; }

    // One sample.  `excitation` is added at the loop input.  Returns the string output (loop signal).
    float tick (float excitation = 0.f)
    {
        const double fromLine = (double) line.tap (N - 1);                 // oldest sample of the delay (N samples)
        const double fir = (1.0 - b) * fromLine + b * x1;
        x1 = fromLine;
        const double g = fir * gain;
        // all-pass: y = eta*x + x1 - eta*y1
        const double ap = eta * g + apX1 - eta * apY1;
        apX1 = g; apY1 = ap;
        line.push ((float) (ap + (double) excitation));
        return (float) ap;
    }

    // Shaped noise burst of length `len`: low-passed by `hardness` (0 soft/dark .. 1 hard/bright),
    // with a comb for the plucking position (0.05 .. 0.5 of the string length).
    static std::vector<float> excitation (Rng& rng, int len, double hardness, double pickPos, double sampleRate)
    {
        std::vector<float> e ((std::size_t) std::max (len, 4));
        OnePole lp;
        lp.setLowpass (200.0 + 9000.0 * hardness * hardness, sampleRate);
        for (auto& v : e) v = (float) lp.lowpass (rng.bipolar());
        const int d = std::max (1, (int) std::lround (pickPos * (double) e.size()));
        std::vector<float> c (e.size());
        for (std::size_t i = 0; i < e.size(); ++i)
            c[i] = e[i] - (i >= (std::size_t) d ? e[i - (std::size_t) d] : 0.f);
        float mean = 0.f;
        for (auto v : c) mean += v;
        mean /= (float) c.size();
        for (auto& v : c) v -= mean;                                   // no DC in the excitation
        return c;
    }

private:
    static double allpassPhaseDelay (double eta, double w)
    {
        // A(z) = (eta + z^-1) / (1 + eta z^-1);  phase delay = -arg(A)/w
        const double nr = eta + std::cos (w), ni = -std::sin (w);
        const double dr = 1.0 + eta * std::cos (w), di = -eta * std::sin (w);
        double arg = std::atan2 (ni, nr) - std::atan2 (di, dr);
        while (arg > 0.0) arg -= kTwoPi;
        while (arg < -kTwoPi) arg += kTwoPi;
        return -arg / w;
    }

    double sr = 44100.0, b = 0.3, eta = 0.0, gain = 0.99, x1 = 0.0, apX1 = 0.0, apY1 = 0.0;
    int N = 16;
    DelayLine line;
};

// ------------------------------------------------------------------------------ level helpers
inline float peakOf (const float* x, std::size_t n)
{
    float p = 0.f;
    for (std::size_t i = 0; i < n; ++i) p = std::max (p, std::fabs (x[i]));
    return p;
}
inline float peakOf (const std::vector<float>& v) { return peakOf (v.data(), v.size()); }

// Level targets of the library (see "Level conventions" in ModelLibrary.h).
inline constexpr double kHardLoudnessDb = -14.0;     // hard velocity layer (vel 81-127) of every articulation
inline constexpr double kSoftLoudnessDb = -20.0;     // soft layer (vel 1-80)
inline constexpr double kPeakCeilingDb = -2.5;       // never exceed this peak when normalising

// "Loudness" used by the analysis gate: highest RMS (dBFS) over any 50 ms window (hop = window/2).
inline double loudnessDb (const float* x, std::size_t n, double sr, double windowSec = 0.05)
{
    const std::size_t w = std::max<std::size_t> (16, (std::size_t) (windowSec * sr));
    if (n == 0) return -200.0;
    double best = 0.0;
    if (n <= w)
    {
        for (std::size_t i = 0; i < n; ++i) best += (double) x[i] * (double) x[i];
        best /= (double) w;                                       // short files: pad with silence
    }
    else
    {
        for (std::size_t s = 0; s + w <= n; s += w / 2)
        {
            double e = 0.0;
            for (std::size_t i = 0; i < w; ++i) e += (double) x[s + i] * (double) x[s + i];
            best = std::max (best, e / (double) w);
        }
    }
    return 10.0 * std::log10 (best > 1.0e-20 ? best : 1.0e-20);
}
inline double loudnessDb (const std::vector<float>& v, double sr) { return loudnessDb (v.data(), v.size(), sr); }

inline void applyGain (std::vector<float>& v, double gain) { for (auto& s : v) s = (float) ((double) s * gain); }

// Scales so the peak equals targetDb.  Returns the gain applied (linear).
inline double normalisePeak (std::vector<float>& v, double targetDb)
{
    const double p = peakOf (v);
    if (p < 1.0e-9) return 1.0;
    const double g = dbToGain (targetDb) / p;
    applyGain (v, g);
    return g;
}

// Scales to a loudness target (see loudnessDb) but never lets the peak exceed peakCeilingDb.
inline double normaliseLoudness (std::vector<float>& v, double sr, double targetLoudnessDb, double peakCeilingDb = -2.0)
{
    const double l = loudnessDb (v, sr);
    const double p = peakOf (v);
    if (p < 1.0e-9) return 1.0;
    double g = dbToGain (targetLoudnessDb - l);
    const double maxG = dbToGain (peakCeilingDb) / p;
    if (g > maxG) g = maxG;
    applyGain (v, g);
    return g;
}

// Removes NaN / Inf and (optionally) the mean (DC offset).  Does NOT clip: scale-invariant, call it on the raw
// render and normalise afterwards (normalisePeak / normaliseLoudness).
inline void sanitize (std::vector<float>& v, bool removeDc = true)
{
    double mean = 0.0;
    for (auto& s : v) if (! std::isfinite (s)) s = 0.f;
    if (removeDc && ! v.empty())
    {
        for (auto s : v) mean += (double) s;
        mean /= (double) v.size();
    }
    for (auto& s : v) s = (float) ((double) s - mean);
}

// Hard limiter of last resort (never use it as a normaliser: clipping is audible).
inline void hardLimit (std::vector<float>& v, float limit = 0.999f)
{
    for (auto& s : v) s = std::max (-limit, std::min (limit, s));
}

// Raised-cosine fades (frames at the start / at the end of the buffer).
inline void fadeIn (std::vector<float>& v, int frames)
{
    const int n = std::min<int> (frames, (int) v.size());
    for (int i = 0; i < n; ++i)
        v[(std::size_t) i] = (float) ((double) v[(std::size_t) i] * (0.5 - 0.5 * std::cos (kPi * (double) i / (double) n)));
}
inline void fadeOut (std::vector<float>& v, int frames)
{
    const int total = (int) v.size();
    const int n = std::min (frames, total);
    for (int i = 0; i < n; ++i)
        v[(std::size_t) (total - n + i)] = (float) ((double) v[(std::size_t) (total - n + i)] * (0.5 + 0.5 * std::cos (kPi * (double) (i + 1) / (double) n)));
    if (! v.empty()) v.back() = 0.f;
}

// Cuts trailing samples quieter than thresholdDb (relative to the file peak, short-term) but keeps
// at least minFrames, then fades out over fadeFrames.  Use for decaying / one-shot material.
inline void trimTail (std::vector<float>& v, double thresholdDb, int minFrames, int fadeFrames)
{
    const double thr = (double) peakOf (v) * dbToGain (thresholdDb);
    std::size_t last = v.size();
    const std::size_t win = 64;
    while (last > (std::size_t) std::max (minFrames, 1))
    {
        const std::size_t s = last > win ? last - win : 0;
        float p = 0.f;
        for (std::size_t i = s; i < last; ++i) p = std::max (p, std::fabs (v[i]));
        if ((double) p > thr) break;
        last = s;
    }
    v.resize(std::max<std::size_t> (last, (std::size_t) std::max (std::min (minFrames, (int) v.size()), 1)));
    fadeOut (v, fadeFrames);
}

// Standard finishing for non-looped material (scale-invariant, call BEFORE normalising): removes NaN/DC, hard zero
// at the first frame (0.4 ms fade-in), optional tail trimming (-75 dB) and a final fade-out (>= 8 ms).
inline void finishOneShot (std::vector<float>& v, double sr, bool trim = true, double fadeOutSec = 0.012)
{
    sanitize (v, true);
    fadeIn (v, std::max (8, secondsToFrames (0.0004, sr)));
    if (trim) trimTail (v, -75.0, (int) (0.05 * sr), std::max (16, secondsToFrames (fadeOutSec, sr)));
    else fadeOut (v, std::max (16, secondsToFrames (fadeOutSec, sr)));
}

// Linear / equal-power crossfade of two buffers into dst (dst may alias a).
inline void crossfade (std::vector<float>& dst, const std::vector<float>& a, const std::vector<float>& b, bool equalPower)
{
    const std::size_t n = std::min (a.size(), b.size());
    dst.resize (n);
    for (std::size_t i = 0; i < n; ++i)
    {
        const double t = n > 1 ? (double) i / (double) (n - 1) : 1.0;
        const double ga = equalPower ? std::cos (0.5 * kPi * t) : 1.0 - t, gb = equalPower ? std::sin (0.5 * kPi * t) : t;
        dst[i] = (float) (ga * (double) a[i] + gb * (double) b[i]);
    }
}

// ------------------------------------------------------------------------------ pitch tools
// Normalised-autocorrelation pitch estimate of x[0..n) near expectedHz (search +-rangeCents),
// parabolic peak interpolation.  Returns 0 when no clear periodicity (< 0.3) is found.
inline double detectPitchHz (const float* x, std::size_t n, double sr, double expectedHz, double rangeCents = 300.0)
{
    if (n < 64 || expectedHz <= 0.0) return 0.0;
    const double lagLo = sr / (expectedHz * std::pow (2.0, rangeCents / 1200.0));
    const double lagHi = sr / (expectedHz * std::pow (2.0, -rangeCents / 1200.0));
    const int iLo = std::max (2, (int) std::floor (lagLo)), iHi = (int) std::ceil (lagHi);
    if ((std::size_t) iHi * 2 >= n) return 0.0;
    const std::size_t m = n - (std::size_t) iHi - 2;
    std::vector<double> r ((std::size_t) iHi + 2, 0.0);
    double e0 = 0.0;
    for (std::size_t i = 0; i < m; ++i) e0 += (double) x[i] * (double) x[i];
    int best = -1;
    double bestV = 0.3;
    for (int l = iLo - 1; l <= iHi + 1; ++l)
    {
        double c = 0.0, e1 = 0.0;
        for (std::size_t i = 0; i < m; ++i) { c += (double) x[i] * (double) x[i + (std::size_t) l]; e1 += (double) x[i + (std::size_t) l] * (double) x[i + (std::size_t) l]; }
        r[(std::size_t) l] = c / std::sqrt (std::max (e0 * e1, 1.0e-30));
    }
    for (int l = iLo; l <= iHi; ++l)
        if (r[(std::size_t) l] > bestV && r[(std::size_t) l] >= r[(std::size_t) l - 1] && r[(std::size_t) l] >= r[(std::size_t) l + 1]) { bestV = r[(std::size_t) l]; best = l; }
    if (best < 0) return 0.0;
    const double a = r[(std::size_t) best - 1], b = r[(std::size_t) best], c = r[(std::size_t) best + 1];
    const double den = a - 2.0 * b + c;
    const double off = std::fabs (den) > 1.0e-12 ? 0.5 * (a - c) / den : 0.0;
    return sr / ((double) best + off);
}

// Playback-speed resampler (Catmull-Rom).  ratio > 1: higher pitch / shorter.  Used to pitch-correct
// a rendered buffer: measure f0 with detectPitchHz, then resampleCubic (v, targetHz / measuredHz).
inline std::vector<float> resampleCubic (const std::vector<float>& in, double ratio)
{
    if (in.size() < 4 || ratio <= 0.0) return in;
    const std::size_t outN = (std::size_t) std::floor ((double) (in.size() - 1) / ratio);
    std::vector<float> out (outN);
    const int last = (int) in.size() - 1;
    for (std::size_t k = 0; k < outN; ++k)
    {
        const double p = (double) k * ratio;
        const int i = (int) p;
        const double f = p - (double) i;
        const double y0 = in[(std::size_t) std::max (i - 1, 0)], y1 = in[(std::size_t) i];
        const double y2 = in[(std::size_t) std::min (i + 1, last)], y3 = in[(std::size_t) std::min (i + 2, last)];
        out[k] = (float) (y1 + 0.5 * f * (y2 - y0 + f * (2.0 * y0 - 5.0 * y1 + 4.0 * y2 - y3 + f * (3.0 * (y1 - y2) + y3 - y0))));
    }
    return out;
}

// Measures the pitch of v[from, from+len) near targetHz (autocorrelation) and resamples the whole buffer so that it
// lands on targetHz (use it to meet the +-8 cent gate when the model is slightly out of tune).  Do it BEFORE fades,
// tail trimming and loop construction (it changes the length by a few permille).  Returns the measured error in
// cents before the correction (0 when no clear pitch was found or the error is below 0.3 cent).
inline double correctPitch (std::vector<float>& v, double sr, double targetHz, std::size_t from, std::size_t len, double rangeCents = 150.0)
{
    if (from >= v.size()) return 0.0;
    len = std::min (len, v.size() - from);
    const double f = detectPitchHz (v.data() + from, len, sr, targetHz, rangeCents);
    if (f <= 0.0) return 0.0;
    const double cents = centsBetween (f, targetHz);
    if (std::fabs (cents) < 0.3) return 0.0;
    v = resampleCubic (v, targetHz / f);
    return cents;
}

// ------------------------------------------------------------------------------ loops
struct LoopPoints { int start = -1, end = -1; double mismatch = 1.0e9; };

// Searches [searchBegin, searchEnd) for loop points with loopEnd - loopStart + 1 >= minLength such that the
// waveform just before `end` matches the waveform just before `start` (so playback end -> start is continuous).
// periodSamples: fundamental period for tonal material (loop length ends up a whole number of periods when
// possible); pass <= 0 for noise-like material.  Candidates are rising zero crossings.
// Returns true when something acceptable (normalised mismatch < 0.35) was found.
inline bool findLoopPoints (const std::vector<float>& x, double periodSamples, int searchBegin, int searchEnd,
                            int minLength, LoopPoints& out)
{
    out = LoopPoints();
    searchEnd = std::min (searchEnd, (int) x.size() - 6);
    searchBegin = std::max (searchBegin, 1);
    if (searchEnd - searchBegin < minLength + 8) return false;
    const int W = clampi (periodSamples > 0.0 ? (int) (2.0 * periodSamples) : 256, 64, 512);
    if (searchBegin < W + 2) searchBegin = W + 2;

    auto crossings = [&] (int from, int to, int maxCount)
    {
        std::vector<int> c;
        for (int i = from; i < to; ++i) if (x[(std::size_t) i - 1] <= 0.f && x[(std::size_t) i] > 0.f) c.push_back (i);
        if ((int) c.size() > maxCount)
        {
            std::vector<int> s;
            for (int k = 0; k < maxCount; ++k) s.push_back (c[(std::size_t) ((long long) k * (long long) c.size() / maxCount)]);
            c.swap (s);
        }
        return c;
    };
    const int startSpan = periodSamples > 0.0 ? (int) (6.0 * periodSamples) : 2048;
    const auto starts = crossings (searchBegin, std::min (searchEnd - minLength, searchBegin + startSpan), 48);
    const auto ends = crossings (searchBegin + minLength, searchEnd, 400);
    if (starts.empty() || ends.empty()) return false;

    double energy = 0.0;
    for (int i = searchBegin - W; i < std::min (searchEnd, searchBegin + 4 * W); ++i) energy += (double) x[(std::size_t) i] * (double) x[(std::size_t) i];
    energy /= (double) (std::min (searchEnd, searchBegin + 4 * W) - searchBegin + W);
    energy = std::max (energy, 1.0e-12);

    for (int s : starts)
        for (int e : ends)
        {
            const int len = e - s + 1;
            if (len < minLength) continue;
            double d = 0.0;
            for (int i = 0; i < W; ++i) { const double df = (double) x[(std::size_t) (e - i)] - (double) x[(std::size_t) (s - 1 - i)]; d += df * df; }
            double m = d / ((double) W * energy);
            if (periodSamples > 0.0)                       // prefer whole numbers of periods
            {
                const double k = (double) len / periodSamples;
                m += 0.02 * std::fabs (k - std::round (k));
            }
            if (m < out.mismatch) { out.mismatch = m; out.start = s; out.end = e; }
        }
    return out.start >= 0 && out.mismatch < 0.35;
}

// Blends the last `fade` frames of the loop (ending at loopEnd) towards the frames that precede loopStart,
// so that playback end -> start is exactly continuous.  Requires loopStart >= fade and loop length >= 2*fade.
// equalPower: use for noise-like material (uncorrelated), linear for tonal material.
inline bool crossfadeLoop (std::vector<float>& x, int loopStart, int loopEnd, int fade, bool equalPower = false)
{
    if (fade < 2 || loopStart < fade || loopEnd >= (int) x.size() || loopEnd - loopStart + 1 < 2 * fade) return false;
    for (int i = 0; i < fade; ++i)
    {
        const double t = (double) (i + 1) / (double) fade;
        const double gOld = equalPower ? std::cos (0.5 * kPi * t) : 1.0 - t, gNew = equalPower ? std::sin (0.5 * kPi * t) : t;
        const std::size_t dst = (std::size_t) (loopEnd - fade + 1 + i), src = (std::size_t) (loopStart - fade + i);
        x[dst] = (float) (gOld * (double) x[dst] + gNew * (double) x[src]);
    }
    return true;
}

// Finalises a looped MONO/interleaved zone: sets loop mode + points and appends 4 guard frames after loopEnd
// (copies of the frames starting at loopStart) so the engine's Hermite interpolator sees a perfectly continuous
// seam.  Any audio after loopEnd is discarded (loopEnd becomes the end of the sample + guard).
inline bool finishLoop (Zone& z, int loopStart, int loopEnd, Loop mode = Loop::sustain)
{
    const std::size_t ch = (std::size_t) std::max (z.numChannels, 1);
    const std::size_t frames = z.data.size() / ch;
    if (loopStart < 0 || loopEnd <= loopStart + 32 || (std::size_t) loopEnd >= frames) return false;
    z.data.resize (((std::size_t) loopEnd + 1 + 4) * ch);
    for (std::size_t g = 0; g < 4; ++g)
        for (std::size_t c = 0; c < ch; ++c)
            z.data[((std::size_t) loopEnd + 1 + g) * ch + c] = z.data[((std::size_t) loopStart + g) * ch + c];
    z.loop = mode;
    z.loopStart = loopStart;
    z.loopEnd = loopEnd;
    return true;
}

// One-call loop construction for a rendered MONO sustained body stored in z.data (z.numChannels == 1):
// finds loop points (tonal: pass the period in samples; noise-like: pass 0), crossfades the seam and appends the
// guard frames.  The material before `introSec` plays once (attack); the loop is at least `minLoopSec` long.
// The signal must already be in a steady state before the loop start.  Returns false when no loop fits.
inline bool makeLoopFromBody (Zone& z, double periodSamples, double introSec, double minLoopSec,
                              double fadeSec = 0.012, bool equalPower = false, Loop mode = Loop::sustain)
{
    auto& x = z.data;
    const double sr = z.sampleRate;
    const int fade = std::max (32, secondsToFrames (fadeSec, sr));
    const int searchBegin = std::max (secondsToFrames (introSec, sr), fade + 2);
    const int minLen = std::max (secondsToFrames (minLoopSec, sr), 4 * fade);
    LoopPoints lp;
    if (! findLoopPoints (x, periodSamples, searchBegin, (int) x.size() - 6, minLen, lp))
    {
        lp.start = searchBegin;                                          // fallback: plain crossfaded loop
        lp.end = (int) x.size() - 6;
    }
    if (lp.start < fade || lp.end - lp.start + 1 < 2 * fade) return false;
    crossfadeLoop (x, lp.start, lp.end, fade, equalPower);
    return finishLoop (z, lp.start, lp.end, mode);
}

// ------------------------------------------------------------------------------ mapping helpers
struct KeySpan { int center = 60, lokey = 60, hikey = 60; };

// Spans of `step` keys covering lo..hi exactly; the sample's keycenter sits in the middle of each span
// (max pitch shift ~ +-step/2 semitones).  A trailing span shorter than half a step is merged into the previous one.
inline std::vector<KeySpan> makeKeySpans (int lo, int hi, int step)
{
    std::vector<KeySpan> v;
    for (int s = lo; s <= hi; s += step)
    {
        const int e = std::min (s + step - 1, hi);
        const int n = e - s + 1;
        if (n * 2 < step && ! v.empty()) { v.back().hikey = e; break; }
        v.push_back ({ s + n / 2, s, e });
    }
    return v;
}

struct VelLayer { const char* name; int lo, hi; };
inline constexpr VelLayer kSoft { "soft", 1, 80 };
inline constexpr VelLayer kHard { "hard", 81, 127 };
inline constexpr VelLayer kAllVel { "all", 1, 127 };

// "pick_052_soft_rr1": articulation + key + optional layer + optional round robin (rr <= 0: omitted).
inline std::string zoneName (const std::string& articulation, int key, const char* layer = nullptr, int rr = 0)
{
    char buf[96];
    std::snprintf (buf, sizeof (buf), "%s_%03d", articulation.c_str(), key);
    std::string s (buf);
    if (layer != nullptr && layer[0] != '\0') { s += "_"; s += layer; }
    if (rr > 0) { s += "_rr"; s += std::to_string (rr); }
    return s;
}

// Appends alias zones (no audio of their own) that reuse the audio of every existing zone overlapping
// [srcLo, srcHi], shifted so that srcLo lands on dstLo.  Typical use:
//   addKeyAliases (lib, 36, 47, 48);   // mirror the percussion map one octave up
//   addKeyAliases (lib, 38, 38, 45);   // key 45 plays the TEK zones
// Release zones are mirrored too.  Returns the number of aliases added.
inline int addKeyAliases (Library& lib, int srcLo, int srcHi, int dstLo)
{
    const int delta = dstLo - srcLo;
    const std::size_t n = lib.zones.size();
    int added = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        const Zone z = lib.zones[i];
        if (z.hikey < srcLo || z.lokey > srcHi) continue;
        Zone a = z;
        a.data.clear();
        a.data.shrink_to_fit();
        a.sharedWith = z.sharedWith >= 0 ? z.sharedWith : (int) i;
        a.lokey = std::max (z.lokey, srcLo) + delta;
        a.hikey = std::min (z.hikey, srcHi) + delta;
        a.keycenter = clampi (z.keycenter + delta, 0, 127);
        a.name = z.name + "_k" + std::to_string (a.lokey);
        if (a.lokey < 0 || a.hikey > 127) continue;
        lib.zones.push_back (std::move (a));
        ++added;
    }
    return added;
}

// ------------------------------------------------------------------------------ progress
// Usage:   ProgressTracker pt (ctx, totalZones);
//          for (...) { if (! pt.next()) { lib.cancelled = true; return lib; }  ...render a zone... }
class ProgressTracker
{
public:
    ProgressTracker (const BuildContext& c, int totalSteps) : ctx (c), total (std::max (totalSteps, 1)) {}
    bool next() { return ctx.progress ((float) done++ / (float) total); }   // reports work completed so far
    bool finished() { return ctx.progress (1.0f); }
private:
    const BuildContext& ctx;
    int total, done = 0;
};

// Starts a library for this context (instrumentId set).
inline Library startLibrary (const BuildContext& ctx, int swLo = kKeyswitchLo, int swHi = kKeyswitchHi, int swDefault = kKeyswitchLo)
{
    Library lib;
    lib.instrumentId = ctx.spec().id;
    lib.swLo = swLo; lib.swHi = swHi; lib.swDefault = swDefault;
    return lib;
}

} // namespace humpire::model
