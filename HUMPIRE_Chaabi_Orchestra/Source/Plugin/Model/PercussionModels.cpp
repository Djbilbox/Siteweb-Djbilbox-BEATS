// ============================================================================
//  PERCUSSION FAMILY: Derbouka, Tar, Bendir  (modelled, not recorded).
//
//  Model (all strokes):
//   * Circular-membrane modal synthesis.  Twelve (m,n) modes with the Bessel-zero frequency ratios
//     (1, 1.59, 2.14, 2.30, 2.65 ...).  A stroke at normalised radius r and angle theta excites mode (m,n)
//     with weight |J_m(j_mn r) cos(m theta)|, so centre strokes ring "round" and edge strokes are thin.
//     Each mode has its own decay (higher modes die faster) and a small tension glide after a hard hit.
//     For the derbouka goblet the ratios are blended towards a harmonic series (the open throat couples
//     the head to the air column, which is why a derbouka DUM has a clear pitch).
//   * Air cavity (Helmholtz style boom) as one extra decaying low mode.
//   * Strike transient: band-passed noise burst (finger / palm) plus short ringing rim/shell partials.
//   * Velocity changes timbre: soft hits have weaker upper modes, less noise, less glide.
//   * Tar: bank of jingle (zil) resonators, inharmonic partials 3-13 kHz, driven by the head's
//     acceleration (strokes) or by a random impact process (ZIL shake).
//   * Bendir: gut snare = high-passed noise gated by the head's amplitude envelope, with a release that
//     outlasts the head; finger roll = a train of small strokes with a sustained buzz.
//  Round robins vary radius, angle, pitch, damping, noise colour and seed.  Everything is seeded and
//  deterministic.  No dependencies other than ModelCommon.h.
// ============================================================================
#include "ModelCommon.h"

