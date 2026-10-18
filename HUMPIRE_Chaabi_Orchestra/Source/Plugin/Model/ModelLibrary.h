#pragma once
// ============================================================================
//  HUMPIRE CHAABI ORCHESTRA - built-in modelled library (interface)
//
//  Pure C++17, standard library only (no JUCE).  Every instrument has an offline
//  renderer that SYNTHESISES its own sample set (physical modelling / modal /
//  waveguide / additive + formant + noise).  The plugin renders it on a background
//  thread when no user SFZ library is found, and turns the zones into the same
//  Region + SampleData objects an SFZ library would produce.
//
//  This is a MODELLED instrument set, not a recording of real instruments.
//
//  Rules for renderers (the 8 build* functions in the three family files)
//  ----------------------------------------------------------------------
//  * Pure functions of (BuildContext): deterministic (same input -> same output
//    on the same binary), re-entrant and thread-safe: NO static / global mutable
//    state, NO std::rand, NO time-based seeds, NO I/O.  Use model::Rng (ModelCommon.h)
//    seeded from the instrument / zone index.
//  * Allocation is allowed inside the renderer only (it never runs on the audio thread).
//  * Honour ctx.sampleRate for every time constant and write it into Zone::sampleRate.
//  * Call ctx.progress(fraction) regularly (at least once per zone).  When it
//    returns false the caller wants to cancel: set lib.cancelled = true and return.
//  * Budget per instrument at 44.1 kHz: <= 25 MB of float samples in total
//    (Library::megabytes(), alias zones are free), <= 3 s single-thread render time.
//    Prefer mono, a sample every 3-4 semitones, 2-3 velocity layers.
//  * Every zone must start at (almost) zero, end faded to zero, have no DC offset
//    and no NaN/Inf, and peak <= -1 dBFS.  See "Level conventions" below.
//  * The produced key map is a CONTRACT with the engine, the phrase bank, the step
//    sequencer and the factory presets.  See "INSTRUMENT SPECIFICATIONS" at the end of
//    this file.  model::validate() (used by the smoke test and by ChaabiRenderLibrary)
//    checks most of it.
//
//  Zone semantics are exactly the SFZ semantics implemented by the engine
//  (SfzRegionDef / Region::matches / ChaabiVoice), see the comments on Zone.
//
//  Author workflow (no JUCE needed, a few seconds per iteration)
//  ----------------------------------------------------------------------
//    ninja -C <build> ChaabiRenderLibrary
//    <build>/ChaabiRenderLibrary --instrument mandole --validate-only        # render + validate + timing only
//    <build>/ChaabiRenderLibrary --out /tmp/lib --instrument mandole --bits 24   # WAV + SFZ to listen to
//    python3 tools/analyze_library.py /tmp/lib --instrument Mandole [--failures-only] [--spectrograms /tmp/specs]
//    ninja -C <build> HumpireSmokeTest && <build>/Tests/HumpireSmokeTest_artefacts/Release/HumpireSmokeTest
//  The exported folder is also a valid user SFZ library (copy it to the Samples folder to hear it in the plug-in).
// ============================================================================
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace humpire::model
{
inline constexpr int kNumModelInstruments = 8;

// ----------------------------------------------------------------------------------------
// Static description of the 8 instruments (index == index in humpire::kInstruments).
// The plugin cross-checks this table against kInstruments with static_asserts.
// ----------------------------------------------------------------------------------------
struct InstrumentSpec
{
    const char* id;        // "mandole", ...
    const char* folder;    // SFZ folder + file stem ("Mandole" -> Mandole/Mandole.sfz)
    int lo, hi;            // key range the library MUST cover (inclusive)
    bool percussive;       // one-shot lanes, no pitch tracking
};

inline constexpr InstrumentSpec kSpecs[kNumModelInstruments] = {
    { "mandole",  "Mandole",  50, 78, false },
    { "banjo",    "Banjo",    55, 91, false },
    { "violon",   "Violon",   55, 96, false },
    { "ney",      "Ney",      60, 93, false },
    { "qanun",    "Qanun",    48, 96, false },
    { "derbouka", "Derbouka", 36, 59, true  },   // 36-47 primary map, 48-59 mirror
    { "tar",      "Tar",      36, 59, true  },
    { "bendir",   "Bendir",   36, 59, true  },
};

// Keyboard areas shared with the plugin (see Instruments.h).
inline constexpr int kKeyswitchLo = 12, kKeyswitchHi = 23;   // articulations (sw_last)
inline constexpr int kPhraseLo = 24, kPhraseHi = 35;          // phrase triggers: never reach a zone
inline constexpr int kPlayLo = 36;

// Memory budget per instrument (bytes of float samples, alias zones excluded).
inline constexpr std::size_t kMaxLibraryBytes = 25u * 1024u * 1024u;

enum class Loop { none, oneShot, continuous, sustain };

// ----------------------------------------------------------------------------------------
// One playable sample zone == one SFZ <region>.
// ----------------------------------------------------------------------------------------
struct Zone
{
    // --- identity (used for the WAV / SFZ export; [a-z0-9_] only, name unique per library)
    std::string articulation = "main";   // sub-folder + grouping key: "pick", "tremolo", "dum", ...
    std::string name;                    // file stem, e.g. "pick_051_soft_rr1"

    // --- audio
    std::vector<float> data;             // mono, or interleaved stereo when numChannels == 2.
                                         // Empty when sharedWith >= 0 (alias, no memory).
    int numChannels = 1;                 // 1 or 2
    double sampleRate = 44100.0;         // sample rate of `data` (the engine resamples on playback)
    int sharedWith = -1;                 // >= 0: reuse the audio of zones[sharedWith]. That zone must have
                                         // a LOWER index, must not itself be an alias and must have the same
                                         // channel count / sample rate / loop points (copy them).

    // --- mapping (SFZ: lokey hikey pitch_keycenter lovel hivel seq_length seq_position sw_last)
    int lokey = 0, hikey = 127, keycenter = 60;
    int lovel = 1, hivel = 127;          // velocities are 1..127 (the engine clamps note velocity to >= 1)
    int seqLength = 1, seqPosition = 1;  // round robin: zone plays when (noteCounter % seqLength) + 1 == seqPosition.
                                         // The counter is per MIDI note and advances on every note-on of that key.
    int swLast = -1;                     // articulation keyswitch (12..23) this zone belongs to; -1 = all articulations

    // --- playback (SFZ: volume pan tune pitch_keytrack ampeg_* amp_veltrack)
    float volumeDb = 0.f;                // static gain
    float pan = 0.f;                     // -100 (left) .. +100 (right)
    float tuneCents = 0.f;
    float pitchKeytrack = 100.f;         // cents per key; 0 for unpitched percussion (zone then plays at its natural pitch on every key)
    float ampAttack = 0.f;               // seconds
    float ampDecay = 0.f;                // seconds
    float ampSustain = 100.f;            // percent
    float ampRelease = 0.03f;            // seconds, applied on note-off (ignored by one-shot zones)
    float ampVeltrack = 100.f;           // percent: gain = (1 - vt) + vt * velocity^2

    // --- looping (SFZ: loop_mode loop_start loop_end).  Frame indices.  loopEnd is the LAST frame
    //     of the loop (inclusive); playback then continues at loopStart.  The engine's Hermite
    //     interpolator reads 2 frames beyond loopEnd: use model::finishLoop() which appends 4 guard
    //     frames (= the frames starting at loopStart) so the seam is exact.
    Loop loop = Loop::none;              // none: plays to the end, note-off applies ampRelease.
                                         // oneShot: plays to the end and ignores note-off (percussion).
                                         // continuous / sustain: loops [loopStart, loopEnd] while the note is held
    int loopStart = -1, loopEnd = -1;

    // --- exclusive groups (percussion choke).  Starting a zone with group g quickly releases (12 ms)
    //     all sounding voices whose zone has offBy == g.  0 = none.
    int group = 0, offBy = 0;

    // --- release trigger: played on note-off instead of note-on (SFZ trigger=release).  Always
    //     treated as one-shot by the engine, plays at 0.6 x the note-on velocity.  Must use
    //     seqLength == 1 (the engine evaluates release zones with round-robin counter 0).
    bool triggerRelease = false;

    std::size_t numFrames() const { return numChannels > 0 ? data.size() / (std::size_t) numChannels : 0; }
};

// ----------------------------------------------------------------------------------------
struct Library
{
    std::string instrumentId;            // "mandole", ...
    std::vector<Zone> zones;
    int swLo = -1, swHi = -1, swDefault = -1;   // keyswitch range + default articulation (-1: no keyswitches)
    bool cancelled = false;              // set by a renderer that obeyed a cancel request (zones may be partial)

    // Zone that owns the audio of zone i (resolves aliases).
    const Zone& audioZone (std::size_t i) const
    {
        const Zone& z = zones[i];
        return z.sharedWith >= 0 && (std::size_t) z.sharedWith < zones.size() ? zones[(std::size_t) z.sharedWith] : z;
    }
    std::size_t numFrames (std::size_t i) const { return audioZone (i).numFrames(); }
    std::size_t totalFloats() const { std::size_t n = 0; for (auto& z : zones) n += z.data.size(); return n; }
    double megabytes() const { return (double) totalFloats() * 4.0 / (1024.0 * 1024.0); }
};

// ----------------------------------------------------------------------------------------
// Build context handed to every renderer.
// ----------------------------------------------------------------------------------------
struct BuildContext
{
    int instrument = 0;                              // index into kSpecs
    double sampleRate = 44100.0;                     // render rate (the plugin always asks for 44100)
    std::function<bool (float)> onProgress;          // optional; returns false to request cancellation

    // Report progress in [0,1].  Returns false when the renderer should stop (then set lib.cancelled).
    bool progress (float fraction) const { return ! onProgress || onProgress (fraction < 0.f ? 0.f : fraction > 1.f ? 1.f : fraction); }
    const InstrumentSpec& spec() const { return kSpecs[instrument]; }
};

// ----------------------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------------------
// Renders the library of instrument `instrumentIndex` (0..7).  Thread-safe; may run several at once.
// Applies a last defensive sanitation (NaN/Inf -> 0, hard clip at +-0.999) to every zone.
Library build (int instrumentIndex, double sampleRate = 44100.0, const std::function<bool (float)>& onProgress = {});

// -1 when unknown.
int findInstrument (const std::string& id);

// The engine's zone selection rule (same as humpire::Region::matches, without lorand/hirand).
inline bool zoneMatches (const Zone& z, int note, int vel127, int seqIndex, int activeSwitch, bool release) noexcept
{
    if (z.triggerRelease != release) return false;
    if (note < z.lokey || note > z.hikey) return false;
    if (vel127 < z.lovel || vel127 > z.hivel) return false;
    if (z.seqLength > 1 && (seqIndex % z.seqLength) + 1 != z.seqPosition) return false;
    if (z.swLast >= 0 && z.swLast != activeSwitch) return false;
    return true;
}

struct ValidationReport
{
    std::vector<std::string> errors, warnings;
    bool ok() const { return errors.empty(); }
};

// Structural check of a finished library against the contract in this file:
// field ranges, finite samples, loops, alias rules, memory budget, key/velocity/round-robin
// coverage of the instrument's range for every articulation, no overlapping zones,
// keyswitch / phrase areas unused, percussion rules.
ValidationReport validate (const Library&, int instrumentIndex);

// Family entry points (one per instrument).  Defined in:
//   PluckedModels.cpp      buildMandole, buildBanjo, buildQanun
//   BowedWindModels.cpp    buildViolon, buildNey
//   PercussionModels.cpp   buildDerbouka, buildTar, buildBendir
Library buildMandole  (const BuildContext&);
Library buildBanjo    (const BuildContext&);
Library buildViolon   (const BuildContext&);
Library buildNey      (const BuildContext&);
Library buildQanun    (const BuildContext&);
Library buildDerbouka (const BuildContext&);
Library buildTar      (const BuildContext&);
Library buildBendir   (const BuildContext&);

// ============================================================================
//  Level conventions (checked by tools/analyze_library.py)
//  * "Loudness" = highest RMS over any 50 ms window, in dBFS (model::loudnessDb).
//  * Peak of every zone: -24 dBFS <= peak <= -1 dBFS.  Aim at -2.5 .. -8 dBFS for hard layers.
//  * Hard layer (vel 81-127) of every articulation: loudness about -14 dBFS (+-3 dB between keys,
//    round robins and articulations; model::kHardLoudnessDb) - never above a peak of -2.5 dBFS.
//    Soft layer (vel 1-80): about 6 dB quieter (model::kSoftLoudnessDb) and darker.
//    One-layer articulations (tremolo, roll, vibrato, breathy, pizz, ghost, ...): -16 .. -22 dBFS.
//  * Inside one (articulation, velocity layer) group all zones must be within +-4 dB of each other.
//  * |DC offset| < 0.002, first frame ~0, last frames faded to ~0.
//  * Pitched zones: measured pitch within 8 cents of keycenter (A4 = 440 Hz, MIDI 69).
//  * Looped zones: seamless seam (use model::findLoopPoints / crossfadeLoop / finishLoop).
//
//  INSTRUMENT SPECIFICATIONS (the contract; "REQUIRED" is enforced by model::validate or by the
//  smoke test; "OPTIONAL" may be added freely but must not break the rules above).
//  Velocity layers: soft = 1..80, hard = 81..127 unless noted.  Keyswitch notes are 12..23
//  (swLo = 12, swHi = 23, swDefault = 12 for every pitched instrument; percussion has no keyswitches).
//  Notes 24..35 are intercepted by the plugin (phrase triggers) and never reach a zone.
//  Pitched key spans come from model::makeKeySpans(lo, hi, step) (ModelCommon.h): spans of `step`
//  keys, the sample's keycenter in the middle (max pitch shift +-step/2 semitones).
//
//  MANDOLE  (Mandole, "Algerian mandole") range 50..78
//    12 "pick"    REQUIRED  makeKeySpans(50,78,3); soft+hard layers; 3 round robins (seqLength 3);
//                           loop none; ampRelease 0.30; ampVeltrack 70.  <= ~1.6 s per zone.
//    13 "tremolo" REQUIRED  same spans; ONE layer (1..127); Loop::continuous seamless; ampAttack 0.02,
//                           ampRelease 0.35; sustained plectrum tremolo (>= 0.8 s of loop body).
//  BANJO    (Banjo, "tenor banjo") range 55..91
//    12 "pick"    REQUIRED  makeKeySpans(55,91,3); soft+hard; 2 round robins; short bright decay;
//                           ampRelease 0.12.
//    13 "roll"    REQUIRED  same spans; one layer; Loop::continuous seamless; banjo roll / tremolo.
//  VIOLON   (Violin) range 55..96
//    12 "sustain" REQUIRED  makeKeySpans(55,96,3); soft+hard; Loop::sustain; ampAttack ~0.05; ampRelease ~0.25.
//    13 "vibrato" REQUIRED  same spans; one layer; Loop::sustain with the vibrato baked into the loop
//                           (loop length = whole number of vibrato cycles); ampRelease ~0.25.
//    14 "pizz"    REQUIRED  same spans; one layer; loop none; short plucked-string decay.
//    "bowlift"    REQUIRED  release trigger (triggerRelease = true, seqLength 1, Loop::oneShot,
//                           pitchKeytrack 0, volumeDb about -14): short bow-lift / string-settle noise,
//                           keys 55..96, one zone for swLast 12 and one for swLast 13 (the second may be
//                           an alias of the first).  Nothing for pizz.
//  NEY      (Ney, end-blown flute) range 60..93
//    12 "sustain" REQUIRED  makeKeySpans(60,93,3); soft+hard; Loop::sustain; ampAttack ~0.06.
//    13 "breathy" REQUIRED  same spans; one layer (or two); Loop::sustain; high noise-to-tone ratio.
//    14 "air"     OPTIONAL  unpitched breath noise, keys 60..93, pitchKeytrack 0, Loop::sustain.
//  QANUN    (Qanun, zither) range 48..96
//    12 "pluck"   REQUIRED  makeKeySpans(48,96,4) (12-13 spans); soft+hard; 2 round robins preferred
//                           (1 allowed); loop none; ampRelease ~0.25; long ringing tails (keep the
//                           25 MB budget: ~2.2 s with 2 RR, ~3.5 s with 1 RR).
//    13 "tremolo" REQUIRED  same spans; one layer; Loop::continuous seamless rapid re-plucking.
//    14 "harmonic" OPTIONAL octave harmonics.
//  DERBOUKA (goblet drum) keys 36..47 primary + 48..59 mirror; Loop::oneShot, pitchKeytrack 0,
//    keycenter = lokey, no keyswitches.  Lanes (soft+hard, round robins in brackets):
//      36-37 "dum"   (3)   deep bass stroke           group 1, offBy 1
//      38-39 "tek"   (3)   high rim stroke            group 2, offBy 2
//      40    "ka"    (3)   closed finger/fingertip    group 2, offBy 2
//      41-42 "slap"  (2)   open palm slap             group 2, offBy 2
//      43-44 "ghost" (3)   ONE layer 1..127, very soft group 2, offBy 2
//      45 / 46 / 47        aliases of tek / ka / slap (no extra memory)
//      48-59               mirror of 36-47 (key + 12), aliases (model::addKeyAliases)
//  TAR      (frame drum with jingles) keys 36..47 primary + 48..59 mirror; Loop::oneShot, pitchKeytrack 0.
//      36-37 "dum"   (3)   centre bass stroke         group 1, offBy 1
//      38-39 "tek"   (3)   rim stroke                 group 1, offBy 1
//      40-41 "ka"    (3)   soft finger/rim stroke     group 1, offBy 1   (the step sequencer's KA lane = 40)
//      42-44 "zil"   (3)   jingle (cymbalettes) shake group 3, offBy 3   (sequencer ZIL lane = 42)
//      45-47 "ghost" (3)   ONE layer, very soft       group 1, offBy 1   (sequencer GHOST lane = 45)
//      48-59               mirror of 36-47 (key + 12)
//  BENDIR   (frame drum with snare) keys 36..47 primary + 48..59 mirror; Loop::oneShot, pitchKeytrack 0.
//      36-37 "dum"   (3)   centre stroke              group 1, offBy 1
//      38-39 "tek"   (3)   edge stroke                group 1, offBy 1
//      40-41 "ka"    (3)   fingertip stroke           group 1, offBy 1
//      42-43 "slap"  (2)   palm slap with snare buzz  group 1, offBy 1
//      44-45 "ghost" (3)   ONE layer, very soft       group 1, offBy 1
//      46-47 "roll"  (2)   ONE layer, finger roll with snare buzz, 0.6-1.0 s   group 2, offBy 1
//      48-59               mirror of 36-47 (key + 12)
//  What the rest of the plugin expects:
//    * PhraseBank melodic phrases play notes 62..81 (mandole, banjo, violon, ney: 62..75; qanun: 62..81)
//      with velocities 85..100 at 1/8 .. 2 beat steps (violin/ney notes last 2 beats): every key of the
//      range must sound on the DEFAULT articulation (12).
//    * Percussion phrases use notes 36 (DUM) 38 (TEK) 40 (KA) 41 (SLAP) 42 (ZIL, tar) 43 (GHOST), velocities
//      45..112, 0.2 beat gates (percussion is one-shot, the gate is irrelevant).
//    * The 16-step sequencer sends derbouka {36,38,40,41,43} and tar {36,38,40,42,45} with velocity 88
//      (normal) or 118 (accent), potentially 16 notes per bar at 200 bpm: tails must not pile up
//      (hence the choke groups).
//    * Factory presets put tar/bendir on keys 48..59 (hence the mirror) and derbouka on 36..47.
// ============================================================================
} // namespace humpire::model
