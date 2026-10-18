// HUMPIRE CHAABI ORCHESTRA - headless smoke test.
//
//   HumpireSmokeTest                         no sample folder: exercises the BUILT-IN MODELLED LIBRARY end to end
//   HumpireSmokeTest --samples <dir>         additionally loads a real SFZ library from <dir>
//   HumpireSmokeTest --ui <out.png> [TAB]    additionally captures the editor (TAB = PLAY | PHRASES | PATTERNS)
//   HumpireSmokeTest --ui-only <out.png> [TAB]   captures the editor only (skips the tests)
//
// Covered: the model renderers themselves (validation, budgets, determinism, thread safety, cancellation), the
// fallback chain (processor -> background render -> bank -> engine), every instrument across its range, keyswitch
// articulations, velocity layers, round robins, sustained loops, percussion lanes, phrases, sequencer, factory
// presets, bypass, host edge cases (no prepareToPlay, oversized block, 0 samples, mono bus, flood of MIDI, all sample
// rates), state save/restore, SFZ precedence + SFZ failure fallback, SFZ round trip of the model library, English-only
// user-visible text, destruction while rendering.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Model/ModelLibrary.h"
#include "LibraryWriter.h"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <set>
#include <thread>

using namespace humpire;
namespace mdl = humpire::model;

static int failures = 0, checks = 0;
#define CHECK(cond, msg) do { ++checks; if (! (cond)) { std::printf ("  FAIL: %s\n", juce::String (msg).toRawUTF8()); ++failures; } else std::printf ("  ok  : %s\n", juce::String (msg).toRawUTF8()); } while (0)
#define CHECKQ(cond, msg) do { ++checks; if (! (cond)) { std::printf ("  FAIL: %s\n", juce::String (msg).toRawUTF8()); ++failures; } } while (0)   // quiet when ok
#define NOTE(...) std::printf ("  " __VA_ARGS__)

static void pump (int ms) { juce::MessageManager::getInstance()->runDispatchLoopUntil (ms); }

// ------------------------------------------------------------------ helpers
static void setP (ChaabiProcessor& p, const juce::String& id, float v)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (v));
}

static void silentBlock (ChaabiProcessor& p, int bs = 512)
{
    juce::AudioBuffer<float> tmp (2, bs);
    juce::MidiBuffer m;
    p.processBlock (tmp, m);
}

// Waits until the slot is ready (state 2) or failed (3).  Keeps feeding the processor (a bank is installed by processBlock).
static bool waitReady (ChaabiProcessor& p, int slot, bool* sawRendering = nullptr, int timeoutMs = 20000)
{
    for (int t = 0; t < timeoutMs; t += 25)
    {
        pump (25);
        silentBlock (p);
        const auto st = p.getSlotStatus (slot);
        if (st.state == 1 && sawRendering != nullptr && st.builtIn && st.progress >= 0.f && st.progress <= 1.f) *sawRendering = true;
        if (st.state == 2) { pump (10); silentBlock (p); silentBlock (p); return true; }
        if (st.state == 3) { NOTE ("status: %s\n", st.message.toRawUTF8()); return false; }
    }
    return false;
}

struct Run { float peak = 0.f; double rms = 0.0; bool bad = false; std::vector<float> left; };

static Run render (ChaabiProcessor& p, const juce::MidiBuffer& first, int blocks, int bs = 512, bool keepLeft = false)
{
    Run r;
    juce::AudioBuffer<float> buf (2, bs);
    double e = 0.0;
    for (int b = 0; b < blocks; ++b)
    {
        juce::MidiBuffer m;
        if (b == 0) m = first;
        buf.clear();
        p.processBlock (buf, m);
        for (int c = 0; c < 2; ++c)
            for (int i = 0; i < bs; ++i)
            {
                const float v = buf.getSample (c, i);
                if (! std::isfinite (v)) r.bad = true;
                r.peak = juce::jmax (r.peak, std::abs (v));
                e += (double) v * v;
            }
        if (keepLeft) r.left.insert (r.left.end(), buf.getReadPointer (0), buf.getReadPointer (0) + bs);
    }
    r.rms = std::sqrt (e / (double) (2 * bs * juce::jmax (1, blocks)));
    return r;
}

static juce::MidiBuffer noteOn (int note, float vel = 0.9f, int pos = 0)
{
    juce::MidiBuffer m;
    m.addEvent (juce::MidiMessage::noteOn (1, note, vel), pos);
    return m;
}
static juce::MidiBuffer noteOff (int note)
{
    juce::MidiBuffer m;
    m.addEvent (juce::MidiMessage::noteOff (1, note), 0);
    return m;
}
static void killVoices (ChaabiProcessor& p)
{
    juce::MidiBuffer m;
    m.addEvent (juce::MidiMessage::allSoundOff (1), 0);
    render (p, m, 2);
}

static juce::File uniqueTempDir (const char* tag)
{
    return juce::File::getSpecialLocation (juce::File::tempDirectory)
        .getChildFile (juce::String ("HumpireSmoke_") + tag + "_" + juce::String::toHexString (juce::Random::getSystemRandom().nextInt64()));
}

// A processor that never sees a sample folder (the library root points to a folder that does not exist).
struct Fixture
{
    explicit Fixture (double sr = 48000.0, int bs = 512)
    {
        proc.setPlayConfigDetails (0, 2, sr, bs);
        proc.setLibraryRoot (uniqueTempDir ("nolib"));
        proc.prepareToPlay (sr, bs);
        setP (proc, pid::authent, 0.f);                   // no humanisation: deterministic comparisons
        for (int i = 0; i < kNumInstruments; ++i) { setP (proc, pid::inst (i, "rev"), 0.f); setP (proc, pid::inst (i, "dly"), 0.f); }
    }
    ChaabiProcessor proc;
};

