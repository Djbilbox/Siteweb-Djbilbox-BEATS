// ============================================================================
//  PLUCKED FAMILY: Mandole, Banjo, Qanun  (modelled, not recorded)
//
//  Model (the same engine drives the three instruments, only the parameter sets differ):
//
//    string   Modal synthesis.  Every partial of a stiff string is one exponentially damped cosine
//             (two-pole resonator started at a fractional sample time), with
//               * inharmonic partial frequencies  f_n = n f0 sqrt((1 + B n^2) / (1 + B)),
//               * partial amplitudes of an ideal plucked string  sin(n pi p) / n^gamma
//                 (p = plucking position, i.e. the pick-position comb) times a 2nd-order low-pass
//                 that stands for the hardness / width of the pick or plectrum,
//               * frequency dependent damping  alpha_n = alpha_0 + hfK (f_n / kHz)^hfQ,
//               * two polarisations per low partial (fast "vertical" + slow "horizontal" decay, slightly
//                 detuned) which gives the two-stage decay and the slow beating of real plucked strings,
//               * courses of 1..3 strings (mandole 2, qanun 3) with small deliberate detunings and a few
//                 tenths of a millisecond of stagger between the strings (the pick drags over them).
//    contact  A short band-passed noise burst (pick / plectrum scrape) and a short bi-phasic pulse that
//             hits the body ("thump"); both depend on the velocity layer.
//    body     A cascade of biquads (air resonance, plate / membrane modes, presence peak) applied to the whole
//             sum, so the body rings on its own modes after the string transient.
//    sympathy Qanun only: octave / twelfth partials of the neighbouring courses fade in slowly.
//
//  Velocity layers differ in timbre, not only in level (pick low-pass corner, pick position, partial roll-off,
//  contact noise, body knock).  Round robins differ in pick position, hardness, course detuning, stagger, string
//  order, slow-polarisation phases and noise.
//
//  Tremolo / roll = a ring buffer that is exactly periodic: K picks are laid on a circle of L samples
//  (the picks are phase-coherent: the pick interval is a whole number of fundamental periods, the string is
//  tuned so that K * m periods fit L samples exactly, picks are jittered by whole periods only).  All
//  decaying modes wrap around the circle and the body filter runs on a 3x tiled copy, so the loop is
//  seamless by construction (no cross-fade, no level step).
//
//  Everything is a pure function of the BuildContext: deterministic, no static state, no I/O.
// ============================================================================
#include "ModelCommon.h"

#include <algorithm>
#include <cmath>

namespace humpire::model
{
namespace
{
constexpr double kLn1000 = 6.907755278982137;   // ln (10^3): T60 -> decay rate

// ---------------------------------------------------------------------------------------------
//  Accumulation buffer (linear, or a circle for the seamless loops) and the damped-cosine primitive
// ---------------------------------------------------------------------------------------------
// A mode that has been set up (first samples already written) and waits for the batched renderer.
struct PendingMode
{
    double c1, c2, y1, y2;      // recursion y[n] = c1 y[n-1] + c2 y[n-2], with the two latest outputs
    std::size_t idx;            // buffer index of the next sample
    long long remaining;        // samples still to write
};

struct Sink
{
    Sink (int frames, double sampleRate, bool isCircular)
        : buf ((std::size_t) std::max (frames, 1), 0.0), sr (sampleRate), circular (isCircular)
    {
        rampLen = std::max (4, secondsToFrames (0.00025, sr));       // 0.25 ms soft onset of every mode
        ramp.resize ((std::size_t) rampLen);
        for (int k = 0; k < rampLen; ++k)
            ramp[(std::size_t) k] = 0.5 - 0.5 * std::cos (kPi * (double) (k + 1) / (double) (rampLen + 1));
    }

    std::vector<double> buf;
    std::vector<double> ramp;
    std::vector<PendingMode> pending;
    double sr;
    bool circular;
    int rampLen = 0;

    long long size() const { return (long long) buf.size(); }

    void add (long long i, double v)
    {
        const long long n = size();
        if (circular) { i %= n; if (i < 0) i += n; buf[(std::size_t) i] += v; }
        else if (i >= 0 && i < n) buf[(std::size_t) i] += v;
    }

