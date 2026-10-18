// ============================================================================
//  PLUCKED FAMILY: Mandole, Banjo, Qanun.
//
//  *** STUB IMPLEMENTATION ***  (architect's placeholder: a tuned Karplus-Strong string with a few
//  tweaks, correct key map / layers / loops so that the whole chain loads, plays and passes tests.)
//  The instrument authors replace the bodies of buildMandole / buildBanjo / buildQanun with real
//  physical models (stiff string with dispersion, plectrum / finger excitation, body modes,
//  sympathetic strings, ...).  Keep the key map, articulations, layers and budgets described in
//  ModelLibrary.h ("INSTRUMENT SPECIFICATIONS").  Only edit this file (+ your own helper header).
// ============================================================================
#include "ModelCommon.h"

namespace humpire::model
{
namespace
{
struct PluckStyle
{
    double seconds = 1.5;     // rendered length
    double t60Ref = 2.0;      // string T60 at 220 Hz
    double t60Exp = 0.4;      // T60 ~ (220/f0)^exp
    double bright = 0.7;      // loop filter brightness
    double hardSoft = 0.45;   // excitation hardness of the soft layer
    double hardHard = 0.85;   // excitation hardness of the hard layer
    double pickPos = 0.16;
    double thumpHz = 0.0;     // optional body / head resonance (0 = none)
    double thumpT60 = 0.1;
    double thumpMix = 0.0;
    double tremoloRate = 11.0;
};

double t60For (double f0, const PluckStyle& st) { return clampd (st.t60Ref * std::pow (220.0 / f0, st.t60Exp), 0.3, 6.0); }

std::vector<float> renderPluck (double f0, double sr, const PluckStyle& st, double hardness, double seconds, Rng& rng)
{
    KarplusString s;
    s.init (f0, sr, t60For (f0, st), st.bright);
    const auto e = KarplusString::excitation (rng, s.loopDelay(), hardness, st.pickPos * rng.range (0.85, 1.15), sr);
    Resonator body;
    if (st.thumpHz > 0.0) body.set (st.thumpHz, st.thumpT60, sr);
    const int total = secondsToFrames (seconds, sr);
    std::vector<float> out ((std::size_t) total);
    for (int n = 0; n < total; ++n)
    {
        const float x = (std::size_t) n < e.size() ? e[(std::size_t) n] : 0.f;
        double y = s.tick (x);
        if (st.thumpHz > 0.0) y += st.thumpMix * body.process ((double) x);
        out[(std::size_t) n] = (float) y;
    }
    return out;
}

std::vector<float> renderTremolo (double f0, double sr, const PluckStyle& st, double hardness, double seconds, Rng& rng)
{
    KarplusString s;
    s.init (f0, sr, t60For (f0, st) * 0.6, st.bright);
    const int total = secondsToFrames (seconds, sr);
    std::vector<float> out ((std::size_t) total);
    std::vector<float> e;
    std::size_t ei = 0;
    double nextPluck = 0.0;
    for (int n = 0; n < total; ++n)
    {
        if ((double) n >= nextPluck)
        {
            e = KarplusString::excitation (rng, s.loopDelay(), hardness, st.pickPos * rng.range (0.85, 1.15), sr);
            for (auto& v : e) v *= (float) rng.range (0.7, 1.0);
            ei = 0;
            nextPluck += sr / st.tremoloRate * rng.range (0.96, 1.04);
        }
        const float x = ei < e.size() ? e[ei++] : 0.f;
        out[(std::size_t) n] = s.tick (x);
    }
    return out;
}

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

// Generic stub: articulation 12 "pick"-like plucks (2 velocity layers x `rr` round robins), articulation 13 tremolo/roll loop.
Library buildPlucked (const BuildContext& ctx, int step, int rr, const char* pluckName, const char* loopName, const PluckStyle& st,
                      double ampRelease, double ampVeltrack, double tremoloBodySec)
{
    const auto spans = makeKeySpans (ctx.spec().lo, ctx.spec().hi, step);
    Library lib = startLibrary (ctx);
    const int total = (int) spans.size() * (2 * rr + 1);
    ProgressTracker pt (ctx, total);
    const double sr = ctx.sampleRate;
    int zoneIndex = 0;

    for (const auto& sp : spans)
    {
        const double f0 = midiToHz (sp.center);
        for (const VelLayer* vl : { &kSoft, &kHard })
            for (int r = 1; r <= rr; ++r)
            {
                if (! pt.next()) { lib.cancelled = true; return lib; }
                Rng rng = Rng::forZone (ctx.instrument, zoneIndex++);
                Zone z = baseZone (ctx, pluckName, zoneName (pluckName, sp.center, vl->name, rr > 1 ? r : 0), sp, *vl, 12);
                z.seqLength = rr; z.seqPosition = r;
                z.ampRelease = (float) ampRelease; z.ampVeltrack = (float) ampVeltrack;
                z.data = renderPluck (f0, sr, st, vl == &kSoft ? st.hardSoft : st.hardHard, st.seconds, rng);
                finishOneShot (z.data, sr);
                normaliseLoudness (z.data, sr, vl == &kSoft ? kSoftLoudnessDb : kHardLoudnessDb, kPeakCeilingDb);
                lib.zones.push_back (std::move (z));
            }

        if (! pt.next()) { lib.cancelled = true; return lib; }
        Rng rng = Rng::forZone (ctx.instrument, zoneIndex++);
        Zone z = baseZone (ctx, loopName, zoneName (loopName, sp.center), sp, kAllVel, 13);
        z.ampAttack = 0.02f; z.ampRelease = 0.35f; z.ampVeltrack = (float) ampVeltrack;
        z.data = renderTremolo (f0, sr, st, 0.7, tremoloBodySec + 0.5, rng);
        sanitize (z.data, true);
        fadeIn (z.data, std::max (8, secondsToFrames (0.0004, sr)));
        if (! makeLoopFromBody (z, sr / f0, 0.35, 0.5, 0.012, false, Loop::continuous))
        {
            lib.zones.clear();                                   // cannot happen with these lengths; keep the contract anyway
            return lib;
        }
        normaliseLoudness (z.data, sr, -17.0, kPeakCeilingDb);
        lib.zones.push_back (std::move (z));
    }
    pt.finished();
    return lib;
}
} // namespace

// ---------------------------------------------------------------------------------------------
Library buildMandole (const BuildContext& ctx)
{
    PluckStyle st;
    st.seconds = 1.5; st.t60Ref = 1.6; st.bright = 0.75; st.pickPos = 0.18;
    st.thumpHz = 190.0; st.thumpT60 = 0.12; st.thumpMix = 0.15;
    st.tremoloRate = 12.0;
    return buildPlucked (ctx, 3, 3, "pick", "tremolo", st, 0.30, 70.0, 1.0);
}

Library buildBanjo (const BuildContext& ctx)
{
    PluckStyle st;
    st.seconds = 1.0; st.t60Ref = 0.9; st.t60Exp = 0.3; st.bright = 0.95; st.hardSoft = 0.6; st.hardHard = 1.0; st.pickPos = 0.12;
    st.thumpHz = 320.0; st.thumpT60 = 0.07; st.thumpMix = 0.25;
    st.tremoloRate = 14.0;
    return buildPlucked (ctx, 3, 2, "pick", "roll", st, 0.12, 70.0, 0.8);
}

Library buildQanun (const BuildContext& ctx)
{
    PluckStyle st;
    st.seconds = 2.2; st.t60Ref = 3.2; st.t60Exp = 0.5; st.bright = 0.6; st.hardSoft = 0.4; st.hardHard = 0.75; st.pickPos = 0.2;
    st.thumpHz = 150.0; st.thumpT60 = 0.2; st.thumpMix = 0.10;
    st.tremoloRate = 9.0;
    return buildPlucked (ctx, 4, 2, "pluck", "tremolo", st, 0.25, 65.0, 1.0);
}
} // namespace humpire::model