static std::uint64_t hashLibrary (const mdl::Library& lib)
{
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&] (const void* d, size_t n) { auto* b = (const unsigned char*) d; for (size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; } };
    for (auto& z : lib.zones)
    {
        mix (z.name.data(), z.name.size());
        const int meta[] = { z.lokey, z.hikey, z.keycenter, z.lovel, z.hivel, z.seqLength, z.seqPosition, z.swLast, z.loopStart, z.loopEnd, z.group, z.offBy, (int) z.loop, z.sharedWith, z.triggerRelease ? 1 : 0 };
        mix (meta, sizeof (meta));
        if (! z.data.empty()) mix (z.data.data(), z.data.size() * sizeof (float));
    }
    return h;
}

static double brightness (const std::vector<float>& x)      // energy of the first difference / energy: grows with the spectral centroid
{
    double e = 0.0, d = 0.0;
    for (size_t i = 1; i < x.size(); ++i) { e += (double) x[i] * x[i]; const double q = (double) x[i] - x[i - 1]; d += q * q; }
    return d / juce::jmax (e, 1.0e-20);
}

// ====================================================================================================
static void testModel()
{
    std::printf ("[Model] built-in renderers (no JUCE): validation, budgets, determinism, threads, cancellation\n");
    std::vector<std::uint64_t> hashes (mdl::kNumModelInstruments);
    for (int i = 0; i < mdl::kNumModelInstruments; ++i)
    {
        const auto t0 = std::chrono::steady_clock::now();
        auto lib = mdl::build (i, 44100.0);
        const double ms = std::chrono::duration<double, std::milli> (std::chrono::steady_clock::now() - t0).count();
        const auto rep = mdl::validate (lib, i);
        for (auto& e : rep.errors) NOTE ("%s: %s\n", mdl::kSpecs[i].id, e.c_str());
        for (size_t k = 0; k < rep.warnings.size() && k < 4; ++k) NOTE ("%s (warning): %s\n", mdl::kSpecs[i].id, rep.warnings[k].c_str());
        CHECK (rep.ok(), juce::String (mdl::kSpecs[i].id) + ": validation (" + juce::String ((int) lib.zones.size()) + " zones, " + juce::String (lib.megabytes(), 1) + " MB, " + juce::String (ms, 0) + " ms)");
        CHECK (lib.megabytes() <= 25.0, juce::String (mdl::kSpecs[i].id) + ": within the 25 MB budget");
        CHECK (ms <= 8000.0, juce::String (mdl::kSpecs[i].id) + ": render time far below the 8 s alarm limit (budget is 3 s)");
        if (ms > 3000.0) NOTE ("%s: warning, render time %.0f ms exceeds the 3 s budget\n", mdl::kSpecs[i].id, ms);
        hashes[(size_t) i] = hashLibrary (lib);

        // determinism
        CHECK (hashLibrary (mdl::build (i, 44100.0)) == hashes[(size_t) i], juce::String (mdl::kSpecs[i].id) + ": deterministic (two renders identical)");

        // cancellation: stops early and flags the library
        int calls = 0;
        auto cancelled = mdl::build (i, 44100.0, [&] (float) { return ++calls < 3; });
        CHECK (cancelled.cancelled && calls <= 4, juce::String (mdl::kSpecs[i].id) + ": cancellation honoured (" + juce::String (calls) + " progress calls)");
    }

    // thread safety: two renders in parallel give the same bytes as sequential ones
    {
        std::vector<std::uint64_t> par (mdl::kNumModelInstruments);
        auto work = [&] (int begin) { for (int i = begin; i < mdl::kNumModelInstruments; i += 2) par[(size_t) i] = hashLibrary (mdl::build (i, 44100.0)); };
        std::thread a (work, 0), b (work, 1);
        a.join(); b.join();
        CHECK (par == hashes, "parallel renders (2 threads) are identical to sequential renders");
    }

    // percussion: DUM must be darker than TEK
    for (int i = 5; i < 8; ++i)
    {
        auto lib = mdl::build (i, 44100.0);
        const mdl::Zone *dum = nullptr, *tek = nullptr;
        for (auto& z : lib.zones)
        {
            if (z.sharedWith >= 0 || z.lovel < 81) continue;
            if (z.articulation == "dum" && dum == nullptr) dum = &z;
            if (z.articulation == "tek" && tek == nullptr) tek = &z;
        }
        CHECK (dum != nullptr && tek != nullptr && brightness (dum->data) < brightness (tek->data), juce::String (mdl::kSpecs[i].id) + ": DUM is darker than TEK");
    }
}

// ====================================================================================================
static void playRange (ChaabiProcessor& p, int idx)
{
    const auto& spec = mdl::kSpecs[idx];
    int silentKeys = 0, badKeys = 0, tested = 0;
    for (int key = spec.lo; key <= spec.hi; ++key)
        for (float vel : { 0.9f, 0.25f })
        {
            if (vel < 0.5f && (key % 3) != 0) continue;
            killVoices (p);
            auto r = render (p, noteOn (key, vel), 14);
            ++tested;
            if (r.bad) ++badKeys;
            if (r.peak < 0.003f) { ++silentKeys; NOTE ("silent key %d vel %.2f\n", key, vel); }
        }
    CHECK (silentKeys == 0 && badKeys == 0, juce::String (spec.id) + ": all " + juce::String (tested) + " key/velocity probes across " + juce::String (spec.lo) + ".." + juce::String (spec.hi) + " are audible and finite");
    killVoices (p);
}