namespace humpire::model
{
namespace
{
struct ModeDef { int m; double zero; };
constexpr ModeDef kModes[12] = {
    { 0, 2.4048 }, { 1, 3.8317 }, { 2, 5.1356 }, { 0, 5.5201 }, { 3, 6.3802 }, { 1, 7.0156 },
    { 4, 7.5883 }, { 2, 8.4172 }, { 0, 8.6537 }, { 5, 8.7715 }, { 3, 9.7610 }, { 1, 10.1735 } };

double besselJ (int m, double x)
{
    double term = std::pow (x * 0.5, m);
    for (int i = 2; i <= m; ++i) term /= (double) i;
    double sum = term;
    for (int k = 1; k < 50; ++k)
    {
        term *= -(x * x * 0.25) / ((double) k * (double) (k + m));
        sum += term;
    }
    return sum;
}

struct Params
{
    double length = 0.3;
    // membrane
    double f0 = 150.0, t60 = 0.3, dampSlope = 0.6, radius = 0.4, harm = 0.0, glide = 0.0, glideTau = 0.04;
    double tilt = 0.6, gain = 1.0;                        // tilt: how strongly softness removes upper modes
    int nModes = 8;
    // strike transient
    double clickHz = 2500.0, clickQ = 1.0, clickTau = 0.01, clickGain = 0.3;
    // rim / shell ring partials
    double ringHz[3] = { 0.0, 0.0, 0.0 };
    double ringT60 = 0.1, ringGain = 0.0;
    // air cavity
    double cavHz = 0.0, cavT60 = 0.3, cavGain = 0.0;
    // jingles (tar): coupling from the head, T60 scale
    double jingleDrive = 0.0, jingleT60 = 1.0;
    double shake = 0.0, shakeTau = 0.45;                  // ZIL shake gain / amplitude decay
    // snare buzz (bendir)
    double buzz = 0.0, buzzSens = 2.0, buzzRelease = 0.07, buzzHz = 4200.0;
    // finger roll (bendir)
    double rollRate = 0.0, rollDur = 0.7;
};

struct Lane
{
    const char* name;
    int lokey, hikey;
    bool twoLayers;
    int roundRobins;
    int group, offBy;
    double singleLoudnessDb;
    Params p;
};

// Adds one membrane stroke (modes + cavity + transient + rim rings) into m at offset off.
void addStroke (std::vector<double>& m, int off, const Params& p, double h, double sr, Rng& rng, double amp)
{
    const int n = (int) m.size();
    const double f0 = p.f0 * (1.0 + 0.02 * rng.bipolar()) * (0.97 + 0.05 * h);
    const double radius = clampd (p.radius + 0.06 * rng.bipolar(), 0.0, 0.93);
    const double theta = rng.range (0.0, kTwoPi);
    const int nm = std::min (p.nModes, 12);
    const double z0 = kModes[0].zero;

    for (int k = 0; k < nm; ++k)
    {
        const double memRatio = kModes[k].zero / z0;
        const double ratio = lerp (memRatio, (double) (k + 1) * 1.0, p.harm);
        double w = besselJ (kModes[k].m, kModes[k].zero * radius);
        if (kModes[k].m > 0) w *= std::cos ((double) kModes[k].m * theta);
        double a = std::fabs (w) * std::pow (h, p.tilt * (ratio - 1.0)) / std::pow (ratio, 0.6);
        if (k == 0) a *= 1.3;
        if (a < 2.0e-3) continue;
        const double fk = f0 * ratio * (1.0 + 0.004 * rng.bipolar());
        const double t60k = p.t60 / (1.0 + p.dampSlope * (ratio - 1.0)) * rng.range (0.92, 1.08);
        const int len = std::min (n - off, (int) (t60k * 1.3 * sr));
        const double dec = std::pow (10.0, -3.0 / (t60k * sr));
        const double gdec = std::exp (-1.0 / (p.glideTau * sr));
        double env = amp * a, g = p.glide * h, ph = 0.0;
        for (int i = 0; i < len; ++i)
        {
            ph += kTwoPi * fk * (1.0 + g) / sr;
            m[(std::size_t) (off + i)] += env * std::sin (ph);
            env *= dec;
            g *= gdec;
        }
    }

    // air cavity boom
    if (p.cavGain > 0.0 && p.cavHz > 0.0)
    {
        const double fc = p.cavHz * (1.0 + 0.01 * rng.bipolar());
        const int len = std::min (n - off, (int) (p.cavT60 * 1.3 * sr));
        const double dec = std::pow (10.0, -3.0 / (p.cavT60 * sr));
        double env = amp * p.cavGain * (0.3 + 0.7 * h), ph = 0.0;
        for (int i = 0; i < len; ++i)
        {
            ph += kTwoPi * fc / sr;
            m[(std::size_t) (off + i)] += env * std::sin (ph);
            env *= dec;
        }
    }

    // rim / shell rings
    if (p.ringGain > 0.0)
        for (double hz : p.ringHz)
        {
            if (hz <= 0.0) continue;
            const double fk = hz * rng.range (0.97, 1.03);
            const double t = p.ringT60 * rng.range (0.8, 1.2);
            const int len = std::min (n - off, (int) (t * 1.3 * sr));
            const double dec = std::pow (10.0, -3.0 / (t * sr));
            double env = amp * p.ringGain * (0.2 + 0.8 * h) * rng.range (0.7, 1.0), ph = rng.range (0.0, 1.0);
            for (int i = 0; i < len; ++i)
            {
                ph += kTwoPi * fk / sr;
                m[(std::size_t) (off + i)] += env * std::sin (ph);
                env *= dec;
            }
        }

    // finger / palm noise burst
    if (p.clickGain > 0.0)
    {
        Biquad bp = Biquad::bandpass (clampd (p.clickHz * rng.range (0.9, 1.1) * (0.8 + 0.4 * h), 100.0, sr * 0.45), p.clickQ, sr);
        const double dec = std::exp (-1.0 / (p.clickTau * (0.7 + 0.3 * h) * sr));
        const int len = std::min (n - off, (int) (p.clickTau * 9.0 * sr));
        double env = amp * p.clickGain * (0.15 + 0.85 * h);
        for (int i = 0; i < len; ++i)
        {
            m[(std::size_t) (off + i)] += env * bp.process (rng.bipolar());
            env *= dec;
        }
    }
}

// Jingle bank: fixed layout per instrument (always the same metal), 12 jingles x 2 partials.
struct JingleBank
{
    static constexpr int kN = 24;
    Resonator r[kN];
    double weight[kN];
    JingleBank (double sr, double t60Scale, Rng& var)
    {
        Rng fixed (0x7A5Cull);
        for (int j = 0; j < kN / 2; ++j)
        {
            const double f = 3000.0 * std::pow (2.9, fixed.uniform());          // 3.0 .. 8.7 kHz
            const double t = (0.35 + 0.65 * fixed.uniform()) * t60Scale;
            const double f2 = f * fixed.range (2.35, 3.1);
            r[2 * j].set (f * var.range (0.995, 1.005), t * var.range (0.9, 1.1), sr);
            r[2 * j + 1].set (f2 * var.range (0.995, 1.005), t * 0.55 * var.range (0.9, 1.1), sr);
            weight[2 * j] = 0.5 + 0.5 * fixed.uniform();
            weight[2 * j + 1] = 0.35 * weight[2 * j];
        }
    }
};

std::vector<float> renderHit (const Params& p, double sr, double h, Rng& rng)
{
    const int n = secondsToFrames (p.length, sr);
    std::vector<double> m ((std::size_t) n, 0.0);

    if (p.rollRate > 0.0)
    {
        // finger roll: train of small strokes
        Params s = p;
        s.t60 = 0.07; s.clickGain *= 0.6; s.glide = 0.0; s.cavGain = 0.0; s.ringGain *= 0.4;
        double t = 0.0;
        while (t < p.rollDur)
        {
            const double u = t / p.rollDur;
            const double envA = 0.45 + 0.55 * std::sin (kPi * std::pow (u, 0.7));
            addStroke (m, (int) (t * sr), s, h * rng.range (0.55, 0.9), sr, rng, envA * rng.range (0.6, 1.0));
            t += (1.0 / p.rollRate) * (1.0 + 0.2 * rng.bipolar());
        }
    }
    else
        addStroke (m, 0, p, h, sr, rng, p.gain);

    // jingles driven by the head (tar strokes)
    if (p.jingleDrive > 0.0 || p.shake > 0.0)
    {
        JingleBank jb (sr, p.jingleT60, rng);
        double prev = 0.0;
        double ev[JingleBank::kN];
        const double pk = 1.0;
        std::vector<double> jo ((std::size_t) n, 0.0);
        for (int i = 0; i < n; ++i)
        {
            const double t = (double) i / sr;
            const double d = (m[(std::size_t) i] - prev) * p.jingleDrive * pk;
            prev = m[(std::size_t) i];
            for (double& e : ev) e = 0.0;
            bool any = false;
            if (p.shake > 0.0)
            {
                const double env = (1.0 - std::exp (-t / 0.012)) * std::exp (-t / p.shakeTau)
                                   * (0.7 + 0.3 * std::cos (kTwoPi * 11.0 * t + 0.5));
                if (rng.uniform() < 150.0 * env / sr + 0.0)
                {
                    const double a = p.shake * env * (0.3 + rng.uniform());
                    for (double& e : ev) e = a * (0.2 + 0.8 * rng.uniform());
                    any = true;
                }
            }
            double s = 0.0;
            for (int j = 0; j < JingleBank::kN; ++j)
                s += jb.weight[j] * jb.r[j].process (d + (any ? ev[j] : 0.0));
            jo[(std::size_t) i] = s;
        }
        for (int i = 0; i < n; ++i) m[(std::size_t) i] += jo[(std::size_t) i] * (0.4 + 0.6 * h);
    }

    // snare buzz gated by the head's amplitude
    if (p.buzz > 0.0)
    {
        Biquad hp = Biquad::highpass (1800.0, 0.7, sr);
        Biquad bp = Biquad::bandpass (p.buzzHz * rng.range (0.95, 1.05), 0.7, sr);
        const double atk = std::exp (-1.0 / (0.003 * sr)), rel = std::exp (-1.0 / (p.buzzRelease * sr));
        double e = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double a = std::fabs (m[(std::size_t) i]);
            e = a > e ? a + (e - a) * atk : e * rel;
            double g = e * p.buzzSens;
            g = g / (1.0 + g);
            const double x = rng.bipolar();
            m[(std::size_t) i] += p.buzz * (0.35 + 0.65 * h) * g * (0.55 * hp.process (x) + 0.9 * bp.process (x));
        }
    }