    // Renders every queued mode.  The modes are sorted by length and run four at a time (independent recursions
    // overlap in the CPU pipeline: about 3x faster than one mode after the other).
    void flush()
    {
        std::stable_sort (pending.begin(), pending.end(), [] (const PendingMode& a, const PendingMode& b) { return a.remaining > b.remaining; });
        double* out = buf.data();
        const std::size_t nn = buf.size();
        std::size_t i = 0;
        for (; i + 4 <= pending.size(); i += 4)
        {
            PendingMode &a = pending[i], &b = pending[i + 1], &c = pending[i + 2], &d = pending[i + 3];
            double ya1 = a.y1, ya2 = a.y2, yb1 = b.y1, yb2 = b.y2, yc1 = c.y1, yc2 = c.y2, yd1 = d.y1, yd2 = d.y2;
            std::size_t ia = a.idx, ib = b.idx, ic = c.idx, id = d.idx;
            const double a1 = a.c1, a2 = a.c2, b1 = b.c1, b2 = b.c2, c1 = c.c1, c2 = c.c2, d1 = d.c1, d2 = d.c2;
            long long k = 0;
            for (const long long stop : { d.remaining, c.remaining, b.remaining, a.remaining })
            {
                // after the shortest ends, the remaining ones continue (a set of fewer chains)
                const int alive = stop == d.remaining ? 4 : stop == c.remaining ? 3 : stop == b.remaining ? 2 : 1;
                for (; k < stop; ++k)
                {
                    const double ya = a1 * ya1 + a2 * ya2; ya2 = ya1; ya1 = ya; out[ia] += ya; if (++ia == nn) ia = 0;
                    if (alive >= 2) { const double yb = b1 * yb1 + b2 * yb2; yb2 = yb1; yb1 = yb; out[ib] += yb; if (++ib == nn) ib = 0; }
                    if (alive >= 3) { const double yc = c1 * yc1 + c2 * yc2; yc2 = yc1; yc1 = yc; out[ic] += yc; if (++ic == nn) ic = 0; }
                    if (alive >= 4) { const double yd = d1 * yd1 + d2 * yd2; yd2 = yd1; yd1 = yd; out[id] += yd; if (++id == nn) id = 0; }
                }
            }
        }
        for (; i < pending.size(); ++i)
        {
            PendingMode& a = pending[i];
            double y1 = a.y1, y2 = a.y2;
            std::size_t ia = a.idx;
            for (long long k = 0; k < a.remaining; ++k)
            {
                const double y = a.c1 * y1 + a.c2 * y2; y2 = y1; y1 = y;
                out[ia] += y; if (++ia == nn) ia = 0;
            }
        }
        pending.clear();
    }
};

// y(t) = amp * exp (-alpha (t - t0)) * cos (2 pi freq (t - t0) + phase)   for t >= t0   (t in samples, t0 fractional).
// Two-pole recursion; stops when the mode has decayed below -106 dB re 1.0.  The first samples (soft onset) are
// written here, the rest is queued for Sink::flush().
void addMode (Sink& sk, double t0, double freq, double alpha, double amp, double phase)
{
    const double sr = sk.sr;
    if (! (freq > 0.0) || freq >= 0.49 * sr || std::fabs (amp) < 1.0e-9 || t0 < 0.0) return;
    const long long N = sk.size();
    const long long n0 = (long long) std::ceil (t0);
    if (! sk.circular && n0 >= N) return;

    const double w = kTwoPi * freq / sr;
    const double a = std::max (alpha, 1.0e-3) / sr;                       // nepers per sample
    const double r = std::exp (-a);
    const double tau = (double) n0 - t0;                                   // [0,1): delay to the first whole sample
    const double A0 = amp * std::exp (-a * tau);
    const double ph = phase + w * tau;

    const double kmaxD = std::log (std::fabs (A0) / 5.0e-6) / a;
    if (kmaxD <= 0.0) return;
    long long kmax = (long long) std::min (kmaxD, sk.circular ? 2.0 * (double) N : (double) (N - n0));
    if (kmax <= 0) return;

    PendingMode m;
    m.c1 = 2.0 * r * std::cos (w);
    m.c2 = -r * r;
    m.y1 = A0 / r * std::cos (ph - w);                                     // states at n = -1, -2
    m.y2 = A0 / (r * r) * std::cos (ph - 2.0 * w);

    double* out = sk.buf.data();
    std::size_t idx = (std::size_t) (n0 % N);
    const std::size_t nn = (std::size_t) N;
    const long long rl = std::min<long long> (sk.rampLen, kmax);
    for (long long k = 0; k < rl; ++k)
    {
        const double y = m.c1 * m.y1 + m.c2 * m.y2;
        m.y2 = m.y1; m.y1 = y;
        out[idx] += y * sk.ramp[(std::size_t) k];
        if (++idx == nn) idx = 0;
    }
    m.idx = idx;
    m.remaining = kmax - rl;
    if (m.remaining > 0) sk.pending.push_back (m);
}

// ---------------------------------------------------------------------------------------------
//  Instrument description
// ---------------------------------------------------------------------------------------------
struct BodyStage
{
    enum Kind { peak, highpass, lowpass, lowShelf, highShelf } kind;
    double hz, q, gainDb;
};

// Velocity-layer dependent excitation.
struct Layer
{
    double fcPick;        // Hz: corner of the pick / finger low-pass
    double gamma;         // partial roll-off exponent (amp ~ 1 / n^gamma)
    double pickPos;       // plucking position, fraction of the string length from the bridge
    double clickAmp;      // pick-noise burst level (re the string)
    double clickHz, clickQ, clickMs;
    double thumpAmp;      // body knock level
    double crestDb;       // peak limiter: allowed peak above the loudness measure
};

struct Model
{
    // strings
    int strings = 1;                  // strings per course
    double centsSpread = 0.0;         // detuning spread across the course (cents)
    double staggerMs = 0.5;           // delay between the strings of a course
    double inh0 = 1.0e-4, inh1 = 3.0e-4;                   // inharmonicity B at 150 Hz and at 800 Hz (log interpolation)
    double t60Ref = 2.0, t60RefHz = 220.0, t60Exp = 0.3, t60Min = 0.5, t60Max = 4.0;   // fundamental T60 versus pitch
    double hfK = 5.0, hfQ = 1.6;      // extra decay rate 1/s = hfK * (f / kHz)^hfQ
    double slowAmp = 0.2, slowRatio = 0.35;                 // 2nd polarisation: amplitude share, decay-rate ratio
    int slowMaxPartial = 10;
    double chorusHz = 2500.0;         // above this the strings of a course are merged into one mode set
    double maxHz = 16000.0;
    int maxPartials = 150;
    double haloDb = -200.0;           // sympathetic octave / 12th (qanun)
    double haloRiseMs = 70.0;
    std::vector<BodyStage> body;
    Layer hard, soft, trem;           // trem: light, fast re-picking (tremolo / roll)
};

// Per-zone realisation (random but deterministic details).
struct Voice
{
    double f0 = 220.0, B = 0.0, alpha0 = 3.0, alphaMul = 1.0, pickPos = 0.15, fc = 4000.0;
    double cents[4] = { 0, 0, 0, 0 }, sgain[4] = { 1, 1, 1, 1 };
    const Layer* layer = nullptr;
};

double partialHz (double f0, double B, int n) { return (double) n * f0 * std::sqrt ((1.0 + B * (double) n * (double) n) / (1.0 + B)); }
double pickLp (double f, double fc) { const double x = f / fc; return 1.0 / std::sqrt (1.0 + x * x * x * x); }

Voice makeVoice (const Model& M, const Layer& L, double f0, double decayScale, Rng& rng)
{
    Voice v;
    v.layer = &L;
    v.f0 = f0;
    const double u = clampd (std::log2 (f0 / 150.0) / std::log2 (800.0 / 150.0), 0.0, 1.0);
    v.B = std::exp (lerp (std::log (M.inh0), std::log (M.inh1), u));
    const double t60 = clampd (M.t60Ref * std::pow (M.t60RefHz / f0, M.t60Exp), M.t60Min, M.t60Max);
    v.alpha0 = kLn1000 / t60;
    v.alphaMul = 1.0 / decayScale;
    v.pickPos = L.pickPos * rng.range (0.88, 1.12);
    v.fc = L.fcPick * rng.range (0.9, 1.12);
    const int S = clampi (M.strings, 1, 4);
    double mean = 0.0;
    for (int s = 0; s < S; ++s)
    {
        const double pos = S > 1 ? (double) s / (double) (S - 1) - 0.5 : 0.0;       // -0.5 .. +0.5
        v.cents[s] = pos * M.centsSpread * rng.range (0.65, 1.3) + (S > 1 ? rng.range (-0.25, 0.25) : 0.0);
        v.sgain[s] = rng.range (0.85, 1.0);
        mean += v.cents[s] / (double) S;
    }
    for (int s = 0; s < S; ++s) v.cents[s] -= mean;                                  // the course stays in tune on average
    return v;
}

// Pick-noise burst: band-passed noise with a very short rise and an exponential decay.
void addClick (Sink& sk, double t0, double amp, const Layer& L, Rng& rng)
{
    if (amp < 1.0e-6) return;
    const double sr = sk.sr;
    Biquad bp = Biquad::bandpass (L.clickHz * rng.range (0.85, 1.15), L.clickQ, sr);
    const double tau = std::max (L.clickMs * 1.0e-3 * sr, 4.0), rise = std::max (0.0002 * sr, 3.0);
    const long long len = (long long) (tau * 6.0);
    const long long n0 = (long long) std::ceil (t0);
    for (long long k = 0; k < len; ++k)
    {
        const double env = (1.0 - std::exp (-(double) k / rise)) * std::exp (-(double) k / tau);
        sk.add (n0 + k, amp * env * bp.process (rng.bipolar() * 1.7));
    }
}

// Bi-phasic pulse (zero mean) of about 1 ms: excites the low body modes.
void addThump (Sink& sk, double t0, double amp, Rng& rng)
{
    if (amp < 1.0e-6) return;
    const int len = std::max (8, secondsToFrames (0.0011 * rng.range (0.9, 1.1), sk.sr));
    const long long n0 = (long long) std::ceil (t0);
    for (int k = 0; k < len; ++k)
    {
        const double x = (double) (k + 1) / (double) (len + 1);
        sk.add (n0 + k, amp * std::sin (kTwoPi * x) * (0.5 - 0.5 * std::cos (kTwoPi * x)) * 2.0);
    }
}

// One pick / pluck of the whole course at time t0 (samples).
void addPick (Sink& sk, const Model& M, const Voice& V, double t0, double gain, double fcMul, bool reversed, Rng& rng)
{
    const double sr = sk.sr;
    const Layer& L = *V.layer;
    const int S = clampi (M.strings, 1, 4);
    const double fc = V.fc * fcMul;
    const double p = V.pickPos * rng.range (0.97, 1.03);

    double tStr[4], gStr[4], tMean = 0.0, gSum = 0.0;
    for (int s = 0; s < S; ++s)
    {
        const int slot = reversed ? S - 1 - s : s;
        tStr[s] = t0 + (double) slot * M.staggerMs * 1.0e-3 * sr * rng.range (0.75, 1.25);
        gStr[s] = gain * V.sgain[s] * rng.range (0.92, 1.08);
        tMean += tStr[s] / (double) S;
        gSum += gStr[s];
    }

    for (int n = 1; n <= M.maxPartials; ++n)
    {
        const double fn = partialHz (V.f0, V.B, n);
        if (fn > M.maxHz || fn > 0.45 * sr) break;
        const double a0 = std::sin ((double) n * kPi * p) / std::pow ((double) n, L.gamma) * pickLp (fn, fc);
        if (std::fabs (a0) < 1.5e-4) continue;
        const double alpha = (V.alpha0 + M.hfK * std::pow (fn * 1.0e-3, M.hfQ)) * V.alphaMul;
        const bool chorus = S > 1 && fn < M.chorusHz;
        const int nS = chorus ? S : 1;
        const double taper = n <= M.slowMaxPartial ? 1.0 - (double) (n - 1) / (double) (M.slowMaxPartial + 1) : 0.0;
        const double gSlow = M.slowAmp * taper;
        for (int s = 0; s < nS; ++s)
        {
            const double f = chorus ? fn * std::exp2 (V.cents[s] / 1200.0) : fn;
            const double t = chorus ? tStr[s] : tMean;
            const double a = a0 * (chorus ? gStr[s] : gSum);
            addMode (sk, t, f, alpha, a * (1.0 - gSlow), 0.0);
            if (gSlow > 0.0)
                addMode (sk, t, f * (1.0 + rng.range (-1.0, 1.0) * 0.0006), alpha * M.slowRatio, a * gSlow, rng.range (-0.7, 0.7));
        }
    }

    // sympathetic strings of the neighbouring courses: octave, twelfth, double octave, fading in slowly
    if (M.haloDb > -100.0)
        for (int h = 2; h <= 4; ++h)
        {
            const double fh = partialHz (V.f0, V.B, h) * (1.0 + rng.range (-1.0, 1.0) * 0.0004);
            const double a = std::fabs (std::sin ((double) h * kPi * p) + 0.35) / std::pow ((double) h, L.gamma) * pickLp (fh, fc)
                             * dbToGain (M.haloDb) * gain * rng.range (0.7, 1.2);
            const double alpha = (V.alpha0 + M.hfK * std::pow (fh * 1.0e-3, M.hfQ)) * V.alphaMul * 0.45;
            const double rise = 1000.0 / M.haloRiseMs * rng.range (0.8, 1.25);
            addMode (sk, tMean, fh, alpha, a, 0.0);
            addMode (sk, tMean, fh, alpha + rise, -a, 0.0);
        }

    addClick (sk, t0, gain * L.clickAmp * rng.range (0.8, 1.2), L, rng);
    addThump (sk, t0, gain * L.thumpAmp * rng.range (0.85, 1.15), rng);
}

// ---------------------------------------------------------------------------------------------
//  Body filter bank and finishing
// ---------------------------------------------------------------------------------------------
Biquad makeBiquad (const BodyStage& s, double sr)
{
    switch (s.kind)
    {
        case BodyStage::peak:      return Biquad::peaking (s.hz, s.q, s.gainDb, sr);
        case BodyStage::highpass:  return Biquad::highpass (s.hz, s.q, sr);
        case BodyStage::lowpass:   return Biquad::lowpass (s.hz, s.q, sr);
        case BodyStage::lowShelf:  return Biquad::lowShelf (s.hz, s.gainDb, sr);
        case BodyStage::highShelf: return Biquad::highShelf (s.hz, s.gainDb, sr);
    }
    return Biquad();
}

// Circular sinks are filtered on a 3x tiled copy (middle copy kept), so the result is periodic.
void applyBody (Sink& sk, const std::vector<BodyStage>& stages)
{
    sk.flush();
    std::vector<Biquad> chain;
    for (const auto& s : stages) chain.push_back (makeBiquad (s, sk.sr));
    const std::size_t N = sk.buf.size();
    auto run = [&chain] (double x) { for (auto& f : chain) x = f.process (x); return x; };
    if (! sk.circular)
    {
        for (auto& v : sk.buf) v = run (v);
        return;
    }
    std::vector<double> t (3 * N);
    for (std::size_t i = 0; i < 3 * N; ++i) t[i] = run (sk.buf[i % N]);
    std::copy (t.begin() + (std::ptrdiff_t) N, t.begin() + (std::ptrdiff_t) (2 * N), sk.buf.begin());
}

// Look-ahead peak limiter with a smooth gain curve (1.2 ms look-ahead / 30 ms release): lowers the crest factor
// so that the loudness target can be met without exceeding the peak ceiling.  `circular` treats the buffer as a loop.
void tamePeaks (std::vector<float>& x, double sr, double crestDb, bool circular)
{
    const long long N = (long long) x.size();
    if (N < 64) return;
    const long long A = std::max (4, secondsToFrames (0.0012, sr));
    const double rel = 1.0 - std::exp (-1.0 / (0.03 * sr));
    for (int pass = 0; pass < 3; ++pass)
    {
        const double thr = dbToGain (loudnessDb (x.data(), x.size(), sr) + crestDb);
        if ((double) peakOf (x) <= thr) return;
        std::vector<double> mm ((std::size_t) N, 1.0);
        for (long long i = 0; i < N; ++i)
        {
            const double ax = std::fabs ((double) x[(std::size_t) i]);
            if (ax <= thr) continue;
            const double need = thr / ax;
            for (long long j = i - A; j <= i + A; ++j)
            {
                long long jj = j;
                if (circular) { jj %= N; if (jj < 0) jj += N; }
                else if (jj < 0 || jj >= N) continue;
                if (need < mm[(std::size_t) jj]) mm[(std::size_t) jj] = need;
            }
        }
        auto at = [&] (long long j) -> double
        {
            if (circular) { j %= N; if (j < 0) j += N; return mm[(std::size_t) j]; }
            return j < 0 || j >= N ? 1.0 : mm[(std::size_t) j];
        };
        std::vector<double> g ((std::size_t) N);
        double sum = 0.0;
        for (long long j = -A; j <= A; ++j) sum += at (j);
        for (long long i = 0; i < N; ++i)
        {
            g[(std::size_t) i] = sum / (double) (2 * A + 1);
            sum += at (i + A + 1) - at (i - A);
        }
        if (circular)
        {
            // release recursion run over two laps so that it is periodic
            double prev = 1.0;
            for (int lap = 0; lap < 2; ++lap)
                for (long long i = 0; i < N; ++i)
                {
                    prev = std::min (g[(std::size_t) i], prev + (1.0 - prev) * rel);
                    if (lap == 1) x[(std::size_t) i] = (float) ((double) x[(std::size_t) i] * prev);
                }
        }
        else
        {
            double prev = 1.0;
            for (long long i = 0; i < N; ++i)
            {
                prev = std::min (g[(std::size_t) i], prev + (1.0 - prev) * rel);
                x[(std::size_t) i] = (float) ((double) x[(std::size_t) i] * prev);
            }
        }
    }
}

std::vector<float> toFloat (const Sink& sk)
{
    std::vector<float> v (sk.buf.size());
    for (std::size_t i = 0; i < v.size(); ++i) v[i] = (float) sk.buf[i];
    return v;
}

// ---------------------------------------------------------------------------------------------
//  Zone builders
// ---------------------------------------------------------------------------------------------
struct Plan
{
    const char* pluckName = "pick";
    const char* loopName = "tremolo";
    int step = 3, rr = 3;
    double ampRelease = 0.3, ampVeltrack = 70.0;
    double loopAttack = 0.02, loopRelease = 0.35;
    // length of a plucked sample: secBase * (secRefHz / f0)^secExp, clamped
    double secBase = 1.5, secRefHz = 220.0, secExp = 0.0, secMin = 0.8, secMax = 2.0;
    double fadeOutSec = 0.3;
    // tremolo / roll
    double tremRate = 11.5, tremLoopSec = 1.1, tremDecay = 0.6, tremJitterMs = 3.5, tremUpGain = 0.82, tremLoudDb = -17.5;
    double tremFcMul = 1.0;
};

Zone baseZone (const BuildContext& ctx, const char* articulation, const std::string& name, const KeySpan& sp, const VelLayer& vl, int swLast)
{
    Zone z;
    z.articulation = articulation;
    z.name = name;
    z.sampleRate = ctx.sampleRate;
    z.lokey = sp.lokey; z.hikey = sp.hikey; z.keycenter = sp.center;
    z.lovel = vl.lo; z.hivel = vl.hi;
    z.swLast = swLast;
    return z;
}

std::vector<float> renderPluck (const Model& M, const Layer& L, const Plan& P, double f0, double sr, Rng& rng)
{
    const double seconds = clampd (P.secBase * std::pow (P.secRefHz / f0, P.secExp), P.secMin, P.secMax);
    Sink sk (secondsToFrames (seconds, sr), sr, false);
    const Voice v = makeVoice (M, L, f0, 1.0, rng);
    addPick (sk, M, v, 0.0, 1.0, 1.0, rng.uniform() < 0.5, rng);
    applyBody (sk, M.body);
    std::vector<float> out = toFloat (sk);
    sanitize (out, true);
    fadeIn (out, std::max (8, secondsToFrames (0.0004, sr)));
    fadeOut (out, std::min ((int) out.size() / 3, secondsToFrames (P.fadeOutSec, sr)));
    tamePeaks (out, sr, L.crestDb, false);
    return out;
}

// Seamless tremolo / roll: see the header comment.
bool renderTremolo (Zone& z, const Model& M, const Plan& P, double f0nom, Rng& rng)
{
    const double sr = z.sampleRate;
    const int m = std::max (2, (int) std::lround (f0nom / P.tremRate));                  // fundamental periods per pick
    const int K = std::max (6, (int) std::ceil (P.tremLoopSec * f0nom / (double) m));    // picks per loop
    const long long Ls = std::llround ((double) K * (double) m * sr / f0nom);
    const double f0 = (double) K * (double) m * sr / (double) Ls;                          // exact: K*m periods == Ls samples
    const double T0 = sr / f0;
    const int J = clampi ((int) std::lround (P.tremJitterMs * 1.0e-3 * f0), 0, m / 3);

    Sink sk ((int) Ls, sr, true);
    const Layer& L = M.trem;
    const Voice v = makeVoice (M, L, f0, P.tremDecay, rng);
    for (int k = 0; k < K; ++k)
    {
        const int jit = (k == 0 || J == 0) ? 0 : rng.rangeInt (-J, J);
        const double t0 = (double) ((long long) k * m + jit) * T0;
        const bool down = (k & 1) == 0;
        const double gain = (down ? 1.0 : P.tremUpGain) * rng.range (0.88, 1.08);
        addPick (sk, M, v, t0, gain, P.tremFcMul * (down ? 1.0 : 0.88) * rng.range (0.93, 1.07), ! down, rng);
    }
    applyBody (sk, M.body);
    std::vector<float> S = toFloat (sk);
    sanitize (S, true);
    tamePeaks (S, sr, 12.0, true);
    normaliseLoudness (S, sr, P.tremLoudDb, kPeakCeilingDb);

    // The sample starts on the first pick.  The loop seam is searched between 30 ms and ~2 pick intervals later for the
    // position where the 20 ms RMS before and after the seam match best and the waveform is smoothest (the seam of a
    // periodic signal is always continuous; this makes it also unobtrusive for level checks).
    const long long w = std::max<long long> (8, secondsToFrames (0.02, sr));
    std::vector<double> cum ((std::size_t) (3 * Ls + 1), 0.0);                 // running sum of squares over 3 laps
    for (long long i = 0; i < 3 * Ls; ++i)
    {
        const double s0 = (double) S[(std::size_t) (i % Ls)];
        cum[(std::size_t) (i + 1)] = cum[(std::size_t) i] + s0 * s0;
    }
    double stepAcc = 0.0;
    for (long long i = 0; i < Ls; ++i)
    {
        const double d = (double) S[(std::size_t) ((i + 1) % Ls)] - (double) S[(std::size_t) i];
        stepAcc += d * d;
    }
    const double step = std::sqrt (stepAcc / (double) Ls) + 1.0e-9;
    auto at = [&S, Ls] (long long i) { return (double) S[(std::size_t) (((i % Ls) + Ls) % Ls)]; };
    const long long searchLo = secondsToFrames (0.03, sr), searchHi = searchLo + (long long) (2.0 * (double) m * T0);
    long long intro = searchLo;
    double bestScore = 1.0e30;
    for (long long s = searchLo; s < std::min (searchHi, Ls - 1); ++s)
    {
        const long long base = Ls;                                             // second lap: indices s-1-w .. s+w-1 are all >= 0
        const double eBefore = cum[(std::size_t) (base + s - 1)] - cum[(std::size_t) (base + s - 1 - w)];
        const double eAfter = cum[(std::size_t) (base + s + w)] - cum[(std::size_t) (base + s)];
        const double levelDb = std::fabs (10.0 * std::log10 ((eAfter + 1.0e-12) / (eBefore + 1.0e-12)));
        const double jump = std::fabs (at (s) - at (s - 1)) / step;
        const double slope = std::fabs ((at (s) - at (s - 1)) - (at (s - 1) - at (s - 2))) / step;
        const double score = levelDb / 1.5 + jump / 3.0 + slope / 5.0;
        if (score < bestScore) { bestScore = score; intro = s; }
    }
    z.data.assign ((std::size_t) (intro + Ls), 0.f);
    for (long long n = 0; n < intro + Ls; ++n) z.data[(std::size_t) n] = S[(std::size_t) (n % Ls)];
    fadeIn (z.data, std::max (8, secondsToFrames (0.0004, sr)));       // touches the first 18 frames only (loop starts at `intro`)
    return finishLoop (z, (int) intro, (int) (intro + Ls - 1), Loop::continuous);
}

Library buildFamily (const BuildContext& ctx, const Model& M, const Plan& P)
{
    const auto spans = makeKeySpans (ctx.spec().lo, ctx.spec().hi, P.step);
    Library lib = startLibrary (ctx);
    const int total = (int) spans.size() * (2 * P.rr + 1);
    ProgressTracker pt (ctx, total);
    const double sr = ctx.sampleRate;
    int zoneIndex = 0;

    for (const auto& sp : spans)
    {
        const double f0 = midiToHz (sp.center);
        for (const VelLayer* vl : { &kSoft, &kHard })
        {
            const Layer& L = vl == &kSoft ? M.soft : M.hard;
            for (int r = 1; r <= P.rr; ++r)
            {
                if (! pt.next()) { lib.cancelled = true; return lib; }
                Rng rng = Rng::forZone (ctx.instrument, zoneIndex++);
                Zone z = baseZone (ctx, P.pluckName, zoneName (P.pluckName, sp.center, vl->name, P.rr > 1 ? r : 0), sp, *vl, 12);
                z.seqLength = P.rr; z.seqPosition = r;
                z.ampRelease = (float) P.ampRelease; z.ampVeltrack = (float) P.ampVeltrack;
                z.data = renderPluck (M, L, P, f0, sr, rng);
                normaliseLoudness (z.data, sr, vl == &kSoft ? kSoftLoudnessDb : kHardLoudnessDb, kPeakCeilingDb);
                lib.zones.push_back (std::move (z));
            }
        }

        if (! pt.next()) { lib.cancelled = true; return lib; }
        Rng rng = Rng::forZone (ctx.instrument, zoneIndex++);
        Zone z = baseZone (ctx, P.loopName, zoneName (P.loopName, sp.center), sp, kAllVel, 13);
        z.ampAttack = (float) P.loopAttack; z.ampRelease = (float) P.loopRelease; z.ampVeltrack = (float) P.ampVeltrack;
        if (! renderTremolo (z, M, P, f0, rng))
        {
            lib.zones.clear();                                   // cannot happen with these lengths; keep the contract anyway
            return lib;
        }
        lib.zones.push_back (std::move (z));
    }
    pt.finished();
    return lib;
}

// ---------------------------------------------------------------------------------------------
//  Instrument parameter sets
// ---------------------------------------------------------------------------------------------
Model mandoleModel()
{
    // Algerian mandole: double courses of steel strings, plectrum, wooden bowl / flat-back body.
    Model m;
    m.strings = 2; m.centsSpread = 3.0; m.staggerMs = 0.7;
    m.inh0 = 1.5e-4; m.inh1 = 4.0e-4;
    m.t60Ref = 2.2; m.t60RefHz = 220.0; m.t60Exp = 0.30; m.t60Min = 0.7; m.t60Max = 3.2;
    m.hfK = 2.6; m.hfQ = 1.5;
    m.slowAmp = 0.22; m.slowRatio = 0.35; m.slowMaxPartial = 10;
    m.chorusHz = 2600.0;
    m.body = { { BodyStage::highpass, 85.0, 0.7, 0.0 },
               { BodyStage::peak, 235.0, 7.0, 6.0 },       // air (Helmholtz) resonance
               { BodyStage::peak, 470.0, 5.0, 3.5 },       // top plate
               { BodyStage::peak, 760.0, 4.0, 4.5 },
               { BodyStage::peak, 1500.0, 2.0, 2.0 },
               { BodyStage::peak, 2600.0, 1.2, 4.0 },      // bridge / presence
               { BodyStage::highShelf, 7000.0, 0.7, -3.0 } };
    m.hard = { 5500.0, 1.00, 0.135, 0.14, 3200.0, 0.9, 2.2, 0.5, 9.0 };
    m.soft = { 2000.0, 1.15, 0.170, 0.04, 2200.0, 0.8, 3.0, 0.3, 10.0 };
    m.trem = { 3600.0, 1.05, 0.150, 0.05, 2800.0, 0.8, 2.5, 0.18, 10.0 };
    return m;
}

Model banjoModel()
{
    // Tenor banjo: single steel strings over a tensioned drum head, bridge twang, very quick bright decay.
    Model m;
    m.strings = 1;
    m.inh0 = 2.5e-4; m.inh1 = 6.0e-4;
    m.t60Ref = 1.0; m.t60RefHz = 330.0; m.t60Exp = 0.30; m.t60Min = 0.35; m.t60Max = 1.4;
    m.hfK = 4.5; m.hfQ = 1.5;
    m.slowAmp = 0.30; m.slowRatio = 0.40; m.slowMaxPartial = 8;
    m.body = { { BodyStage::highpass, 140.0, 0.8, 0.0 },
               { BodyStage::peak, 380.0, 5.0, 9.0 },       // head main mode
               { BodyStage::peak, 600.0, 5.0, 5.0 },       // head mode 1.59x
               { BodyStage::peak, 820.0, 4.0, 4.0 },       // head mode 2.14x
               { BodyStage::peak, 1050.0, 4.0, 2.5 },
               { BodyStage::peak, 1600.0, 2.0, -2.5 },
               { BodyStage::peak, 2300.0, 1.3, 5.0 },      // bridge ping
               { BodyStage::peak, 3700.0, 1.5, 3.0 },
               { BodyStage::lowpass, 10000.0, 0.7, 0.0 } };
    m.hard = { 7500.0, 0.90, 0.100, 0.22, 4200.0, 0.9, 1.6, 0.8, 9.0 };
    m.soft = { 3200.0, 1.05, 0.130, 0.07, 3000.0, 0.8, 2.2, 0.45, 10.0 };
    m.trem = { 4800.0, 0.95, 0.110, 0.10, 3600.0, 0.8, 1.8, 0.35, 10.0 };
    return m;
}

Model qanunModel()
{
    // Qanun: three nylon strings per course, metal plectra, fish-skin window on a large trapezoid box.
    Model m;
    m.strings = 3; m.centsSpread = 4.0; m.staggerMs = 0.9;
    m.inh0 = 4.0e-5; m.inh1 = 1.2e-4;
    m.t60Ref = 3.6; m.t60RefHz = 262.0; m.t60Exp = 0.50; m.t60Min = 1.0; m.t60Max = 6.5;
    m.hfK = 2.8; m.hfQ = 1.5;
    m.slowAmp = 0.28; m.slowRatio = 0.40; m.slowMaxPartial = 12;
    m.chorusHz = 2200.0;
    m.haloDb = -26.0; m.haloRiseMs = 70.0;
    m.body = { { BodyStage::highpass, 60.0, 0.7, 0.0 },
               { BodyStage::peak, 150.0, 5.0, 4.0 },       // air cavity
               { BodyStage::peak, 340.0, 4.0, 5.0 },       // skin window main mode
               { BodyStage::peak, 700.0, 4.0, 3.5 },
               { BodyStage::peak, 1350.0, 2.5, 4.0 },
               { BodyStage::peak, 2800.0, 1.5, 4.0 } };
    m.hard = { 6500.0, 1.00, 0.170, 0.12, 3800.0, 0.9, 1.8, 0.4, 9.0 };
    m.soft = { 2500.0, 1.10, 0.200, 0.04, 2500.0, 0.8, 2.6, 0.25, 10.0 };
    m.trem = { 4200.0, 1.05, 0.180, 0.05, 2800.0, 0.8, 2.2, 0.15, 10.0 };
    return m;
}
} // namespace

// ---------------------------------------------------------------------------------------------
Library buildMandole (const BuildContext& ctx)
{
    Plan p;
    p.pluckName = "pick"; p.loopName = "tremolo"; p.step = 3; p.rr = 3;
    p.ampRelease = 0.30; p.ampVeltrack = 70.0; p.loopAttack = 0.02; p.loopRelease = 0.35;
    p.secBase = 1.6; p.secExp = 0.0; p.secMin = 1.6; p.secMax = 1.6;
    p.fadeOutSec = 0.30;
    p.tremRate = 11.5; p.tremLoopSec = 1.1; p.tremDecay = 0.8; p.tremJitterMs = 3.5; p.tremUpGain = 0.82; p.tremLoudDb = -17.5;
    return buildFamily (ctx, mandoleModel(), p);
}

Library buildBanjo (const BuildContext& ctx)
{
    Plan p;
    p.pluckName = "pick"; p.loopName = "roll"; p.step = 3; p.rr = 2;
    p.ampRelease = 0.12; p.ampVeltrack = 70.0; p.loopAttack = 0.015; p.loopRelease = 0.18;
    p.secBase = 1.0; p.secExp = 0.0; p.secMin = 1.0; p.secMax = 1.0;
    p.fadeOutSec = 0.15;
    p.tremRate = 15.0; p.tremLoopSec = 1.0; p.tremDecay = 0.75; p.tremJitterMs = 2.5; p.tremUpGain = 0.8; p.tremLoudDb = -17.5;
    return buildFamily (ctx, banjoModel(), p);
}

Library buildQanun (const BuildContext& ctx)
{
    Plan p;
    p.pluckName = "pluck"; p.loopName = "tremolo"; p.step = 4; p.rr = 2;
    p.ampRelease = 0.25; p.ampVeltrack = 65.0; p.loopAttack = 0.02; p.loopRelease = 0.35;
    p.secBase = 2.9; p.secRefHz = 262.0; p.secExp = 0.35; p.secMin = 1.5; p.secMax = 3.0;
    p.fadeOutSec = 0.40;
    p.tremRate = 10.0; p.tremLoopSec = 1.2; p.tremDecay = 0.75; p.tremJitterMs = 4.0; p.tremUpGain = 0.85; p.tremLoudDb = -17.5;
    return buildFamily (ctx, qanunModel(), p);
}
} // namespace humpire::model