static void testInstrument (int idx)
{
    const auto& spec = mdl::kSpecs[idx];
    std::printf ("[Instrument] %s (fallback, no sample folder)\n", spec.id);
    Fixture fx;
    auto& p = fx.proc;
    setP (p, pid::selected, (float) idx);
    bool sawRendering = false;
    CHECK (waitReady (p, idx, &sawRendering), juce::String (spec.id) + ": built-in instrument ready");
    const auto st = p.getSlotStatus (idx);
    CHECK (st.state == 2 && st.builtIn && ! st.warning, juce::String (spec.id) + ": status = built-in, no warning");
    CHECK (st.message.startsWith ("Built-in modelled instrument"), "status text: \"" + st.message + "\"");
    NOTE ("rendering state observed on the way: %s\n", sawRendering ? "yes" : "no (render finished before the first poll)");

    const auto lib = mdl::build (idx, 44100.0);

    playRange (p, idx);

    // ---- keyswitch articulations
    std::set<int> sws;
    for (auto& z : lib.zones) if (z.swLast >= 0 && ! z.triggerRelease) sws.insert (z.swLast);
    if (! spec.percussive)
    {
        const int probe = (spec.lo + spec.hi) / 2;
        std::vector<double> rmsPerSw;
        for (int sw : sws)
        {
            killVoices (p);
            render (p, noteOn (sw, 1.0f), 1);                          // keyswitch: silent
            auto r = render (p, noteOn (probe, 0.9f), 30);
            CHECKQ (r.peak > 0.003f && ! r.bad, "articulation " + juce::String (sw) + " sounds");
            rmsPerSw.push_back (r.rms);
        }
        CHECK (sws.size() >= 2, juce::String (spec.id) + ": " + juce::String ((int) sws.size()) + " articulations reachable by keyswitch");
        killVoices (p);
        render (p, noteOn (mdl::kKeyswitchLo, 1.0f), 1);
        auto ks = render (p, noteOn (mdl::kKeyswitchLo, 1.0f), 4);
        CHECK (ks.peak < 1.0e-6f, "keyswitch notes are silent");
    }

    // ---- velocity: the hard layer is louder than the soft layer
    {
        const int probe = spec.percussive ? 38 : (spec.lo + spec.hi) / 2;
        killVoices (p);
        auto soft = render (p, noteOn (probe, 0.2f), 20);
        killVoices (p);
        auto hard = render (p, noteOn (probe, 1.0f), 20);
        CHECK (hard.rms > soft.rms * 1.3 && soft.rms > 0.0, juce::String (spec.id) + ": hard velocity louder than soft");
    }

    // ---- round robins: sequence of equal triggers repeats with the zone period and differs between neighbours
    {
        const int probe = spec.percussive ? 38 : (spec.lo + spec.hi) / 2;
        const float vel = 0.95f;
        const int vel127 = juce::jlimit (1, 127, juce::roundToInt (vel * 127.0f));
        int period = 1;
        std::set<int> distinct;
        for (const auto& z : lib.zones)                 // zones that could answer this key / velocity on the default articulation
            if (! z.triggerRelease && probe >= z.lokey && probe <= z.hikey && vel127 >= z.lovel && vel127 <= z.hivel && (z.swLast < 0 || z.swLast == lib.swDefault))
                period = std::max (period, z.seqLength);
        for (int k = 0; k < period; ++k)
            for (size_t zi = 0; zi < lib.zones.size(); ++zi)
                if (mdl::zoneMatches (lib.zones[zi], probe, vel127, k, lib.swDefault, false)) distinct.insert ((int) zi);
        if (period > 1)
        {
            std::vector<Run> runs;
            for (int k = 0; k < period * 2 + 1; ++k)
            {
                killVoices (p);
                runs.push_back (render (p, noteOn (probe, vel), 8, 512, true));
            }
            auto diff = [&] (const Run& a, const Run& b) { float d = 0.f; for (size_t i = 0; i < a.left.size() && i < b.left.size(); ++i) d = std::max (d, std::abs (a.left[i] - b.left[i])); return d; };
            bool neighboursDiffer = true, wraps = true;
            for (int k = 0; k + 1 < (int) runs.size(); ++k) if (diff (runs[(size_t) k], runs[(size_t) k + 1]) < 1.0e-4f) neighboursDiffer = false;
            for (int k = 0; k + period < (int) runs.size(); ++k) if (diff (runs[(size_t) k], runs[(size_t) k + (size_t) period]) > 1.0e-5f) wraps = false;
            CHECK (neighboursDiffer && wraps, juce::String (spec.id) + ": round robin period " + juce::String (period) + " (" + juce::String ((int) distinct.size()) + " different zones) cycles correctly");
        }
        killVoices (p);
    }

    // ---- sustained loops keep sounding, then release
    {
        bool anyLoop = false;
        for (auto& z : lib.zones) if (z.loop == mdl::Loop::sustain || z.loop == mdl::Loop::continuous) anyLoop = true;
        if (anyLoop)
        {
            const int probe = (spec.lo + spec.hi) / 2;
            killVoices (p);
            render (p, noteOn (mdl::kKeyswitchLo + 1, 1.0f), 1);          // articulation 13: loop in every pitched instrument
            auto r = render (p, noteOn (probe, 0.9f), 560);               // ~6 s at 48 kHz
            auto tail = render (p, {}, 4);
            CHECK (tail.rms > 0.002 && ! r.bad, juce::String (spec.id) + ": looped articulation still sounds after 6 s");
            auto rel = render (p, noteOff (probe), 400);
            auto end = render (p, {}, 4);
            CHECK (end.peak < 1.0e-3f, juce::String (spec.id) + ": released note dies out");
            (void) rel;
            render (p, noteOn (mdl::kKeyswitchLo, 1.0f), 1);               // back to the default articulation
        }
    }

    // ---- percussion lanes + mirror, one-shot (note-off ignored), choke groups
    if (spec.percussive)
    {
        int silent = 0;
        for (int key = 36; key <= 59; ++key)
        {
            killVoices (p);
            auto r = render (p, noteOn (key, 0.85f), 20);
            if (r.peak < 0.003f || r.bad) { ++silent; NOTE ("silent lane key %d\n", key); }
        }
        CHECK (silent == 0, juce::String (spec.id) + ": every key 36..59 (lanes + mirror) triggers a hit");
        killVoices (p);
        render (p, noteOn (36, 1.0f), 4);
        render (p, noteOff (36), 1);
        auto ring = render (p, {}, 40);                                    // one-shot: the hit keeps ringing after note-off
        CHECK (ring.peak > 0.002f, "one-shot rings out after note-off");
        killVoices (p);
        {
            juce::MidiBuffer m;
            for (int k = 0; k < 24; ++k) m.addEvent (juce::MidiMessage::noteOn (1, 36 + (k % 6), 0.9f), k * 20);   // fast repeated strokes: choke groups
            auto r = render (p, m, 40);
            CHECK (! r.bad && r.peak > 0.01f, "fast strokes (choke groups) stay finite");
        }
        killVoices (p);
    }

    // ---- release triggers (violin bow lift)
    if (std::string (spec.id) == "violon")
    {
        for (int sw : { 12, 13 })
        {
            killVoices (p);
            render (p, noteOn (sw, 1.0f), 1);
            render (p, noteOn (64, 0.9f), 12);
            auto r = render (p, noteOff (64), 12);
            CHECK (! r.bad, "violin release path (articulation " + juce::String (sw) + ") finite");
        }
        render (p, noteOn (12, 1.0f), 1);
    }

    // ---- phrases
    {
        const auto& list = phrasesFor (idx);
        int silent = 0;
        for (int k = 0; k < (int) list.size(); ++k)
        {
            killVoices (p);
            auto r = render (p, noteOn (kPhraseLo + k, 0.8f), 260);
            if (r.peak < 0.003f || r.bad) { ++silent; NOTE ("silent phrase '%s'\n", list[(size_t) k].name.toRawUTF8()); }
            render (p, noteOff (kPhraseLo + k), 4);
        }
        CHECK (silent == 0, juce::String (spec.id) + ": all " + juce::String ((int) list.size()) + " phrases play");
        killVoices (p);
    }
}