    std::vector<float> out ((std::size_t) n);
    for (int i = 0; i < n; ++i) out[(std::size_t) i] = (float) m[(std::size_t) i];
    return out;
}

// Gentle saturation above 'knee' towards 'ceiling' (keeps spiky hand-drum transients from limiting loudness).
void softLimit (std::vector<float>& v, double knee, double ceiling)
{
    for (auto& s : v)
    {
        const double a = std::fabs ((double) s);
        if (a > knee) s = (float) (std::copysign (knee + (ceiling - knee) * std::tanh ((a - knee) / (ceiling - knee)), (double) s));
    }
}

void finalise (std::vector<float>& v, double sr, double targetDb)
{
    const double ceiling = dbToGain (-2.9), knee = dbToGain (-9.0);
    for (int pass = 0; pass < 4; ++pass)
    {
        normaliseLoudness (v, sr, targetDb, 40.0);     // effectively no ceiling here
        softLimit (v, knee, ceiling);
    }
    // the asymmetric soft limiter can leave a little DC: remove it and re-fade the ends
    DcBlocker dc (18.0, sr);
    for (auto& s : v) s = (float) dc.process ((double) s);
    sanitize (v, true);
    fadeIn (v, std::max (8, secondsToFrames (0.0004, sr)));
    fadeOut (v, std::max (16, secondsToFrames (0.012, sr)));
}

Library buildLanes (const BuildContext& ctx, const std::vector<Lane>& lanes)
{
    Library lib = startLibrary (ctx, -1, -1, -1);          // percussion has no keyswitches
    int total = 0;
    for (const auto& l : lanes) total += (l.twoLayers ? 2 : 1) * l.roundRobins;
    ProgressTracker pt (ctx, total);
    const double sr = ctx.sampleRate;
    int zi = 0;

    for (const auto& lane : lanes)
        for (int li = 0; li < (lane.twoLayers ? 2 : 1); ++li)
        {
            const VelLayer& vl = lane.twoLayers ? (li == 0 ? kSoft : kHard) : kAllVel;
            const double hardness = lane.twoLayers ? (li == 0 ? 0.38 : 1.0) : 0.4;
            for (int rr = 1; rr <= lane.roundRobins; ++rr)
            {
                if (! pt.next()) { lib.cancelled = true; return lib; }
                Rng rng = Rng::forZone (ctx.instrument, zi++);
                Zone z;
                z.articulation = lane.name;
                z.name = zoneName (lane.name, lane.lokey, lane.twoLayers ? vl.name : nullptr, lane.roundRobins > 1 ? rr : 0);
                z.sampleRate = sr;
                z.lokey = lane.lokey; z.hikey = lane.hikey; z.keycenter = lane.lokey;
                z.lovel = vl.lo; z.hivel = vl.hi;
                z.seqLength = lane.roundRobins; z.seqPosition = rr;
                z.loop = Loop::oneShot; z.pitchKeytrack = 0.f;
                z.ampRelease = 0.08f; z.ampVeltrack = 55.f;
                z.group = lane.group; z.offBy = lane.offBy;
                z.data = renderHit (lane.p, sr, hardness * rng.range (0.97, 1.03), rng);
                finishOneShot (z.data, sr);
                finalise (z.data, sr, lane.twoLayers ? (li == 0 ? kSoftLoudnessDb : kHardLoudnessDb) : lane.singleLoudnessDb);
                lib.zones.push_back (std::move (z));
            }
        }
    pt.finished();
    return lib;
}

Params P (double length, double f0, double t60, double radius)
{
    Params p; p.length = length; p.f0 = f0; p.t60 = t60; p.radius = radius; return p;
}
} // namespace

