// ============================================================================
//  PERCUSSION FAMILY: Derbouka, Tar, Bendir.
//
//  *** STUB IMPLEMENTATION ***  (architect's placeholder: pitch-swept sine + resonators + shaped noise
//  bursts, correct lane map / layers / round robins / choke groups / mirror so that the whole chain
//  loads, plays and passes tests.)  The instrument authors replace the bodies of buildDerbouka /
//  buildTar / buildBendir with real models (modal membrane + air cavity, rim/finger excitation,
//  jingle (zil) bank, snare buzz ...).  Keep the key map, lanes, layers, groups and budgets described
//  in ModelLibrary.h ("INSTRUMENT SPECIFICATIONS").  Only edit this file (+ your own helper header).
// ============================================================================
#include "ModelCommon.h"

namespace humpire::model
{
namespace
{
struct HitRecipe
{
    double length = 0.3;                                  // seconds
    // membrane pitch drop (bass strokes)
    double sweepHz0 = 0.0, sweepHz1 = 0.0, sweepTau = 0.04, sweepT60 = 0.5, sweepGain = 0.0;
    // transient noise burst
    double noiseHz = 2500.0, noiseQ = 1.2, noiseTau = 0.012, noiseGain = 0.0;
    // ringing partials
    double modeHz[3] = { 0.0, 0.0, 0.0 };
    double modeT60 = 0.12, modeGain = 0.0;
    // snare / jingle buzz (high noise tail)
    double buzzGain = 0.0, buzzTau = 0.08;
    // jingle bank (zil)
    double zilGain = 0.0, zilT60 = 0.7;
    // finger roll amplitude modulation
    double rollRate = 0.0;
};

struct Lane
{
    const char* name;
    int lokey, hikey;
    bool twoLayers;          // soft+hard, otherwise a single 1..127 layer
    int roundRobins;
    int group, offBy;
    double singleLoudnessDb; // loudness target for single-layer lanes
    HitRecipe recipe;
};

std::vector<float> renderHit (const HitRecipe& r, double sr, double hardness, Rng& rng)
{
    const int n = secondsToFrames (r.length, sr);
    std::vector<float> out ((std::size_t) n, 0.f);
    const double tune = 1.0 + 0.012 * rng.bipolar();      // round-robin variation
    const double bright = 0.75 + 0.5 * hardness;

    // pitch-swept membrane tone
    if (r.sweepGain > 0.0)
    {
        double ph = rng.range (0.0, 0.5);
        for (int i = 0; i < n; ++i)
        {
            const double t = (double) i / sr;
            const double f = (r.sweepHz1 + (r.sweepHz0 - r.sweepHz1) * std::exp (-t / r.sweepTau)) * tune;
            ph += kTwoPi * f / sr;
            out[(std::size_t) i] += (float) (r.sweepGain * std::sin (ph) * std::exp (-6.9 * t / r.sweepT60));
        }
    }

    // ringing partials: impulse-excited two-pole resonators
    if (r.modeGain > 0.0)
        for (double hz : r.modeHz)
        {
            if (hz <= 0.0) continue;
            Resonator res;
            res.set (hz * tune * rng.range (0.99, 1.01), r.modeT60 * rng.range (0.9, 1.1), sr);
            const double g = r.modeGain * (0.5 + 0.5 * hardness);
            for (int i = 0; i < n; ++i) out[(std::size_t) i] += (float) (g * res.process (i == 0 ? 1.0 : 0.0));
        }

    // transient noise burst
    if (r.noiseGain > 0.0)
    {
        Biquad bp = Biquad::bandpass (r.noiseHz * bright, r.noiseQ, sr);
        for (int i = 0; i < n; ++i)
        {
            const double t = (double) i / sr;
            out[(std::size_t) i] += (float) (r.noiseGain * (0.4 + 0.6 * hardness) * bp.process (rng.bipolar()) * std::exp (-t / r.noiseTau));
        }
    }

    // buzz tail (snare wires / jingles rattling)
    if (r.buzzGain > 0.0)
    {
        Biquad hp = Biquad::highpass (3500.0, 0.7, sr);
        for (int i = 0; i < n; ++i)
        {
            const double t = (double) i / sr;
            double mod = 1.0;
            if (r.rollRate > 0.0) mod = 0.55 + 0.45 * std::sin (kPi * r.rollRate * t) * std::sin (kPi * r.rollRate * t);
            out[(std::size_t) i] += (float) (r.buzzGain * (0.3 + 0.7 * hardness) * hp.process (rng.bipolar()) * std::exp (-t / r.buzzTau) * mod);
        }
    }

    // jingle bank: inharmonic high partials excited by a short burst
    if (r.zilGain > 0.0)
    {
        for (int k = 0; k < 10; ++k)
        {
            Resonator res;
            res.set (rng.range (3800.0, 9500.0), r.zilT60 * rng.range (0.5, 1.2), sr);
            const double g = r.zilGain * (0.4 + 0.6 * hardness) / 10.0;
            const int burst = secondsToFrames (0.012, sr);
            for (int i = 0; i < n; ++i)
                out[(std::size_t) i] += (float) (g * res.process (i < burst ? rng.bipolar() * (1.0 - (double) i / (double) burst) : 0.0));
        }
    }
    return out;
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
            const double hardness = lane.twoLayers ? (li == 0 ? 0.35 : 1.0) : 0.3;
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
                z.data = renderHit (lane.recipe, sr, hardness, rng);
                finishOneShot (z.data, sr);
                normaliseLoudness (z.data, sr, lane.twoLayers ? (li == 0 ? kSoftLoudnessDb : kHardLoudnessDb) : lane.singleLoudnessDb, kPeakCeilingDb);
                lib.zones.push_back (std::move (z));
            }
        }
    pt.finished();
    return lib;
}
} // namespace