// ====================================================================================================
static void testSequencerAndPresets()
{
    std::printf ("[Sequencer] derbouka + tar in multi mode (fallback)\n");
    Fixture fx;
    auto& p = fx.proc;
    setP (p, pid::multi, 1.f);
    for (int i = 0; i < kNumInstruments; ++i) setP (p, pid::inst (i, "on"), (i == 5 || i == 6) ? 1.f : 0.f);
    setP (p, pid::selected, 5.f);
    CHECK (waitReady (p, 5) && waitReady (p, 6), "derbouka and tar ready");
    setP (p, pid::seqOn, 1.f);
    auto r = render (p, {}, 300);
    CHECK (r.peak > 0.01f && ! r.bad, "16-step sequencer sounds");
    setP (p, pid::seqOn, 0.f);
    render (p, {}, 100);

    std::printf ("[Presets] every factory preset loads and sounds (fallback)\n");
    setP (p, pid::multi, 0.f);
    int bad = 0;
    const int n = p.presets->getNumPresets();
    for (int i = 0; i < n; ++i)
    {
        p.presets->loadIndex (i);
        pump (30);
        for (int s = 0; s < kNumInstruments; ++s) if (p.isSlotActive (s)) waitReady (p, s);
        killVoices (p);
        juce::MidiBuffer m;
        for (int k : { 40, 52, 62, 70, 80 }) m.addEvent (juce::MidiMessage::noteOn (1, k, 0.9f), k);
        auto rr = render (p, m, 40);
        if (rr.bad || rr.peak < 0.003f) { ++bad; NOTE ("preset %d '%s': peak %.4f\n", i, p.presets->getName (i).toRawUTF8(), rr.peak); }
    }
    CHECK (bad == 0, juce::String (n) + " presets load, sound and stay finite");
}