// ---------------------------------------------------------------------------------------------
Library buildDerbouka (const BuildContext& ctx)
{
    // DUM: centre stroke, open head; clear pitch (membrane blended towards harmonic) + cavity boom.
    Params dum = P (0.75, 142.0, 0.50, 0.30);
    dum.harm = 0.55; dum.nModes = 6; dum.dampSlope = 0.35; dum.glide = 0.09; dum.glideTau = 0.05;
    dum.cavHz = 92.0; dum.cavT60 = 0.38; dum.cavGain = 0.45;
    dum.clickHz = 800.0; dum.clickQ = 0.8; dum.clickTau = 0.007; dum.clickGain = 0.22;
    // TEK: rim stroke, thin and bright, metallic ring of the rim/shell.
    Params tek = P (0.28, 520.0, 0.13, 0.86);
    tek.nModes = 9; tek.dampSlope = 0.2; tek.harm = 0.0; tek.tilt = 0.8;
    tek.clickHz = 3200.0; tek.clickQ = 1.2; tek.clickTau = 0.006; tek.clickGain = 0.7;
    tek.ringHz[0] = 1900.0; tek.ringHz[1] = 2750.0; tek.ringHz[2] = 4100.0; tek.ringT60 = 0.11; tek.ringGain = 0.22;
    // KA: left-hand edge stroke, dry and short.
    Params ka = P (0.18, 640.0, 0.07, 0.91);
    ka.nModes = 8; ka.dampSlope = 0.3; ka.tilt = 0.8;
    ka.clickHz = 4300.0; ka.clickQ = 1.4; ka.clickTau = 0.005; ka.clickGain = 0.7;
    ka.ringHz[0] = 2400.0; ka.ringHz[1] = 3500.0; ka.ringT60 = 0.06; ka.ringGain = 0.18;
    // SLAP: fingers splayed, cracking broadband stroke with a short head ring.
    Params slap = P (0.42, 330.0, 0.22, 0.74);
    slap.nModes = 9; slap.dampSlope = 0.4; slap.tilt = 0.7; slap.harm = 0.2;
    slap.clickHz = 1900.0; slap.clickQ = 0.55; slap.clickTau = 0.02; slap.clickGain = 0.9;
    slap.ringHz[0] = 1250.0; slap.ringHz[1] = 2050.0; slap.ringHz[2] = 3400.0; slap.ringT60 = 0.16; slap.ringGain = 0.2;
    slap.cavHz = 105.0; slap.cavT60 = 0.12; slap.cavGain = 0.15;
    // GHOST: finger tap, heavily damped.
    Params ghost = P (0.14, 410.0, 0.045, 0.62);
    ghost.nModes = 7; ghost.dampSlope = 0.5; ghost.tilt = 0.9;
    ghost.clickHz = 2300.0; ghost.clickQ = 0.9; ghost.clickTau = 0.009; ghost.clickGain = 0.5;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 2, 2, 0.0,   tek },
        { "ka",    40, 40, true,  3, 2, 2, 0.0,   ka },
        { "slap",  41, 42, true,  2, 2, 2, 0.0,   slap },
        { "ghost", 43, 44, false, 3, 2, 2, -20.0, ghost },
    });
    if (lib.cancelled) return lib;
    addKeyAliases (lib, 38, 38, 45);      // 45 plays TEK
    addKeyAliases (lib, 40, 40, 46);      // 46 plays KA
    addKeyAliases (lib, 41, 41, 47);      // 47 plays SLAP
    addKeyAliases (lib, 36, 47, 48);      // mirror 48..59
    return lib;
}