// ---------------------------------------------------------------------------------------------
Library buildDerbouka (const BuildContext& ctx)
{
    HitRecipe dum;  dum.length = 0.75; dum.sweepHz0 = 190; dum.sweepHz1 = 88; dum.sweepTau = 0.035; dum.sweepT60 = 0.55; dum.sweepGain = 1.0;
    dum.noiseHz = 700; dum.noiseQ = 0.8; dum.noiseTau = 0.008; dum.noiseGain = 0.25; dum.modeHz[0] = 176; dum.modeT60 = 0.25; dum.modeGain = 0.15;
    HitRecipe tek;  tek.length = 0.28; tek.noiseHz = 3000; tek.noiseQ = 1.4; tek.noiseTau = 0.014; tek.noiseGain = 1.0;
    tek.modeHz[0] = 1750; tek.modeHz[1] = 2650; tek.modeHz[2] = 3900; tek.modeT60 = 0.14; tek.modeGain = 0.6;
    HitRecipe ka;   ka.length = 0.20; ka.noiseHz = 4200; ka.noiseQ = 1.6; ka.noiseTau = 0.009; ka.noiseGain = 1.0;
    ka.modeHz[0] = 2300; ka.modeHz[1] = 3400; ka.modeT60 = 0.08; ka.modeGain = 0.5;
    HitRecipe slap; slap.length = 0.45; slap.noiseHz = 1800; slap.noiseQ = 0.7; slap.noiseTau = 0.04; slap.noiseGain = 1.0;
    slap.modeHz[0] = 1150; slap.modeHz[1] = 1900; slap.modeHz[2] = 3100; slap.modeT60 = 0.22; slap.modeGain = 0.7;
    HitRecipe ghost; ghost.length = 0.14; ghost.noiseHz = 2400; ghost.noiseQ = 1.0; ghost.noiseTau = 0.012; ghost.noiseGain = 1.0;
    ghost.modeHz[0] = 1500; ghost.modeT60 = 0.06; ghost.modeGain = 0.3;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 2, 2, 0.0,   tek },
        { "ka",    40, 40, true,  3, 2, 2, 0.0,   ka },
        { "slap",  41, 42, true,  2, 2, 2, 0.0,   slap },
        { "ghost", 43, 44, false, 3, 2, 2, -24.0, ghost },
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
    HitRecipe dum;  dum.length = 0.9; dum.sweepHz0 = 230; dum.sweepHz1 = 105; dum.sweepTau = 0.03; dum.sweepT60 = 0.6; dum.sweepGain = 1.0;
    dum.noiseHz = 900; dum.noiseQ = 0.8; dum.noiseTau = 0.01; dum.noiseGain = 0.2; dum.modeHz[0] = 210; dum.modeT60 = 0.3; dum.modeGain = 0.15;
    dum.zilGain = 0.18; dum.zilT60 = 0.45;
    HitRecipe tek;  tek.length = 0.35; tek.noiseHz = 2800; tek.noiseQ = 1.3; tek.noiseTau = 0.014; tek.noiseGain = 1.0;
    tek.modeHz[0] = 1500; tek.modeHz[1] = 2300; tek.modeT60 = 0.16; tek.modeGain = 0.5; tek.zilGain = 0.25; tek.zilT60 = 0.4;
    HitRecipe ka;   ka.length = 0.28; ka.noiseHz = 3600; ka.noiseQ = 1.5; ka.noiseTau = 0.01; ka.noiseGain = 1.0;
    ka.modeHz[0] = 1900; ka.modeT60 = 0.1; ka.modeGain = 0.4; ka.zilGain = 0.15; ka.zilT60 = 0.3;
    HitRecipe zil;  zil.length = 1.0; zil.noiseHz = 6500; zil.noiseQ = 0.8; zil.noiseTau = 0.03; zil.noiseGain = 0.5; zil.zilGain = 1.2; zil.zilT60 = 0.8;
    HitRecipe ghost; ghost.length = 0.2; ghost.noiseHz = 2200; ghost.noiseQ = 1.0; ghost.noiseTau = 0.012; ghost.noiseGain = 1.0;
    ghost.modeHz[0] = 1400; ghost.modeT60 = 0.07; ghost.modeGain = 0.3; ghost.zilGain = 0.2; ghost.zilT60 = 0.25;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 1, 1, 0.0,   tek },
        { "ka",    40, 41, true,  3, 1, 1, 0.0,   ka },
        { "zil",   42, 44, true,  3, 3, 3, 0.0,   zil },
        { "ghost", 45, 47, false, 3, 1, 1, -24.0, ghost },
    });
    if (lib.cancelled) return lib;
    addKeyAliases (lib, 36, 47, 48);      // mirror 48..59
    return lib;
}