// ====================================================================================================
static void testHostEdgeCases()
{
    std::printf ("[Host] edge cases\n");
    {
        ChaabiProcessor p;                           // processBlock before prepareToPlay
        juce::AudioBuffer<float> b (2, 512);
        for (int c = 0; c < 2; ++c) for (int i = 0; i < 512; ++i) b.setSample (c, i, 1.0f);
        juce::MidiBuffer m = noteOn (60);
        p.processBlock (b, m);
        CHECK (b.getMagnitude (0, 512) == 0.0f, "processBlock before prepareToPlay: silence, no crash");
    }
    Fixture fx (48000.0, 256);
    auto& p = fx.proc;
    setP (p, pid::selected, 0.f);
    CHECK (waitReady (p, 0), "mandole ready");
    {
        juce::AudioBuffer<float> b (2, 0);
        juce::MidiBuffer m;
        p.processBlock (b, m);
        CHECK (true, "zero-length block");
    }
    {
        juce::AudioBuffer<float> b (2, 20000);           // far larger than the prepared size
        b.clear();
        juce::MidiBuffer m = noteOn (64, 0.9f, 10000);
        p.processBlock (b, m);
        float early = 0.f, late = 0.f;
        bool finite = true;
        for (int i = 0; i < 20000; ++i)
            for (int c = 0; c < 2; ++c)
            {
                const float v = b.getSample (c, i);
                if (! std::isfinite (v)) finite = false;
                (i < 10000 ? early : late) = juce::jmax (i < 10000 ? early : late, std::abs (v));
            }
        CHECK (finite && early < 1.0e-6f && late > 0.003f, "block larger than prepared (20000 > 4096): rendered in chunks, note starts at sample 10000");
    }
    killVoices (p);
    {
        juce::AudioBuffer<float> b (1, 512);             // mono bus
        b.clear();
        juce::MidiBuffer m = noteOn (64, 0.9f);
        p.processBlock (b, m);
        float pk = 0.f;
        for (int blk = 0; blk < 10; ++blk) { juce::MidiBuffer none; p.processBlock (b, none); pk = juce::jmax (pk, b.getMagnitude (0, 512)); }
        CHECK (pk > 0.003f, "mono bus renders (stereo mix-down)");
    }
    killVoices (p);
    {
        juce::AudioBuffer<float> b (5, 512);             // more channels than outputs
        for (int c = 0; c < 5; ++c) for (int i = 0; i < 512; ++i) b.setSample (c, i, 0.7f);
        juce::MidiBuffer m = noteOn (64, 0.9f);
        p.processBlock (b, m);
        CHECK (b.getMagnitude (2, 0, 512) == 0.0f && b.getMagnitude (4, 0, 512) == 0.0f, "extra channels are cleared");
    }
    killVoices (p);
    {
        juce::MidiBuffer m;                              // flood: far more notes than voices, same sample
        for (int k = 0; k < 400; ++k) m.addEvent (juce::MidiMessage::noteOn (1, 50 + (k % 28), 0.8f), 0);
        auto r = render (p, m, 20);
        CHECK (! r.bad && r.peak > 0.003f, "400 simultaneous note-ons (voice stealing)");
        killVoices (p);
    }
    {
        // controllers and extreme parameters
        setP (p, pid::master, 6.f); setP (p, pid::poly, 8.f); setP (p, pid::bendRange, 12.f);
        setP (p, pid::inst (0, "pitch"), 12.f); setP (p, pid::inst (0, "filter"), 80.f); setP (p, pid::inst (0, "atk"), 2.f); setP (p, pid::inst (0, "rel"), 4.f);
        juce::MidiBuffer m;
        m.addEvent (juce::MidiMessage::noteOn (1, 64, 1.0f), 0);
        m.addEvent (juce::MidiMessage::pitchWheel (1, 16383), 50);
        m.addEvent (juce::MidiMessage::controllerEvent (1, 64, 127), 80);
        m.addEvent (juce::MidiMessage::aftertouchChange (1, 64, 100), 90);
        m.addEvent (juce::MidiMessage::channelPressureChange (1, 120), 95);
        m.addEvent (juce::MidiMessage::pitchWheel (1, 0), 200);
        auto r = render (p, m, 40);
        CHECK (! r.bad, "pitch bend / sustain / pressure / extreme parameters stay finite");
        setP (p, pid::inst (0, "pitch"), 0.f); setP (p, pid::inst (0, "filter"), 20000.f); setP (p, pid::inst (0, "atk"), 0.f); setP (p, pid::inst (0, "rel"), 0.f);
        setP (p, pid::master, 0.f); setP (p, pid::poly, 32.f); setP (p, pid::bendRange, 2.f);
        juce::MidiBuffer off; off.addEvent (juce::MidiMessage::controllerEvent (1, 64, 0), 0); off.addEvent (juce::MidiMessage::allNotesOff (1), 1);
        render (p, off, 4);
        killVoices (p);
    }
    {
        // bypass: fades out, stays silent even with new notes, comes back
        render (p, noteOn (64, 0.9f), 6);
        setP (p, pid::bypass, 1.f);
        render (p, {}, 4);
        auto silent = render (p, noteOn (66, 1.0f), 20);
        CHECK (silent.peak < 1.0e-6f && ! silent.bad, "bypass: silent output (voices stopped, MIDI ignored)");
        setP (p, pid::bypass, 0.f);
        killVoices (p);
        auto back = render (p, noteOn (66, 1.0f), 20);
        CHECK (back.peak > 0.003f && ! back.bad, "bypass released: sound returns");
        CHECK (p.getBypassParameter() != nullptr, "bypass parameter is reported to the host");
        juce::AudioBuffer<float> b (2, 256); b.clear(); juce::MidiBuffer m;
        p.processBlockBypassed (b, m);
        CHECK (b.getMagnitude (0, 256) == 0.0f, "processBlockBypassed: silence");
        killVoices (p);
    }

    std::printf ("[Host] sample rates and block sizes\n");
    for (double sr : { 22050.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 })
        for (int bs : { 1, 64, 480, 2048 })
        {
            if (bs == 1 && sr > 50000.0) continue;
            ChaabiProcessor q;
            q.setPlayConfigDetails (0, 2, sr, bs);
            q.setLibraryRoot (uniqueTempDir ("sr"));
            q.prepareToPlay (sr, bs);
            setP (q, pid::selected, (float) ((int) (sr + bs) % 8));
            const int slot = (int) q.apvts.getRawParameterValue (pid::selected)->load();
            const bool ready = waitReady (q, slot);
            bool ok = ready;
            if (ready)
            {
                const int key = (mdl::kSpecs[slot].lo + mdl::kSpecs[slot].hi) / 2;
                const int blocks = juce::jmax (4, (int) (0.3 * sr / bs));
                juce::AudioBuffer<float> b (2, bs);
                float pk = 0.f;
                bool finite = true;
                for (int i = 0; i < blocks && i < 2000; ++i)
                {
                    juce::MidiBuffer m; if (i == 0) m.addEvent (juce::MidiMessage::noteOn (1, key, 0.9f), 0);
                    b.clear(); q.processBlock (b, m);
                    for (int c = 0; c < 2; ++c) for (int s = 0; s < bs; ++s) { const float v = b.getSample (c, s); if (! std::isfinite (v)) finite = false; pk = juce::jmax (pk, std::abs (v)); }
                }
                ok = finite && pk > 0.003f;
            }
            CHECKQ (ok, juce::String ((int) sr) + " Hz / " + juce::String (bs) + " samples: " + juce::String (mdl::kSpecs[slot].id) + " sounds");
        }
    CHECK (true, "all sample rate / block size combinations");
}