Library buildTar (const BuildContext& ctx)
{
    // Frame drum: thumpy, inharmonic membrane modes, jingles rattle with every stroke.
    Params dum = P (0.9, 112.0, 0.55, 0.28);
    dum.nModes = 8; dum.dampSlope = 0.6; dum.glide = 0.08; dum.glideTau = 0.05;
    dum.clickHz = 900.0; dum.clickQ = 0.8; dum.clickTau = 0.009; dum.clickGain = 0.2;
    dum.jingleDrive = 9.0; dum.jingleT60 = 0.55;
    Params tek = P (0.36, 290.0, 0.17, 0.84);
    tek.nModes = 9; tek.dampSlope = 0.35; tek.tilt = 0.8;
    tek.clickHz = 2800.0; tek.clickQ = 1.1; tek.clickTau = 0.008; tek.clickGain = 0.6;
    tek.ringHz[0] = 1250.0; tek.ringT60 = 0.09; tek.ringGain = 0.15;
    tek.jingleDrive = 7.0; tek.jingleT60 = 0.45;
    Params ka = P (0.28, 370.0, 0.10, 0.9);
    ka.nModes = 8; ka.dampSlope = 0.4; ka.tilt = 0.8;
    ka.clickHz = 3500.0; ka.clickQ = 1.3; ka.clickTau = 0.006; ka.clickGain = 0.6;
    ka.jingleDrive = 5.0; ka.jingleT60 = 0.35;
    Params zil = P (1.0, 150.0, 0.12, 0.5);        // jingle shake: a little frame thump plus the shaken jingles
    zil.nModes = 4; zil.clickHz = 6000.0; zil.clickQ = 0.7; zil.clickTau = 0.02; zil.clickGain = 0.12;
    zil.shake = 0.35; zil.shakeTau = 0.42; zil.jingleT60 = 0.9; zil.jingleDrive = 3.0; zil.gain = 0.5;
    Params ghost = P (0.2, 320.0, 0.07, 0.6);
    ghost.nModes = 7; ghost.dampSlope = 0.5; ghost.tilt = 0.9;
    ghost.clickHz = 2300.0; ghost.clickQ = 0.9; ghost.clickTau = 0.01; ghost.clickGain = 0.4;
    ghost.jingleDrive = 4.0; ghost.jingleT60 = 0.28;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 1, 1, 0.0,   tek },
        { "ka",    40, 41, true,  3, 1, 1, 0.0,   ka },
        { "zil",   42, 44, true,  3, 3, 3, 0.0,   zil },
        { "ghost", 45, 47, false, 3, 1, 1, -20.0, ghost },
    });
    if (lib.cancelled) return lib;
    addKeyAliases (lib, 36, 47, 48);      // mirror 48..59
    return lib;
}

