// ============================================================================
//  BOWED + WIND FAMILY: Violon (violin), Ney (end-blown flute).
//
//  *** STUB IMPLEMENTATION ***  (architect's placeholder: periodic additive tone + formants + filtered
//  noise, exact loop points, correct key map / layers / keyswitches so that the whole chain loads,
//  plays and passes tests.)  The instrument authors replace buildViolon / buildNey with real models
//  (bowed string: Helmholtz motion / friction waveguide + body modes; ney: jet-driven air column with
//  breath noise, chiff and overblowing).  Keep the key map described in ModelLibrary.h
//  ("INSTRUMENT SPECIFICATIONS").  Only edit this file (+ your own helper header).
// ============================================================================
#include "ModelCommon.h"

namespace humpire::model
{
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

std::vector<float> renderPizz (double f0, double sr, Rng& rng)
{
    KarplusString s;
    s.init (f0, sr, clampd (1.2 * std::pow (220.0 / f0, 0.4), 0.25, 2.5), 0.7);
    const auto e = KarplusString::excitation (rng, s.loopDelay(), 0.7, 0.14, sr);
    const int total = secondsToFrames (1.1, sr);
    std::vector<float> out ((std::size_t) total);
    for (int n = 0; n < total; ++n) out[(std::size_t) n] = s.tick ((std::size_t) n < e.size() ? e[(std::size_t) n] : 0.f);
    return out;
}
} // namespace

// ---------------------------------------------------------------------------------------------
Library buildViolon (const BuildContext& ctx)
{
    const auto spans = makeKeySpans (ctx.spec().lo, ctx.spec().hi, 3);
    Library lib = startLibrary (ctx);
    ProgressTracker pt (ctx, (int) spans.size() * 4 + 1);
    int zi = 0;

    SustainStyle plain;                       // bowed: sawtooth-like spectrum, bow noise
    plain.rolloff = 1.05; plain.noiseAmount = 0.035; plain.noiseCentreHz = 3500.0; plain.attackSec = 0.07; plain.introSec = 0.22; plain.bodySec = 1.0;
    SustainStyle vibrato = plain;
    vibrato.vibratoHz = 5.4; vibrato.vibratoDepth = 0.011; vibrato.bodySec = 1.1;

    for (const auto& sp : spans)
    {
        for (const VelLayer* vl : { &kSoft, &kHard })
        {
            if (! pt.next()) { lib.cancelled = true; return lib; }
            lib.zones.push_back (sustainZone (ctx, "sustain", sp, *vl, 12, plain, vl == &kSoft ? 0.9 : 0.0, vl == &kSoft ? kSoftLoudnessDb : kHardLoudnessDb, zi++, 0.05f, 0.25f));
        }
        if (! pt.next()) { lib.cancelled = true; return lib; }
        lib.zones.push_back (sustainZone (ctx, "vibrato", sp, kAllVel, 13, vibrato, 0.3, -17.0, zi++, 0.06f, 0.25f));

        if (! pt.next()) { lib.cancelled = true; return lib; }
        Rng rng = Rng::forZone (ctx.instrument, zi++);
        Zone z;
        z.articulation = "pizz"; z.name = zoneName ("pizz", sp.center);
        z.sampleRate = ctx.sampleRate;
        z.lokey = sp.lokey; z.hikey = sp.hikey; z.keycenter = sp.center;
        z.lovel = kAllVel.lo; z.hivel = kAllVel.hi; z.swLast = 14;
        z.ampRelease = 0.2f; z.ampVeltrack = 70.f;
        z.data = renderPizz (midiToHz (sp.center), ctx.sampleRate, rng);
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
        Biquad bp = Biquad::bandpass (2800.0, 0.9, ctx.sampleRate);
        const int n = secondsToFrames (0.09, ctx.sampleRate);
        z.data.resize ((std::size_t) n);
        for (int i = 0; i < n; ++i)
            z.data[(std::size_t) i] = (float) (bp.process (rng.bipolar()) * std::exp (-(double) i / (0.025 * ctx.sampleRate)));
        finishOneShot (z.data, ctx.sampleRate, false, 0.02);
        normalisePeak (z.data, -10.0);
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