// ====================================================================================================
static void testStateAndLibraries()
{
    std::printf ("[State] save / restore with no sample folder\n");
    {
        Fixture fx;
        auto& p = fx.proc;
        setP (p, pid::selected, 3.f);                    // ney
        setP (p, pid::multi, 1.f);
        setP (p, pid::master, -3.f);
        p.setSeqStep (0, 1, 3, 2);
        waitReady (p, 3);
        juce::MemoryBlock mb;
        p.getStateInformation (mb);
        CHECK (mb.getSize() > 100, "state serialised");

        ChaabiProcessor q;
        q.setPlayConfigDetails (0, 2, 48000.0, 512);
        q.setStateInformation (mb.getData(), (int) mb.getSize());
        q.prepareToPlay (48000.0, 512);
        CHECK ((int) q.apvts.getRawParameterValue (pid::selected)->load() == 3 && q.apvts.getRawParameterValue (pid::multi)->load() > 0.5f, "instrument + multi mode restored");
        CHECK (std::abs (q.apvts.getRawParameterValue (pid::master)->load() + 3.f) < 0.01f, "master level restored");
        CHECK (q.getSeqStep (0, 1, 3) == 2, "sequencer grid restored");
        CHECK (q.getLibraryRoot() == p.getLibraryRoot(), "library root restored");
        CHECK (waitReady (q, 3), "restored processor renders the built-in ney");
        setP (q, pid::inst (3, "rev"), 0.f);
        auto r = render (q, noteOn (70, 0.9f), 30);
        CHECK (r.peak > 0.003f && ! r.bad, "restored processor plays");

        const char garbage[] = "this is not a state";
        q.setStateInformation (garbage, (int) sizeof (garbage));
        q.setStateInformation (nullptr, 0);
        CHECK (true, "garbage state ignored");
    }

    std::printf ("[Libraries] SFZ precedence, round trip, unusable libraries\n");
    {
        const auto dir = uniqueTempDir ("lib");
        // Round trip: write the model library of the mandole as an SFZ (32-bit float WAV = lossless) and load it with the engine's loader.
        const int mandole = 0;
        auto lib = mdl::build (mandole, 44100.0);
        auto wr = mdl::writer::writeLibrary (lib, mandole, dir.getFullPathName().toStdString(), 32, true);
        CHECK (wr.ok, "SFZ + WAV export of the mandole library (" + juce::String (wr.wavFiles) + " files)");

        std::atomic<float> progress { 0.f };
        juce::String err;
        auto fromSfz = Bank::load (juce::File (juce::String (wr.sfzPath.string())), progress, [] { return false; }, err);
        auto fromLib = Bank::fromLibrary (lib, err);
        CHECK (fromSfz != nullptr && fromLib != nullptr && fromSfz->regions.size() == fromLib->regions.size(),
               "SFZ export loads with the same number of regions (" + juce::String (fromLib != nullptr ? fromLib->regions.size() : -1) + ")");
        if (fromSfz != nullptr && fromLib != nullptr && fromSfz->regions.size() == fromLib->regions.size())
        {
            bool same = fromSfz->swLo == fromLib->swLo && fromSfz->swHi == fromLib->swHi && fromSfz->swDefault == fromLib->swDefault && fromSfz->hasReleaseRegions == fromLib->hasReleaseRegions;
            double maxDiff = 0.0;
            for (int i = 0; i < fromLib->regions.size() && same; ++i)
            {
                const auto& a = fromSfz->regions[i]->def;
                const auto& b = fromLib->regions[i]->def;
                same = a.lokey == b.lokey && a.hikey == b.hikey && a.keycenter == b.keycenter && a.lovel == b.lovel && a.hivel == b.hivel
                    && a.seqLength == b.seqLength && a.seqPosition == b.seqPosition && a.swLast == b.swLast && a.loop == b.loop
                    && a.loopStart == b.loopStart && a.loopEnd == b.loopEnd && a.group == b.group && a.offBy == b.offBy
                    && a.triggerRelease == b.triggerRelease && std::abs (a.ampRelease - b.ampRelease) < 1.0e-5f && std::abs (a.ampAttack - b.ampAttack) < 1.0e-5f
                    && std::abs (a.ampVeltrack - b.ampVeltrack) < 1.0e-4f && std::abs (a.pitchKeytrack - b.pitchKeytrack) < 1.0e-4f;
                const auto& da = *fromSfz->regions[i]->data;
                const auto& db = *fromLib->regions[i]->data;
                same = same && da.numFrames == db.numFrames && da.audio.getNumChannels() == db.audio.getNumChannels();
                if (same)
                    for (int s = 0; s < da.numFrames; s += 7) maxDiff = std::max (maxDiff, (double) std::abs (da.audio.getSample (0, s) - db.audio.getSample (0, s)));
            }
            CHECK (same && maxDiff < 1.0e-6, "exported SFZ describes exactly the in-memory regions (mapping, loops, envelopes, audio; max sample diff " + juce::String (maxDiff, 8) + ")");
        }

        // The same SFZ placed in a library folder takes precedence over the built-in instrument.
        {
            ChaabiProcessor p;
            p.setPlayConfigDetails (0, 2, 48000.0, 512);
            p.setLibraryRoot (dir);
            p.prepareToPlay (48000.0, 512);
            setP (p, pid::selected, 0.f);
            CHECK (waitReady (p, 0), "user SFZ library loads");
            const auto st = p.getSlotStatus (0);
            CHECK (! st.builtIn && st.message.contains ("regions"), "user SFZ takes precedence over the built-in instrument: \"" + st.message + "\"");
            setP (p, pid::inst (0, "rev"), 0.f);
            auto r = render (p, noteOn (64, 0.9f), 30);
            CHECK (r.peak > 0.003f && ! r.bad, "user SFZ plays");

            // partially missing samples: the library still loads, the missing ones are reported as warnings
            {
                int removed = 0;
                for (auto& f : dir.getChildFile ("Mandole/samples/pick").findChildFiles (juce::File::findFiles, false, "*_rr3.wav"))
                    if (removed < 3 && f.deleteFile()) ++removed;
                std::atomic<float> prog { 0.f };
                juce::String e2;
                auto partial = Bank::load (dir.getChildFile ("Mandole/Mandole.sfz"), prog, [] { return false; }, e2);
                CHECK (partial != nullptr && partial->warnings.size() >= removed && partial->regions.size() > 40, "SFZ with " + juce::String (removed) + " missing samples loads partially with warnings");
            }

            // deleting the library and re-selecting the folder switches back to the built-in instrument
            dir.getChildFile ("Mandole").deleteRecursively();
            p.setLibraryRoot (dir);
            pump (60);
            CHECK (waitReady (p, 0) && p.getSlotStatus (0).builtIn, "library removed + folder re-selected: back to the built-in instrument");
        }

        // Unusable libraries must fall back to the built-in instrument, with an explanation, never silence.
        const auto bad = uniqueTempDir ("bad");
        bad.getChildFile ("Ney").createDirectory();
        bad.getChildFile ("Ney/Ney.sfz").replaceWithText ("<region> sample=missing/nothing.wav lokey=60 hikey=93 pitch_keycenter=70\n<region> sample=missing/other.wav lokey=60 hikey=93\n");
        bad.getChildFile ("Qanun").createDirectory();
        bad.getChildFile ("Qanun/Qanun.sfz").replaceWithText ("// empty library\n");
        bad.getChildFile ("Tar/samples").createDirectory();
        bad.getChildFile ("Tar/Tar.sfz").replaceWithText ("<control> default_path=samples/\n<region> sample=broken.wav lokey=36 hikey=59\n");
        {
            juce::MemoryBlock junk; junk.setSize (4096);
            for (size_t i = 0; i < 4096; ++i) junk[i] = (char) (i * 31 + 7);
            bad.getChildFile ("Tar/samples/broken.wav").replaceWithData (junk.getData(), junk.getSize());
        }
        {
            ChaabiProcessor p;
            p.setPlayConfigDetails (0, 2, 48000.0, 512);
            p.setLibraryRoot (bad);
            p.prepareToPlay (48000.0, 512);
            for (int slot : { 3, 4, 6 })
            {
                setP (p, pid::selected, (float) slot);
                setP (p, pid::inst (slot, "rev"), 0.f);
                CHECK (waitReady (p, slot), juce::String (mdl::kSpecs[slot].id) + ": unusable SFZ library -> fallback ready");
                const auto st = p.getSlotStatus (slot);
                NOTE ("status: %s\n", st.message.toRawUTF8());
                CHECK (st.builtIn && st.warning && st.message.contains ("SFZ library not usable"), juce::String (mdl::kSpecs[slot].id) + ": explanation shown to the user");
                const int key = slot == 6 ? 38 : (mdl::kSpecs[slot].lo + mdl::kSpecs[slot].hi) / 2;
                auto r = render (p, noteOn (key, 0.9f), 30);
                CHECK (r.peak > 0.003f && ! r.bad, juce::String (mdl::kSpecs[slot].id) + ": plays through the fallback");
            }
        }
        dir.deleteRecursively();
        bad.deleteRecursively();
    }

    std::printf ("[Bank] cancellation and destruction while rendering\n");
    {
        std::atomic<float> progress { 0.f };
        juce::String err;
        int polls = 0;
        auto bank = Bank::buildFromModel (4, 44100.0, progress, [&] { return ++polls > 2; }, err);
        // (a cached bank from an earlier test may be returned instantly; both outcomes are valid, a crash is not)
        CHECK (bank == nullptr ? err == "Cancelled" : bank->regions.size() > 0, "buildFromModel honours cancellation or serves the cached bank");
        auto bank2 = Bank::buildFromModel (4, 44100.0, progress, [] { return false; }, err);
        CHECK (bank2 != nullptr && bank2->builtIn && bank2->regions.size() > 40, "buildFromModel produces a bank");
        auto bank3 = Bank::buildFromModel (4, 44100.0, progress, [] { return false; }, err);
        CHECK (bank3 == bank2, "identical requests share one cached bank");
    }
    for (int i = 0; i < 6; ++i)
    {
        const auto t0 = std::chrono::steady_clock::now();
        {
            auto* p = new ChaabiProcessor();
            p->setPlayConfigDetails (0, 2, 48000.0, 512);
            p->setLibraryRoot (uniqueTempDir ("destroy"));
            p->prepareToPlay (48000.0, 512);
            setP (*p, pid::selected, (float) (i % 8));
            pump (5 + i * 10);
            delete p;                                    // possibly while the background render is running
        }
        const double ms = std::chrono::duration<double, std::milli> (std::chrono::steady_clock::now() - t0).count();
        CHECKQ (ms < 8000.0, "destruction while rendering took " + juce::String (ms) + " ms");
    }
    CHECK (true, "processors destroyed while a render was running (6 times)");
}