Library buildBendir (const BuildContext& ctx)
{
    HitRecipe dum;  dum.length = 0.85; dum.sweepHz0 = 165; dum.sweepHz1 = 92; dum.sweepTau = 0.04; dum.sweepT60 = 0.5; dum.sweepGain = 1.0;
    dum.noiseHz = 800; dum.noiseQ = 0.8; dum.noiseTau = 0.01; dum.noiseGain = 0.2; dum.modeHz[0] = 185; dum.modeT60 = 0.25; dum.modeGain = 0.15;
    dum.buzzGain = 0.18; dum.buzzTau = 0.16;
    HitRecipe tek;  tek.length = 0.35; tek.noiseHz = 2600; tek.noiseQ = 1.2; tek.noiseTau = 0.016; tek.noiseGain = 1.0;
    tek.modeHz[0] = 1350; tek.modeHz[1] = 2100; tek.modeT60 = 0.15; tek.modeGain = 0.5; tek.buzzGain = 0.22; tek.buzzTau = 0.12;
    HitRecipe ka;   ka.length = 0.26; ka.noiseHz = 3300; ka.noiseQ = 1.4; ka.noiseTau = 0.01; ka.noiseGain = 1.0;
    ka.modeHz[0] = 1800; ka.modeT60 = 0.09; ka.modeGain = 0.4; ka.buzzGain = 0.15; ka.buzzTau = 0.08;
    HitRecipe slap; slap.length = 0.55; slap.noiseHz = 1700; slap.noiseQ = 0.7; slap.noiseTau = 0.045; slap.noiseGain = 1.0;
    slap.modeHz[0] = 1000; slap.modeHz[1] = 1700; slap.modeT60 = 0.2; slap.modeGain = 0.6; slap.buzzGain = 0.45; slap.buzzTau = 0.25;
    HitRecipe ghost; ghost.length = 0.2; ghost.noiseHz = 2200; ghost.noiseQ = 1.0; ghost.noiseTau = 0.012; ghost.noiseGain = 1.0;
    ghost.modeHz[0] = 1300; ghost.modeT60 = 0.06; ghost.modeGain = 0.3; ghost.buzzGain = 0.15; ghost.buzzTau = 0.07;
    HitRecipe roll; roll.length = 0.9; roll.noiseHz = 2400; roll.noiseQ = 0.8; roll.noiseTau = 0.30; roll.noiseGain = 0.5;
    roll.buzzGain = 0.6; roll.buzzTau = 0.35; roll.rollRate = 26.0; roll.modeHz[0] = 1250; roll.modeT60 = 0.3; roll.modeGain = 0.15;

    Library lib = buildLanes (ctx, {
        { "dum",   36, 37, true,  3, 1, 1, 0.0,   dum },
        { "tek",   38, 39, true,  3, 1, 1, 0.0,   tek },
        { "ka",    40, 41, true,  3, 1, 1, 0.0,   ka },
        { "slap",  42, 43, true,  2, 1, 1, 0.0,   slap },
        { "ghost", 44, 45, false, 3, 1, 1, -24.0, ghost },
        { "roll",  46, 47, false, 2, 2, 1, -18.0, roll },
    });
    if (lib.cancelled) return lib;
    addKeyAliases (lib, 36, 47, 48);      // mirror 48..59
    return lib;
}
} // namespace humpire::model
