#include "SaturnPresets.h"
#include "SaturnParams.h"

using namespace saturn;

namespace
{
    /** Voice settings of a preset: on, semitones, degrees, level (dB), pan (-100..100), detune (cents). */
    struct VoiceP { bool on; int semi, deg; float level, pan, detune; };

    constexpr VoiceP kOff { false, 0, 0, -6.0f, 0.0f, 0.0f };

    struct P
    {
        const char* category;
        const char* name;
        int mode;               // IntervalMode
        int scale;              // Scale
        int key;                // 0 = C
        float glide;            // ms
        float mix;              // %
        VoiceP voices[kNumVoices];
    };

    // Degree intervals are scale steps relative to the sung note; semitone fields approximate them.
    const P kList[] = {
        // ---- Trap Vocals ----
        { "Trap Vocals", "Trap Stack Thirds", ModeDegrees, Minor, 0, 20.0f, 100.0f, {
            { true,   4,  2,  -6.0f, -35.0f,  0.0f }, { true,   7,  4,  -8.0f,  35.0f,  0.0f }, kOff, kOff } },
        { "Trap Vocals", "Trap Octave Double", ModeDegrees, Minor, 9, 15.0f, 100.0f, {
            kOff, kOff, { true, -12, -7, -10.0f, -15.0f,  4.0f }, { true, 12, 7, -14.0f, 15.0f, -4.0f } } },
        { "Trap Vocals", "Dark Fifth Pad", ModeDegrees, HarmonicMinor, 2, 60.0f, 80.0f, {
            { true,   7,  4, -10.0f, -30.0f,  6.0f }, { true, -12, -7, -14.0f, 25.0f, -6.0f }, kOff, kOff } },

        // ---- Afrotrap ----
        { "Afrotrap", "Afro Pentatonic Lift", ModeDegrees, MajorPentatonic, 7, 25.0f, 100.0f, {
            { true,   4,  2,  -6.0f, -40.0f,  0.0f }, { true,   9,  4, -10.0f,  35.0f,  0.0f }, kOff, kOff } },
        { "Afrotrap", "Amapiano Sixths", ModeDegrees, Major, 5, 35.0f, 100.0f, {
            { true,   9,  5,  -6.0f, -30.0f,  3.0f }, { true,   4,  2,  -9.0f,  30.0f, -3.0f }, kOff, kOff } },
        { "Afrotrap", "Afrobeat Silk Harmony", ModeDegrees, Mixolydian, 2, 30.0f, 90.0f, {
            { true,   4,  2,  -7.0f, -35.0f,  0.0f }, { true,   7,  4,  -9.0f,  35.0f,  5.0f }, kOff, kOff } },

        // ---- Oriental / Rai ----
        { "Oriental / Rai", "Oriental Hijaz Stack", ModeDegrees, Hijaz, 4, 25.0f, 100.0f, {
            { true,   4,  2,  -6.0f, -35.0f,  0.0f }, { true,   7,  4,  -8.0f,  35.0f,  0.0f }, kOff, kOff } },
        { "Oriental / Rai", "Rai Bayati Choir", ModeDegrees, Bayati, 2, 40.0f, 100.0f, {
            { true,   3,  2,  -6.0f, -40.0f, -8.0f }, { true,  -4, -2,  -8.0f,  40.0f,  8.0f },
            { true,   7,  4, -12.0f,   0.0f,  0.0f }, kOff } },
        { "Oriental / Rai", "Maqam Rast Pair", ModeDegrees, Rast, 0, 25.0f, 85.0f, {
            { true,   4,  2,  -5.0f, -30.0f,  0.0f }, { true,  -7, -4,  -8.0f,  30.0f,  0.0f }, kOff, kOff } },
        { "Oriental / Rai", "Saba Shade", ModeDegrees, Saba, 9, 50.0f, 70.0f, {
            { true,   3,  2,  -9.0f, -25.0f,  0.0f }, { true, -10, -6, -12.0f,  25.0f, -10.0f }, kOff, kOff } },

        // ---- Deep House ----
        { "Deep House", "Deep House Haze", ModeDegrees, Minor, 0, 90.0f, 70.0f, {
            { true,   3,  2, -12.0f, -50.0f,  7.0f }, { true,   7,  4, -14.0f,  50.0f, -7.0f }, kOff, kOff } },
        { "Deep House", "Dorian Chord Pad", ModeDegrees, Dorian, 7, 120.0f, 85.0f, {
            { true,   3,  2, -10.0f, -60.0f,  0.0f }, { true,   7,  4, -12.0f,   0.0f,  0.0f },
            { true,  10,  6, -14.0f,  60.0f,  0.0f }, kOff } },

        // ---- G-Funk ----
        { "G-Funk", "G-Funk Synth Talk", ModeDegrees, MinorPentatonic, 7, 15.0f, 100.0f, {
            { true,   5,  2,  -8.0f, -45.0f,  0.0f }, { true,   7,  3, -10.0f,  40.0f,  0.0f }, kOff, kOff } },
        { "G-Funk", "Blues Octave Talk", ModeDegrees, Blues, 4, 20.0f, 100.0f, {
            kOff, { true, -12, -6, -10.0f, -35.0f,  0.0f }, { true, 12, 6, -16.0f, 35.0f, 0.0f }, kOff } },

        // ---- R&B ----
        { "R&B", "R&B Silk Thirds", ModeDegrees, Major, 5, 35.0f, 100.0f, {
            { true,   4,  2,  -5.0f, -30.0f,  4.0f }, { true,   9,  5,  -9.0f,  30.0f, -4.0f }, kOff, kOff } },
        { "R&B", "Neo Soul Stack", ModeDegrees, Dorian, 9, 40.0f, 100.0f, {
            { true,   3,  2,  -7.0f, -45.0f,  0.0f }, { true,   7,  4,  -9.0f,   0.0f,  0.0f },
            { true,  10,  6, -11.0f,  45.0f,  0.0f }, kOff } },
        { "R&B", "Fem Soul Doubles", ModeSemitones, Major, 0, 22.0f, 100.0f, {
            { true,   0,  0,  -8.0f, -40.0f, 12.0f }, { true,   0,  0,  -8.0f,  40.0f, -12.0f }, kOff, kOff } },

        // ---- Spoken Word / Podcast ----
        { "Spoken Word / Podcast", "Podcast Warm Thirds", ModeDegrees, Major, 0, 60.0f, 30.0f, {
            { true,   4,  2, -12.0f, -20.0f,  0.0f }, kOff, kOff, kOff } },
        { "Spoken Word / Podcast", "Narration Width", ModeSemitones, Chromatic, 0, 80.0f, 35.0f, {
            { true,   0,  0, -14.0f, -60.0f, 10.0f }, { true,   0,  0, -14.0f,  60.0f, -10.0f }, kOff, kOff } },
        { "Spoken Word / Podcast", "Narrator Sub Warmth", ModeDegrees, Minor, 4, 100.0f, 25.0f, {
            kOff, { true, -12, -7, -16.0f, -20.0f,  3.0f }, kOff, kOff } },
    };
}

std::vector<HumpirePreset> makeSaturnFactoryPresets()
{
    std::vector<HumpirePreset> out;
    out.reserve (sizeof (kList) / sizeof (kList[0]));

    for (const auto& p : kList)
    {
        HumpirePreset preset;
        preset.category = p.category;
        preset.name = p.name;

        auto add = [&preset] (juce::String paramId, float value) { preset.values.emplace_back (std::move (paramId), value); };
        add (id::mode, (float) p.mode);
        add (id::key, (float) p.key);
        add (id::scale, (float) p.scale);
        add (id::glide, p.glide);
        add (id::mix, p.mix);

        for (int v = 0; v < kNumVoices; ++v)
        {
            const VoiceP& vp = p.voices[v];
            add (voiceId (v, vOn), vp.on ? 1.0f : 0.0f);
            add (voiceId (v, vSemi), (float) vp.semi);
            add (voiceId (v, vDeg), (float) vp.deg);
            add (voiceId (v, vLevel), vp.level);
            add (voiceId (v, vPan), vp.pan);
            add (voiceId (v, vDetune), vp.detune);
        }
        out.push_back (std::move (preset));
    }
    return out;
}