// ====================================================================================================
static void collectTexts (juce::Component& c, juce::StringArray& out)
{
    if (auto* b = dynamic_cast<juce::Button*> (&c)) out.add (b->getButtonText());
    if (auto* l = dynamic_cast<juce::Label*> (&c)) out.add (l->getText());
    if (auto* s = dynamic_cast<juce::Slider*> (&c)) out.add (s->getName());
    if (auto* cb = dynamic_cast<juce::ComboBox*> (&c)) for (int i = 0; i < cb->getNumItems(); ++i) out.add (cb->getItemText (i));
    for (auto* ch : c.getChildren()) collectTexts (*ch, out);
}

static bool looksFrench (const juce::String& s)
{
    for (auto ch : s)
        if (ch >= 0x00C0 && ch <= 0x00FF && ch != 0x00D7 && ch != 0x00F7) return true;           // accented Latin-1 letters
    static const char* const words[] = { "Taille", "Amorti", "Temps", "Filtre", "Zone debut", "Sequenceur", "Polyphonie", "Actif", "Annuler", "Dossier",
                                         "Chargement", "introuvable", "Enregistrer", "Biblioth", "Naturel", "Salle", "Utilisateur", "Mix chaabi", "Violon (alto)", "Saisir", "pinc" };
    for (auto* w : words) if (s.containsIgnoreCase (w)) return true;
    return false;
}