Library buildBendir (const BuildContext& ctx)
{
    // Large frame drum with a gut snare under the skin: deep thump plus a buzz that follows the head.
    Params dum = P (0.85, 82.0, 0.50, 0.28);
    dum.nModes = 8; dum.dampSlope = 0.6; dum.glide = 0.07; dum.glideTau = 0.05;
    dum.clickHz = 700.0; dum.clickQ = 0.8; dum.clickTau = 0.01; dum.clickGain = 0.18;
    dum.buzz = 0.12; dum.buzzSens = 1.5; dum.buzzRelease = 0.11;
    Params tek = P (0.36, 230.0, 0.15, 0.84);
    tek.nModes = 9; tek.dampSlope = 0.4; tek.tilt = 0.8;
    tek.clickHz = 2500.0; tek.clickQ = 1.1; tek.clickTau = 0.009; tek.clickGain = 0.5;
    tek.buzz = 0.2; tek.buzzSens = 1.2; tek.buzzRelease = 0.09;
    Params ka = P (0.26, 300.0, 0.09, 0.9);
    ka.nModes = 8; ka.dampSlope = 0.4; ka.tilt = 0.8;
    ka.clickHz = 3200.0; ka.clickQ = 1.2; ka.clickTau = 0.007; ka.clickGain = 0.5;
    ka.buzz = 0.15; ka.buzzSens = 1.0; ka.buzzRelease = 0.07;
    Params slap = P (0.55, 200.0, 0.2, 0.72);
    slap.nModes = 9; slap.dampSlope = 0.5; slap.tilt = 0.7;
    slap.clickHz = 1700.0; slap.clickQ = 0.55; slap.clickTau = 0.03; slap.clickGain = 0.7;
    slap.buzz = 0.4; slap.buzzSens = 1.5; slap.buzzRelease = 0.17;
    Params ghost = P (0.2, 250.0, 0.06, 0.6);
    ghost.nModes = 7; ghost.dampSlope = 0.5; ghost.tilt = 0.9;
    ghost.clickHz = 2200.0; ghost.clickQ = 0.9; ghost.clickTau = 0.01; ghost.clickGain = 0.35;
    ghost.buzz = 0.15; ghost.buzzSens = 1.0; ghost.buzzRelease = 0.06;
    Params roll = P (0.9, 240.0, 0.06, 0.8);
    roll.nModes = 8; roll.dampSlope = 0.5; roll.tilt = 0.8;
    roll.clickHz = 2400.0; roll.clickQ = 0.8; roll.clickTau = 0.012; roll.clickGain = 0.6;
    roll.buzz = 0.5; roll.buzzSens = 2.5; roll.buzzRelease = 0.05;
    roll.rollRate = 23.0; roll.rollDur = 0.72;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 1, 1, 0.0,   tek },
        { "ka",    40, 41, true,  3, 1, 1, 0.0,   ka },
        { "slap",  42, 43, true,  2, 1, 1, 0.0,   slap },
        { "ghost", 44, 45, false, 3, 1, 1, -20.0, ghost },
        { "roll",  46, 47, false, 2, 2, 1, -19.0, roll },
    });
    if (lib.cancelled) return lib;
    addKeyAliases (lib, 36, 47, 48);      // mirror 48..59
    return lib;
}
} // namespace humpire::model
