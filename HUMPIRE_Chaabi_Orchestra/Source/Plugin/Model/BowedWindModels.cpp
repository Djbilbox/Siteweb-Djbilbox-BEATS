// ============================================================================
//  BOWED + WIND FAMILY: Violon (violin), Ney (end-blown flute).
//
//  VIOLON  Bowed-string digital waveguide: a string split at the bow into a bridge-side and a nut-side
//          delay line, one-pole reflection (loss) filters at both ends and a memoryless friction table
//          at the bow (stick-slip: the Helmholtz motion emerges by itself).  The bridge force drives a
//          bank of body resonances (air mode, main wood modes, bridge hill).  Bow speed, bow force noise,
//          slow pitch / loudness drift and vibrato are control signals that are exactly periodic in the
//          loop length, so the sustain loop is seamless.  Pizzicato = detuned pair of Karplus-Strong
//          strings through the same body.  Bow lift = short filtered noise tick.
//  NEY     (see below)
//
//  Everything is deterministic (model::Rng seeded per zone), std only, no globals.
// ============================================================================
#include "ModelCommon.h"

namespace humpire::model
{
namespace
{
// ------------------------------------------------------------------------------------------------
//  Shared helpers
// ------------------------------------------------------------------------------------------------

// Mean frequency (Hz) of the component near `guessHz`: phase slope of Hann-windowed complex demodulation
// blocks (sub-cent accuracy on steady tones).  Returns 0 when the buffer is too short.  `share` receives
// the fraction of the signal power carried by that component (about 0.6 for a Helmholtz sawtooth).
double measureF0Hz (const float* x, std::size_t n, double sr, double guessHz, double* share = nullptr)
{
    double f = guessHz;
    double sh = 0.0;
    for (int iter = 0; iter < 3; ++iter)
    {
        const int win = (int) std::max (256.0, std::floor (6.0 * sr / f));
        const int hop = std::max (1, win / 2);
        if ((int) n < win + hop) return 0.0;
        std::vector<double> hann ((std::size_t) win);
        for (int i = 0; i < win; ++i) hann[(std::size_t) i] = 0.5 - 0.5 * std::cos (kTwoPi * ((double) i + 0.5) / (double) win);
        const double w = kTwoPi * f / sr;
        const double cr = std::cos (w), ci = std::sin (w);
        std::vector<double> ph, wt, tc;
        double z2 = 0.0, power = 0.0;
        for (std::size_t s = 0; s + (std::size_t) win <= n; s += (std::size_t) hop)
        {
            double pr = std::cos (w * (double) s), pi = -std::sin (w * (double) s), re = 0.0, im = 0.0;
            for (int i = 0; i < win; ++i)
            {
                const double v = (double) x[s + (std::size_t) i];
                re += v * hann[(std::size_t) i] * pr; im += v * hann[(std::size_t) i] * pi;
                power += v * v;
                const double nr = pr * cr + pi * ci, ni = pi * cr - pr * ci;
                pr = nr; pi = ni;
            }
            ph.push_back (std::atan2 (im, re));
            wt.push_back (re * re + im * im);
            tc.push_back (((double) s + 0.5 * (double) win) / sr);
            z2 += re * re + im * im;
        }
        const std::size_t m = ph.size();
        if (m < 2) return 0.0;
        for (std::size_t i = 1; i < m; ++i)
        {
            while (ph[i] - ph[i - 1] > kPi) ph[i] -= kTwoPi;
            while (ph[i] - ph[i - 1] < -kPi) ph[i] += kTwoPi;
        }
        double sw = 0.0, st = 0.0, sp = 0.0;
        for (std::size_t i = 0; i < m; ++i) { sw += wt[i]; st += wt[i] * tc[i]; sp += wt[i] * ph[i]; }
        if (sw <= 1.0e-30) return 0.0;
        st /= sw; sp /= sw;
        double num = 0.0, den = 0.0;
        for (std::size_t i = 0; i < m; ++i) { num += wt[i] * (tc[i] - st) * (ph[i] - sp); den += wt[i] * (tc[i] - st) * (tc[i] - st); }
        if (den <= 1.0e-30) return 0.0;
        f += num / den / kTwoPi;
        // sinusoid of amplitude A: |Z| = A * win / 4  ->  power = A^2 / 2 = 8 |Z|^2 / win^2
        const double meanPower = power / ((double) m * (double) win);        // blocks overlap 2x, so ~ mean x^2 * 2 / 2
        sh = meanPower > 1.0e-30 ? (8.0 * (z2 / (double) m) / ((double) win * (double) win)) / (meanPower * 1.0) : 0.0;
    }
    if (share != nullptr) *share = sh;
    return f;
}

// Zero-phase windowed-sinc decimator (Kaiser, ~90 dB stop band, pass band to 0.9 x output Nyquist).
double besselI0 (double x)
{
    double sum = 1.0, term = 1.0;
    for (int k = 1; k < 40; ++k) { term *= (x / (2.0 * (double) k)) * (x / (2.0 * (double) k)); sum += term; if (term < 1.0e-14 * sum) break; }
    return sum;
}

std::vector<float> decimateBy (const std::vector<float>& x, int os, std::size_t outFrames)
{
    std::vector<float> y (outFrames, 0.f);
    if (os <= 1) { for (std::size_t i = 0; i < outFrames && i < x.size(); ++i) y[i] = x[i]; return y; }
    const int half = 24 * os;
    const double fc = 0.45 / (double) os;                           // cycles per input sample
    std::vector<double> h ((std::size_t) (2 * half + 1));
    const double beta = 9.0, i0b = besselI0 (beta);
    double sum = 0.0;
    for (int k = -half; k <= half; ++k)
    {
        const double u = (double) k / (double) half;
        const double win = besselI0 (beta * std::sqrt (std::max (0.0, 1.0 - u * u))) / i0b;
        const double a = kTwoPi * fc * (double) k;
        const double s = k == 0 ? 2.0 * fc : std::sin (a) / (kPi * (double) k);
        h[(std::size_t) (k + half)] = s * win;
        sum += s * win;
    }
    for (auto& v : h) v /= sum;
    const long long nIn = (long long) x.size();
    for (std::size_t m = 0; m < outFrames; ++m)
    {
        const long long c = (long long) m * os;
        double acc = 0.0;
        for (int k = -half; k <= half; ++k)
        {
            const long long idx = c + k;
            if (idx >= 0 && idx < nIn) acc += h[(std::size_t) (k + half)] * (double) x[(std::size_t) idx];
        }
        y[m] = (float) acc;
    }
    return y;
}

// Low-pass filtered unit-variance noise track; `circular` makes the end join the start smoothly
// (the track can then be indexed modulo its length).
std::vector<float> noiseTrack (Rng& rng, int length, double fcHz, double sr, bool circular)
{
    std::vector<float> v ((std::size_t) std::max (length, 8));
    for (auto& s : v) s = (float) rng.gaussian();
    OnePole lp, lp2;
    lp.setLowpass (fcHz, sr); lp2.setLowpass (fcHz, sr);
    const int passes = circular ? 2 : 1;
    for (int p = 0; p < passes; ++p)
        for (auto& s : v) s = (float) lp2.lowpass (lp.lowpass ((double) s));
    double e = 0.0;
    for (auto s : v) e += (double) s * (double) s;
    e = std::sqrt (e / (double) v.size());
    if (e > 1.0e-12) for (auto& s : v) s = (float) ((double) s / e);
    return v;
}

// Sum of a few sinusoids whose frequencies are whole multiples of 1/period (periodic in `period` seconds),
// or free-running when period <= 0.  Used for the slow pitch / loudness drift of a "hand-played" note.
struct SlowDrift
{
    static constexpr int kN = 3;
    double fr[kN] {}, ph[kN] {}, am[kN] {};
    void init (Rng& rng, double periodSec, double lo, double hi)
    {
        double sumA = 0.0;
        for (int i = 0; i < kN; ++i)
        {
            if (periodSec > 0.0)
            {
                const int mLo = std::max (1, (int) std::ceil (lo * periodSec)), mHi = std::max (mLo, (int) std::floor (hi * periodSec));
                fr[i] = (double) rng.rangeInt (mLo, mHi) / periodSec;
            }
            else fr[i] = rng.range (lo, hi);
            ph[i] = rng.range (0.0, kTwoPi);
            am[i] = rng.range (0.4, 1.0);
            sumA += am[i];
        }
        for (int i = 0; i < kN; ++i) am[i] /= sumA;                       // |eval| <= 1
    }
    double eval (double t) const
    {
        double s = 0.0;
        for (int i = 0; i < kN; ++i) s += am[i] * std::sin (kTwoPi * fr[i] * t + ph[i]);
        return s;
    }
};

// Phase delay (samples) of the one-pole low-pass of OnePole at angular frequency w.
double onePolePhaseDelay (const OnePole& f, double w)
{
    return std::atan2 (f.a * std::sin (w), 1.0 - f.a * std::cos (w)) / w;
}

// ------------------------------------------------------------------------------------------------
//  Violin body: air mode, main wood modes, mid-range plateau, bridge hill, high-frequency roll-off.
//  Fixed in frequency (it does not follow the note), which is what makes a violin sound like a violin.
// ------------------------------------------------------------------------------------------------
class ViolinBody
{
public:
    // dark: 0 = bright (forte), 1 = dark (pianissimo / soft layer)
    ViolinBody (double sr, double dark)
    {
        sec.push_back (Biquad::highpass (170.0, 0.65, sr));
        struct Pk { double f, g, q; };
        static const Pk modes[] = {
            { 275.0, 5.5, 7.0 }, { 405.0, 3.0, 5.0 }, { 472.0, 5.0, 6.5 }, { 556.0, 3.5, 6.0 }, { 668.0, 2.0, 5.0 },
            { 785.0, 3.0, 6.0 }, { 940.0, 2.5, 5.0 }, { 1130.0, 2.0, 5.0 }, { 1350.0, 2.5, 4.0 }, { 1620.0, 1.5, 3.5 },
        };
        for (const auto& m : modes) sec.push_back (Biquad::peaking (m.f, m.q, m.g, sr));
        sec.push_back (Biquad::peaking (2750.0, 1.1, 8.0 - 3.0 * dark, sr));          // bridge hill
        sec.push_back (Biquad::peaking (3650.0, 2.0, 3.0 - 1.5 * dark, sr));
        sec.push_back (Biquad::highShelf (5200.0, -7.0 - 4.0 * dark, sr));
        sec.push_back (Biquad::lowpass (9000.0 - 3500.0 * dark, 0.6, sr));
    }
    double process (double x) { for (auto& b : sec) x = b.process (x); return x; }
    void run (std::vector<float>& v) { for (auto& s : v) s = (float) process ((double) s); }

private:
    std::vector<Biquad> sec;
};

// ------------------------------------------------------------------------------------------------
//  Bowed string waveguide
// ------------------------------------------------------------------------------------------------
struct BowParams
{
    double f0 = 440.0;
    double beta = 0.1;               // bow position, fraction of the sounding string length from the bridge
    double slopeK = 3.0;             // friction-table slope = slopeK * beta  (bow speed = 1): larger = lighter bow force
    double bridgeFc = 5000.0;        // bridge reflection low-pass (Hz): corner rounding / brightness
    double nutFc = 8000.0;           // finger / nut reflection low-pass (Hz)
    double attackSec = 0.04;         // bow speed ramp
    double scoopCents = 0.0;         // start flat, rise to pitch (bow capture)
    double scoopSec = 0.04;
    double noiseDepth = 0.05;        // multiplicative noise on the friction slope (bow hair / rosin hiss)
    double noiseFc = 3000.0;
    double vibHz = 0.0, vibCents = 0.0, vibStart = 0.0, vibRamp = 0.3;
    double driftCents = 1.5;         // slow pitch wander (peak, cents)
    double driftAmp = 0.03;          // slow bow-speed wander (relative)
    double tune = 1.0;               // delay scale (pitch calibration)
    int controlPeriod = 0;           // frames at sr: drift + noise are periodic in this length (0: free)
    int os = 1;                      // internal oversampling
    std::uint64_t seed = 1;
};

// Returns the bridge-side wave (force on the bridge) at the base sample rate, `frames` long.
std::vector<float> simulateBow (const BowParams& p, double sr, int frames)
{
    const int os = std::max (1, p.os);
    const double srI = sr * (double) os;
    const long long N = (long long) frames * os;
    Rng rng (p.seed);
    const double periodSec = p.controlPeriod > 0 ? (double) p.controlPeriod / sr : 0.0;
    SlowDrift pitchDrift, speedDrift;
    pitchDrift.init (rng, periodSec, 0.7, 4.0);
    speedDrift.init (rng, periodSec, 0.5, 3.0);
    const int nzLen = p.controlPeriod > 0 ? p.controlPeriod : frames;
    const std::vector<float> nz = noiseTrack (rng, nzLen, p.noiseFc, sr, p.controlPeriod > 0);

    OnePole fb, fn;
    fb.setLowpass (p.bridgeFc, srI);
    fn.setLowpass (p.nutFc, srI);
    const double w0 = kTwoPi * p.f0 / srI;
    const double tauF = onePolePhaseDelay (fb, w0) + onePolePhaseDelay (fn, w0);
    const double base0 = srI / p.f0 * p.tune - tauF - 2.0;
    const double gB = 0.9965, gN = 0.998;
    DelayLine bl, nl;
    const int cap = (int) std::ceil (base0 * 1.1) + 16;
    bl.init (cap); nl.init (cap);

    const double slope0 = p.slopeK * p.beta;
    std::vector<float> out ((std::size_t) N);

    // slow controls (pitch wander, scoop, vibrato, bow speed) are evaluated every kBlk samples and interpolated
    constexpr long long kBlk = 32;
    auto controls = [&] (long long n, double& scale, double& speed)
    {
        const double t = (double) n / srI;
        double cents = p.driftCents * pitchDrift.eval (t);
        if (p.scoopCents != 0.0) cents -= p.scoopCents * std::exp (-t / p.scoopSec);
        if (p.vibHz > 0.0) cents += p.vibCents * smoothstep ((t - p.vibStart) / p.vibRamp) * std::sin (kTwoPi * p.vibHz * t);
        scale = std::exp2 (-cents / 1200.0);
        speed = smoothstep (t / p.attackSec) * (1.0 + p.driftAmp * speedDrift.eval (t));
    };
    double sc0 = 1.0, sp0 = 0.0, sc1 = 1.0, sp1 = 0.0;
    controls (0, sc0, sp0);
    controls (kBlk, sc1, sp1);
    for (long long n = 0; n < N; ++n)
    {
        const long long ph = n % kBlk;
        if (n > 0 && ph == 0) { sc0 = sc1; sp0 = sp1; controls (n + kBlk, sc1, sp1); }
        const double u = (double) ph * (1.0 / (double) kBlk);
        const double sc = sc0 + (sc1 - sc0) * u, vb = sp0 + (sp1 - sp0) * u;
        const double base = base0 * sc;
        const double dB = std::max (1.0, p.beta * base), dN = std::max (1.0, (1.0 - p.beta) * base);

        const double ob = (double) bl.read (dB);
        const double on = (double) nl.read (dN);
        const double bIn = -gB * fb.lowpass (ob);
        const double nIn = -gN * fn.lowpass (on);

        const double d0 = vb - (bIn + nIn);
        const double noise = (double) nz[(std::size_t) ((n / os) % (long long) nz.size())];
        const double s = slope0 * std::max (0.2, 1.0 + p.noiseDepth * noise);
        const double xx = std::fabs (d0 * s) + 0.75;
        const double x2 = xx * xx;
        const double rho = std::min (1.0, 1.0 / (x2 * x2));
        const double vf = d0 * rho;
        bl.push ((float) (nIn + vf));
        nl.push ((float) (bIn + vf));
        out[(std::size_t) n] = (float) ob;
    }
    return decimateBy (out, os, (std::size_t) frames);
}

int oversampleFor (double f0, double sr)
{
    const double period = sr / f0;
    return period >= 60.0 ? 1 : (period >= 30.0 ? 2 : 4);
}

// Is the (pre-body) buffer a clean Helmholtz-like oscillation near f0?  fMeasured: mean frequency.
bool helmholtzOk (const std::vector<float>& x, double sr, double f0, double& fMeasured)
{
    const std::size_t a = (std::size_t) secondsToFrames (0.16, sr);
    if (x.size() < a + 2048) return false;
    double share = 0.0;
    fMeasured = measureF0Hz (x.data() + a, x.size() - a, sr, f0, &share);
    if (fMeasured <= 0.0) return false;
    if (std::fabs (centsBetween (fMeasured, f0)) > 80.0) return false;
    if (share < 0.28) return false;
    // periodicity: normalised autocorrelation at one period
    const double P = sr / fMeasured;
    const int lag = (int) std::lround (P);
    const std::size_t n = x.size() - a - (std::size_t) lag - 2;
    double best = 0.0;
    for (int l = lag - 1; l <= lag + 1; ++l)
    {
        double c = 0.0, e0 = 0.0, e1 = 0.0;
        for (std::size_t i = 0; i < n; ++i) { const double u = x[a + i], v = x[a + i + (std::size_t) l]; c += u * v; e0 += u * u; e1 += v * v; }
        best = std::max (best, c / std::sqrt (std::max (e0 * e1, 1.0e-30)));
    }
    return best > 0.97;
}

struct BowStyle
{
    double dAbs = 0.022;             // bow-bridge distance (metres)
    double slopeK = 2.6;
    double bridgeFc = 6500.0;
    double noiseDepth = 0.05;
    double attack = 0.035;
    double scoop = 6.0;
    double vibHz = 0.0, vibCents = 0.0, vibStart = 0.22, vibRamp = 0.28;
    double bodyDark = 0.0;
    double introSec = 0.30;
    double loopSec = 1.2;
};

double violinBeta (int note, double dAbs)
{
    // sounding string length of a violin string stopped at `note` (32.5 cm open strings G3 D4 A4 E5)
    const double openHz = note >= 76 ? 659.26 : (note >= 69 ? 440.0 : (note >= 62 ? 293.66 : 196.0));
    const double len = 0.325 * openHz / midiToHz ((double) note);
    return clampd (dAbs / len, 0.07, 0.22);
}

struct SustainResult { std::vector<float> data; int loopStart = 0, loopEnd = 0; double centsError = 0.0; };

SustainResult renderBowedNote (const BuildContext& ctx, int note, const BowStyle& st, int zoneIndex, int salt)
{
    const double sr = ctx.sampleRate;
    const double f0 = midiToHz ((double) note);

    // --- loop geometry: a whole number of string periods (and of vibrato cycles)
    int periods = std::max (8, (int) std::lround (st.loopSec * f0));
    double vibHz = 0.0;
    if (st.vibHz > 0.0)
    {
        double bestErr = 1.0e9;
        for (int c = 5; c <= 8; ++c)
        {
            const int k = (int) std::lround ((double) c * f0 / st.vibHz);
            const int len = (int) std::lround ((double) k * sr / f0);
            const double rate = (double) c * sr / (double) len;
            const double err = std::fabs (rate - st.vibHz) + 3.0 * std::max (0.0, std::fabs ((double) len / sr - st.loopSec) - 0.15);
            if (err < bestErr) { bestErr = err; periods = k; vibHz = rate; }
        }
    }
    const int L = (int) std::lround ((double) periods * sr / f0);
    const int Pint = (int) std::ceil (sr / f0);
    const int t0 = secondsToFrames (st.introSec, sr);
    const int total = t0 + L + 3 * Pint + 16;

    BowParams p;
    p.f0 = f0;
    p.beta = violinBeta (note, st.dAbs);
    p.slopeK = st.slopeK;
    p.bridgeFc = st.bridgeFc;
    p.nutFc = 0.5 * (st.bridgeFc + 7000.0);
    p.noiseDepth = st.noiseDepth * std::min (1.0, std::pow (400.0 / f0, 0.8));
    p.attackSec = st.attack;
    p.scoopCents = st.scoop;
    p.vibHz = vibHz; p.vibCents = st.vibCents; p.vibStart = st.vibStart; p.vibRamp = st.vibRamp;
    p.os = oversampleFor (f0, sr);
    p.seed = Rng::mix64 ((std::uint64_t) (zoneIndex + 1) * 7919u + (std::uint64_t) salt * 104729u + 11u);

    // --- find a clean Helmholtz regime and calibrate the pitch (delay scale)
    const double kForce[] = { 1.0, 0.82, 1.22, 0.66, 1.5, 0.5 };
    bool ok = false;
    for (double kf : kForce)
    {
        BowParams c = p;
        c.slopeK = st.slopeK * kf;
        c.vibHz = 0.0; c.driftCents = 0.0; c.driftAmp = 0.0; c.controlPeriod = 0;
        const int calFrames = secondsToFrames (0.40, sr);
        double fm = 0.0;
        auto x = simulateBow (c, sr, calFrames);
        if (! helmholtzOk (x, sr, f0, fm)) continue;
        c.tune *= fm / f0;                                   // longer delay -> lower pitch
        x = simulateBow (c, sr, calFrames);
        if (! helmholtzOk (x, sr, f0, fm)) continue;
        c.tune *= fm / f0;
        p.slopeK = c.slopeK; p.tune = c.tune;
        ok = true;
        break;
    }
    (void) ok;

    p.controlPeriod = L;
    std::vector<float> y = simulateBow (p, sr, total);

    // --- body, DC, start
    ViolinBody body (sr, st.bodyDark);
    body.run (y);
    sanitize (y, false);
    fadeIn (y, std::max (8, secondsToFrames (0.0004, sr)));

    // --- loop start: rising zero crossing with a small step (never on a slip pulse); loop end: best waveform
    //     match around start + L (the controls are periodic, so it is close already)
    int s = t0;
    {
        double bestStep = 1.0e30;
        for (int i = t0; i < t0 + Pint; ++i)
        {
            if (y[(std::size_t) i - 1] <= 0.f && y[(std::size_t) i] > 0.f)
            {
                const double step = (double) y[(std::size_t) i] - (double) y[(std::size_t) i - 1];
                if (step < bestStep) { bestStep = step; s = i; }
            }
        }
    }
    const int W = std::min (2 * Pint + 16, 700);
    double bestM = 1.0e30;
    int bestE = s + L - 1;
    for (int e = s + L - 1 - Pint; e <= s + L - 1 + Pint; ++e)
    {
        double d = 0.0;
        for (int i = 0; i < W; ++i) { const double df = (double) y[(std::size_t) (e - i)] - (double) y[(std::size_t) (s - 1 - i)]; d += df * df; }
        if (d < bestM) { bestM = d; bestE = e; }
    }
    SustainResult r;
    y.resize ((std::size_t) bestE + 1);
    const int fade = std::max (64, secondsToFrames (0.015, sr));
    crossfadeLoop (y, s, bestE, fade, false);
    r.data = std::move (y);
    r.loopStart = s; r.loopEnd = bestE;
    return r;
}

Zone bowedZone (const BuildContext& ctx, const char* articulation, const KeySpan& sp, const VelLayer& vl, int swLast, const BowStyle& st,
                double loudDb, int zoneIndex, float attack, float release, float veltrack)
{
    Zone z;
    z.articulation = articulation;
    z.name = zoneName (articulation, sp.center, vl.lo == 1 && vl.hi == 127 ? nullptr : vl.name);
    z.sampleRate = ctx.sampleRate;
    z.lokey = sp.lokey; z.hikey = sp.hikey; z.keycenter = sp.center;
    z.lovel = vl.lo; z.hivel = vl.hi; z.swLast = swLast;
    z.ampAttack = attack; z.ampRelease = release; z.ampVeltrack = veltrack;
    SustainResult r = renderBowedNote (ctx, sp.center, st, zoneIndex, 0);
    z.data = std::move (r.data);
    finishLoop (z, r.loopStart, r.loopEnd, Loop::sustain);
    normaliseLoudness (z.data, ctx.sampleRate, loudDb, kPeakCeilingDb);
    return z;
}

// ------------------------------------------------------------------------------------------------
//  Pizzicato: two slightly detuned polarisations of a plucked string, through the same body.
// ------------------------------------------------------------------------------------------------
std::vector<float> renderPizz (int note, double sr, Rng& rng)
{
    const double f0 = midiToHz ((double) note);
    const double t60 = clampd (1.25 * std::pow (196.0 / f0, 0.5), 0.22, 1.5);
    const int total = secondsToFrames (std::min (1.35, 0.1 + 1.05 * t60), sr);
    KarplusString a, b;
    const double det = rng.range(0.4, 1.1);                              // cents between the polarisations
    a.init (f0 * std::pow (2.0, 0.5 * det / 1200.0), sr, t60, 0.62);
    b.init (f0 * std::pow (2.0, -0.5 * det / 1200.0), sr, t60 * 0.6, 0.5);
    const int N = std::max (a.loopDelay(), b.loopDelay());
    // plucked shape: triangular displacement peaking at the pluck point, softened by the finger
    const double pickPos = rng.range (0.12, 0.2);
    std::vector<float> e ((std::size_t) N + 8, 0.f);
    const int pk = std::max (2, (int) std::lround (pickPos * (double) N));
    for (int i = 0; i < N; ++i)
        e[(std::size_t) i] = (float) (i < pk ? (double) i / (double) pk : (double) (N - i) / (double) (N - pk));
    {
        double mean = 0.0;
        for (int i = 0; i < N; ++i) mean += e[(size_t) i];
        mean /= (double) N;
        for (int i = 0; i < N; ++i) e[(size_t) i] -= (float) mean;
    }
    // finger release snap: broadband noise burst in the first few ms
    std::vector<float> snap ((std::size_t) secondsToFrames (0.012, sr));
    {
        Biquad bp = Biquad::bandpass (3200.0, 0.7, sr);
        for (std::size_t i = 0; i < snap.size(); ++i)
            snap[i] = (float) (bp.process (rng.bipolar()) * std::exp (-(double) i / (0.003 * sr)));
    }
    std::vector<float> out ((std::size_t) total);
    for (int n = 0; n < total; ++n)
    {
        const float ex = n < N ? e[(std::size_t) n] : 0.f;
        const float ex2 = n < N ? e[(std::size_t) n] * 0.7f : 0.f;
        const float sn = (std::size_t) n < snap.size() ? snap[(std::size_t) n] * 0.02f : 0.f;
        out[(std::size_t) n] = a.tick (ex + sn) + 0.6f * b.tick (ex2);
    }
    ViolinBody body (sr, 0.3);
    body.run (out);
    return out;
}

// Bow lift / string settle: a faint scrape tick followed by a short body thump.
std::vector<float> renderBowLift (double sr, Rng& rng, double lengthSec)
{
    const int n = secondsToFrames (lengthSec, sr);
    std::vector<float> v ((std::size_t) n);
    Biquad bp = Biquad::bandpass (2600.0, 0.9, sr);
    Biquad hp = Biquad::highpass (900.0, 0.7, sr);
    Resonator thump;
    thump.set (285.0, 0.07, sr);
    for (int i = 0; i < n; ++i)
    {
        const double t = (double) i / sr;
        const double nz = rng.bipolar();
        const double scrape = hp.process (bp.process (nz)) * (std::exp (-t / 0.018) * smoothstep (t / 0.002));
        const double tk = thump.process (i == 4 ? 1.0 : 0.0) * 0.5;
        v[(std::size_t) i] = (float) (scrape + 0.35 * tk);
    }
    ViolinBody body (sr, 0.5);
    body.run (v);
    return v;
}

} // namespace

// =================================================================================================
Library buildViolon (const BuildContext& ctx)
{
    const auto spans = makeKeySpans (ctx.spec().lo, ctx.spec().hi, 3);
    Library lib = startLibrary (ctx);
    ProgressTracker pt (ctx, (int) spans.size() * 4 + 1);
    int zi = 0;

    BowStyle hard;                                    // forte: close to the bridge, firm bow, bright
    hard.dAbs = 0.022; hard.slopeK = 2.4; hard.bridgeFc = 6800.0; hard.noiseDepth = 0.05; hard.attack = 0.032; hard.scoop = 5.0;
    hard.bodyDark = 0.0; hard.introSec = 0.30;
    BowStyle soft = hard;                             // piano: further from the bridge, lighter bow, darker
    soft.dAbs = 0.034; soft.slopeK = 3.4; soft.bridgeFc = 3600.0; soft.noiseDepth = 0.08; soft.attack = 0.065; soft.scoop = 9.0;
    soft.bodyDark = 0.9; soft.introSec = 0.36;
    BowStyle vib = hard;                              // mezzo-forte with vibrato
    vib.dAbs = 0.027; vib.slopeK = 2.9; vib.bridgeFc = 5200.0; vib.noiseDepth = 0.06; vib.attack = 0.05; vib.scoop = 8.0;
    vib.vibHz = 5.5; vib.vibCents = 20.0; vib.vibStart = 0.18; vib.vibRamp = 0.30; vib.bodyDark = 0.4; vib.introSec = 0.62; vib.loopSec = 1.15;

    for (const auto& sp : spans)
    {
        if (! pt.next()) { lib.cancelled = true; return lib; }
        lib.zones.push_back (bowedZone (ctx, "sustain", sp, kSoft, 12, soft, kSoftLoudnessDb, zi++, 0.04f, 0.25f, 60.f));
        lib.zones.push_back (bowedZone (ctx, "sustain", sp, kHard, 12, hard, kHardLoudnessDb, zi++, 0.03f, 0.25f, 60.f));

        if (! pt.next()) { lib.cancelled = true; return lib; }
        lib.zones.push_back (bowedZone (ctx, "vibrato", sp, kAllVel, 13, vib, -17.0, zi++, 0.04f, 0.25f, 60.f));

        if (! pt.next()) { lib.cancelled = true; return lib; }
        Rng rng = Rng::forZone (ctx.instrument, zi++);
        Zone z;
        z.articulation = "pizz"; z.name = zoneName ("pizz", sp.center);
        z.sampleRate = ctx.sampleRate;
        z.lokey = sp.lokey; z.hikey = sp.hikey; z.keycenter = sp.center;
        z.lovel = kAllVel.lo; z.hivel = kAllVel.hi; z.swLast = 14;
        z.ampRelease = 0.2f; z.ampVeltrack = 70.f;
        z.data = renderPizz (sp.center, ctx.sampleRate, rng);
        finishOneShot (z.data, ctx.sampleRate);
        normaliseLoudness (z.data, ctx.sampleRate, -18.0, kPeakCeilingDb);
        lib.zones.push_back (std::move (z));
    }

    // Bow-lift / string-settle release noise (release trigger), one zone per sustained articulation (12, 13).
    {
        if (! pt.next()) { lib.cancelled = true; return lib; }
        Rng rng = Rng::forZone (ctx.instrument, zi++);
        Zone z;
        z.articulation = "bowlift"; z.name = "bowlift_12";
        z.sampleRate = ctx.sampleRate;
        z.lokey = ctx.spec().lo; z.hikey = ctx.spec().hi; z.keycenter = 60;
        z.swLast = 12; z.triggerRelease = true; z.loop = Loop::oneShot;
        z.pitchKeytrack = 0.f; z.volumeDb = -14.f; z.ampRelease = 0.03f; z.ampVeltrack = 40.f;
        z.data = renderBowLift (ctx.sampleRate, rng, 0.16);
        finishOneShot (z.data, ctx.sampleRate, false, 0.03);
        normalisePeak (z.data, -9.0);
        lib.zones.push_back (z);

        Zone a = z;                                       // same audio for the vibrato articulation
        a.data.clear(); a.data.shrink_to_fit();
        a.sharedWith = (int) lib.zones.size() - 1;
        a.name = "bowlift_13"; a.swLast = 13;
        lib.zones.push_back (std::move (a));
    }
    pt.finished();
    return lib;
}

// ---------------------------------------------------------------------------------------------
// TEMPORARY stub for the ney (kept until the real model is written).
namespace
{
struct SustainStyle
{
    double rolloff = 1.2;          // harmonic amplitude ~ h^-rolloff
    double formant1Hz = 600.0, formant1Gain = 2.0, formant1Width = 400.0;
    double formant2Hz = 2500.0, formant2Gain = 1.0, formant2Width = 900.0;
    double noiseAmount = 0.05;     // bow / breath noise relative to the tone
    double noiseCentreHz = 3000.0;
    double attackSec = 0.06;
    double introSec = 0.20;        // material before the loop
    double bodySec = 1.0;          // approximate loop length
    double vibratoHz = 0.0;        // 0 = none
    double vibratoDepth = 0.0;     // relative frequency deviation
    int maxHarmonics = 18;
};

struct Sustained { std::vector<float> data; int loopStart = 0, loopEnd = 0; };

// Tone with exactly periodic body + separately cross-faded noise.  The loop length is a whole number of
// fundamental periods (and of vibrato cycles), the fundamental is nudged by < 4 cents to make that possible.
Sustained renderSustained (double f0, double sr, const SustainStyle& st, double layerGain, double layerRolloffAdd, Rng& rng)
{
    const bool vib = st.vibratoHz > 0.0;
    int L = 0;
    double f0p = f0, fv = st.vibratoHz;
    if (vib)
    {
        double bestErr = 1.0e9;
        for (int cv = 3; cv <= 9; ++cv)
        {
            const int l = (int) std::lround (sr * (double) cv / st.vibratoHz);
            const double k = std::max (1.0, std::round (f0 * (double) l / sr));
            const double adj = k * sr / (double) l;
            const double e = std::fabs (centsBetween (adj, f0)) + 0.4 * std::fabs ((double) l / sr - st.bodySec);
            if (e < bestErr) { bestErr = e; L = l; f0p = adj; fv = (double) cv * sr / (double) l; }
        }
    }
    else
    {
        const double k = std::max (1.0, std::round (f0 * st.bodySec));
        L = (int) std::lround (k * sr / f0);
        f0p = k * sr / (double) L;
    }

    const int fade = std::max (64, secondsToFrames (0.012, sr));
    Sustained out;
    out.loopStart = std::max (secondsToFrames (st.introSec, sr), fade + 8);
    out.loopEnd = out.loopStart + L - 1;
    const int total = out.loopEnd + 1 + 8;

    // harmonic amplitudes
    const int nh = std::max (1, std::min (st.maxHarmonics, (int) std::floor (0.45 * sr / f0p)));
    std::vector<double> amp ((std::size_t) nh), ph0 ((std::size_t) nh);
    for (int h = 1; h <= nh; ++h)
    {
        const double fh = f0p * (double) h;
        const double f1 = (fh - st.formant1Hz) / st.formant1Width, f2 = (fh - st.formant2Hz) / st.formant2Width;
        amp[(std::size_t) h - 1] = std::pow ((double) h, -(st.rolloff + layerRolloffAdd)) * (1.0 + st.formant1Gain * std::exp (-f1 * f1) + st.formant2Gain * std::exp (-f2 * f2));
        ph0[(std::size_t) h - 1] = rng.range (0.0, kTwoPi);
    }

    std::vector<float> tone ((std::size_t) total), noise ((std::size_t) total);
    const double dPhi = kTwoPi * f0p / sr;
    for (int n = 0; n < total; ++n)
    {
        const double t = (double) n / sr;
        double vphase = 0.0, ampMod = 1.0;
        if (vib)
        {
            vphase = -(f0p * st.vibratoDepth / fv) * std::cos (kTwoPi * fv * t);
            ampMod = 1.0 + 0.08 * std::sin (kTwoPi * fv * t);
        }
        double s = 0.0;
        for (int h = 1; h <= nh; ++h)
            s += amp[(std::size_t) h - 1] * std::sin ((double) h * (dPhi * (double) n + vphase) + ph0[(std::size_t) h - 1]);
        const double att = st.attackSec > 0.0 ? smoothstep (t / st.attackSec) : 1.0;
        tone[(std::size_t) n] = (float) (s * att * ampMod);
    }

    // noise: band-limited around noiseCentreHz, with an onset "scratch/chiff"
    Biquad bp = Biquad::bandpass (st.noiseCentreHz, 0.8, sr);
    for (int n = 0; n < total; ++n)
    {
        const double t = (double) n / sr;
        const double att = st.attackSec > 0.0 ? smoothstep (t / (0.7 * st.attackSec)) : 1.0;
        const double onset = 1.0 + 2.5 * std::exp (-t / 0.05);
        noise[(std::size_t) n] = (float) (bp.process (rng.bipolar()) * att * onset);
    }
    // calibrate the noise to the tone, then make the noise loop seamless (equal power, uncorrelated)
    double te = 0.0, ne = 0.0;
    for (int n = out.loopStart; n <= out.loopEnd; ++n) { te += (double) tone[(std::size_t) n] * (double) tone[(std::size_t) n]; ne += (double) noise[(std::size_t) n] * (double) noise[(std::size_t) n]; }
    const double ng = st.noiseAmount * std::sqrt (te / std::max (ne, 1.0e-12));
    for (auto& v : noise) v = (float) ((double) v * ng);
    crossfadeLoop (noise, out.loopStart, out.loopEnd, fade, true);

    out.data.resize ((std::size_t) total);
    for (int n = 0; n < total; ++n) out.data[(std::size_t) n] = (float) ((double) tone[(std::size_t) n] + (double) noise[(std::size_t) n]) * (float) layerGain;
    return out;
}

Zone sustainZone (const BuildContext& ctx, const char* articulation, const KeySpan& sp, const VelLayer& vl, int swLast, const SustainStyle& st,
                  double rolloffAdd, double loudnessDb, int zoneIndex, float attack, float release)
{
    Zone z;
    z.articulation = articulation;
    z.name = zoneName (articulation, sp.center, vl.lo == 1 && vl.hi == 127 ? nullptr : vl.name);
    z.sampleRate = ctx.sampleRate;
    z.lokey = sp.lokey; z.hikey = sp.hikey; z.keycenter = sp.center;
    z.lovel = vl.lo; z.hivel = vl.hi; z.swLast = swLast;
    z.ampAttack = attack; z.ampRelease = release; z.ampVeltrack = 60.f;
    Rng rng = Rng::forZone (ctx.instrument, zoneIndex);
    Sustained s = renderSustained (midiToHz (sp.center), ctx.sampleRate, st, 1.0, rolloffAdd, rng);
    z.data = std::move (s.data);
    fadeIn (z.data, std::max (8, secondsToFrames (0.0004, ctx.sampleRate)));
    finishLoop (z, s.loopStart, s.loopEnd, Loop::sustain);
    // loudness over the loop body (the 300 ms window is inside the sustain)
    normaliseLoudness (z.data, ctx.sampleRate, loudnessDb, kPeakCeilingDb);
    return z;
}


} // namespace

Library buildNey (const BuildContext& ctx)
{
    const auto spans = makeKeySpans (ctx.spec().lo, ctx.spec().hi, 3);
    Library lib = startLibrary (ctx);
    ProgressTracker pt (ctx, (int) spans.size() * 3);
    int zi = 0;

    SustainStyle clear;                       // flute-like: strong fundamental, few harmonics, breath noise
    clear.rolloff = 2.2; clear.formant1Gain = 0.6; clear.formant2Gain = 0.0; clear.noiseAmount = 0.10; clear.noiseCentreHz = 4200.0;
    clear.attackSec = 0.08; clear.introSec = 0.25; clear.bodySec = 1.0; clear.maxHarmonics = 10;
    SustainStyle breathy = clear;
    breathy.noiseAmount = 0.55; breathy.rolloff = 2.6; breathy.noiseCentreHz = 3300.0;

    for (const auto& sp : spans)
    {
        for (const VelLayer* vl : { &kSoft, &kHard })
        {
            if (! pt.next()) { lib.cancelled = true; return lib; }
            lib.zones.push_back (sustainZone (ctx, "sustain", sp, *vl, 12, clear, vl == &kSoft ? 0.8 : 0.0, vl == &kSoft ? kSoftLoudnessDb : kHardLoudnessDb, zi++, 0.06f, 0.20f));
        }
        if (! pt.next()) { lib.cancelled = true; return lib; }
        lib.zones.push_back (sustainZone (ctx, "breathy", sp, kAllVel, 13, breathy, 0.3, -18.0, zi++, 0.08f, 0.25f));
    }
    pt.finished();
    return lib;
}

} // namespace humpire::model