static void testEnglishText()
{
    std::printf ("[UI] English-only user-visible text\n");
    Fixture fx;
    auto& p = fx.proc;
    setP (p, pid::selected, 0.f);
    waitReady (p, 0);

    juce::StringArray texts;
    for (int i = 0; i < kNumInstruments; ++i) { texts.add (instrumentName (i)); texts.add (juce::String::fromUTF8 (kInstruments[i].tagline)); for (auto& ph : phrasesFor (i)) texts.add (ph.name); }
    for (int i = 0; i < p.presets->getNumPresets(); ++i) { texts.add (p.presets->getName (i)); texts.add (p.presets->getCategory (i)); }
    for (auto* prm : p.getParameters()) texts.add (prm->getName (128));
    for (int s = 0; s < kNumInstruments; ++s) texts.add (p.getSlotStatus (s).message);

    {
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1280, 760);
        for (int i = 0; i < 6; ++i) { pump (50); silentBlock (p); }
        collectTexts (*ed, texts);
        bool builtInShown = false;
        for (auto& t : texts) if (t.startsWith ("Built-in modelled instrument")) builtInShown = true;
        CHECK (builtInShown, "editor shows \"Built-in modelled instrument\" for the selected slot");
        setP (p, pid::selected, 5.f);
        for (int i = 0; i < 12; ++i) { pump (50); silentBlock (p); }
        juce::StringArray t2; collectTexts (*ed, t2);
        for (auto& t : t2) texts.add (t);
        NOTE ("editor texts collected: %d\n", t2.size());
    }

    int french = 0;
    for (auto& t : texts) if (looksFrench (t)) { ++french; NOTE ("non-English text: \"%s\"\n", t.toRawUTF8()); }
    CHECK (french == 0, "no French / accented user-visible string (" + juce::String (texts.size()) + " strings scanned)");
    CHECK (juce::String (kInstruments[2].name) == "Violin", "violin display name is \"Violin\"");
}

// ====================================================================================================
static void testSfzLibrary (const juce::File& dir)
{
    std::printf ("[SFZ] user library: %s\n", dir.getFullPathName().toRawUTF8());
    ChaabiProcessor p;
    p.setPlayConfigDetails (0, 2, 48000.0, 512);
    p.setLibraryRoot (dir);
    p.prepareToPlay (48000.0, 512);
    setP (p, pid::selected, 0.f);
    if (waitReady (p, 0) && ! p.getSlotStatus (0).builtIn)
    {
        NOTE ("%s\n", p.getSlotStatus (0).message.toRawUTF8());
        setP (p, pid::inst (0, "rev"), 0.f);
        auto r = render (p, noteOn (60, 0.9f), 40);
        CHECK (r.peak > 0.02f && ! r.bad, "SFZ mandole note plays");
    }
    else NOTE ("no mandole SFZ library with samples in this folder: skipped\n");
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    juce::File samples, uiOut;
    juce::String uiTab;
    bool uiOnly = false;
    for (int i = 1; i < argc; ++i)
    {
        const juce::String a (argv[i]);
        if (a == "--samples" && i + 1 < argc) samples = juce::File (argv[++i]);
        else if ((a == "--ui" || a == "--ui-only") && i + 1 < argc) { uiOnly = (a == "--ui-only"); uiOut = juce::File (argv[++i]); if (i + 1 < argc && argv[i + 1][0] != '-') uiTab = argv[++i]; }
        else if (i == 1 && ! a.startsWith ("-")) samples = juce::File (a);                      // legacy: first argument = samples folder
    }

    const auto t0 = juce::Time::getMillisecondCounterHiRes();
    if (! uiOnly)
    {
        testModel();
        for (int i = 0; i < kNumInstruments; ++i) testInstrument (i);
        testSequencerAndPresets();
        testHostEdgeCases();
        testStateAndLibraries();
        testEnglishText();
        if (samples != juce::File()) testSfzLibrary (samples);
    }

    if (! uiOnly)
    {
        std::printf ("[CPU] 16 voices (built-in mandole)\n");
        Fixture fx;
        auto& p = fx.proc;
        setP (p, pid::selected, 0.f);
        waitReady (p, 0);
        juce::MidiBuffer m;
        for (int n = 52; n < 68; ++n) m.addEvent (juce::MidiMessage::noteOn (1, n, 0.9f), 0);
        juce::AudioBuffer<float> buf (2, 512);
        const auto c0 = juce::Time::getMillisecondCounterHiRes();
        for (int b = 0; b < 200; ++b) { juce::MidiBuffer mm; if (b == 0) mm = m; p.processBlock (buf, mm); }
        const double ms = juce::Time::getMillisecondCounterHiRes() - c0;
        const double budget = 200.0 * 512.0 / 48000.0 * 1000.0;
        std::printf ("  %.1f ms for %.1f ms of audio (%.1f %% CPU, build %s)\n", ms, budget, 100.0 * ms / budget,
#if JUCE_DEBUG
                     "Debug"
#else
                     "Release"
#endif
        );
    }

    if (uiOut != juce::File())          // editor capture: HumpireSmokeTest --ui out.png [PLAY|PHRASES|PATTERNS]
    {
        Fixture fx;
        auto& p = fx.proc;
        setP (p, pid::multi, 0.f); setP (p, pid::selected, 5.f);
        waitReady (p, 5);
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        ed->setSize (1280, 760);
        pump (400);
        if (uiTab.isNotEmpty())
        {
            std::function<void (juce::Component&)> rec = [&] (juce::Component& c)
            {
                if (auto* b = dynamic_cast<juce::TextButton*> (&c)) if (b->getButtonText() == uiTab) b->triggerClick();
                for (auto* ch : c.getChildren()) rec (*ch);
            };
            rec (*ed);
            pump (300);
        }
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        uiOut.deleteFile();
        juce::PNGImageFormat png;
        if (auto os = uiOut.createOutputStream()) png.writeImageToStream (img, *os);
        std::printf ("UI capture: %s\n", uiOut.getFullPathName().toRawUTF8());
    }

    std::printf ("\n%s (%d check(s), %d failure(s), %.1f s)\n", failures == 0 ? "ALL TESTS PASSED" : "FAILED", checks, failures,
                 (juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0);
    return failures == 0 ? 0 : 1;
}
