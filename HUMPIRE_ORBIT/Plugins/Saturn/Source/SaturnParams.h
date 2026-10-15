#pragma once
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE SATURN - parameter identifiers, scales (in cents, Oriental maqams included),
    scale-degree helpers, per-voice parameters and the APVTS layout. */
namespace saturn
{
    constexpr double kPi = 3.14159265358979323846;
    constexpr int kNumVoices = 4;
    constexpr float kMaxShiftSemitones = 24.0f;

    namespace id
    {
        inline constexpr const char* mode = "mode", *key = "key", *scale = "scale", *glide = "glide",
            *mix = "mix", *inGain = "inGain", *outGain = "outGain", *bypass = "bypass", *simple = "simple";
    }

    /** Per-voice parameters: "v1On", "v1Semi", "v1Deg", "v1Level", "v1Pan", "v1Detune", "v2On", ... */
    enum VoiceParam { vOn = 0, vSemi, vDeg, vLevel, vPan, vDetune, kNumVoiceParams };

    inline juce::String voiceId (int voice, VoiceParam p)
    {
        static const char* const suffix[kNumVoiceParams] = { "On", "Semi", "Deg", "Level", "Pan", "Detune" };
        return "v" + juce::String (voice + 1) + juce::String (suffix[(int) p]);
    }

    /** Interval mode: intervals follow the scale degrees of the detected note, or fixed semitones. */
    enum IntervalMode { ModeDegrees = 0, ModeSemitones = 1 };

    /** Scales. Degrees are in cents above the tonic (octave excluded).
        Rast, Bayati, Kurd and Saba are microtonal maqams (quarter tones included). */
    enum Scale { Chromatic, Major, Minor, HarmonicMinor, MajorPentatonic, MinorPentatonic, Blues, Dorian,
                 Mixolydian, Hijaz, HijazKar, Rast, Bayati, Kurd, Saba, NumScales };

    struct ScaleDef { const char* name; int n; float cents[12]; };

    inline constexpr ScaleDef kScales[NumScales] = {
        { "Chromatic",         12, { 0,100,200,300,400,500,600,700,800,900,1000,1100 } },
        { "Major",              7, { 0,200,400,500,700,900,1100 } },
        { "Minor",              7, { 0,200,300,500,700,800,1000 } },
        { "Harmonic Minor",     7, { 0,200,300,500,700,800,1100 } },
        { "Major Pentatonic",   5, { 0,200,400,700,900 } },
        { "Minor Pentatonic",   5, { 0,300,500,700,1000 } },
        { "Blues",              6, { 0,300,500,600,700,1000 } },
        { "Dorian",             7, { 0,200,300,500,700,900,1000 } },
        { "Mixolydian",         7, { 0,200,400,500,700,900,1000 } },
        { "Hijaz",              7, { 0,100,400,500,700,800,1000 } },
        { "Hijaz Kar",          7, { 0,100,400,500,700,800,1100 } },
        { "Maqam Rast",         7, { 0,200,350,500,700,900,1050 } },
        { "Maqam Bayati",       7, { 0,150,300,500,700,800,1000 } },
        { "Maqam Kurd",         7, { 0,100,300,500,700,800,1000 } },
        { "Maqam Saba",         7, { 0,150,300,400,700,800,1000 } },
    };

    inline const char* noteName (int pitchClass) noexcept
    {
        static const char* const names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return names[((pitchClass % 12) + 12) % 12];
    }

    //==========================================================================
    /** Degrees of a scale in semitones (floats: quarter tones) above the tonic; d[n] = 12 (the octave). */
    struct Degrees
    {
        float d[14] {};
        int n = 0;
    };

    inline Degrees buildDegrees (int scale) noexcept
    {
        Degrees out;
        const auto& s = kScales[std::max (0, std::min ((int) NumScales - 1, scale))];
        for (int i = 0; i < s.n; ++i) out.d[out.n++] = 0.01f * s.cents[i];
        out.d[out.n] = 12.0f;
        return out;
    }

    /** Floor division and modulo (towards minus infinity) for negative degree indices. b must be > 0. */
    inline int floorDiv (int a, int b) noexcept { return a >= 0 ? a / b : -((-a + b - 1) / b); }
    inline int floorMod (int a, int b) noexcept { return a - b * floorDiv (a, b); }

    /** Absolute degree index -> MIDI-like pitch (float). Index 0 = tonic in octave 0 (midi = key). */
    inline float degreeToMidi (int key, const Degrees& deg, int index) noexcept
    {
        const int oct = floorDiv (index, deg.n);
        return (float) key + 12.0f * (float) oct + deg.d[floorMod (index, deg.n)];
    }

    /** Absolute index of the scale degree closest to `midi`. */
    inline int nearestDegree (float midi, int key, const Degrees& deg) noexcept
    {
        const float rel = midi - (float) key;
        const float octF = std::floor (rel / 12.0f);
        const float x = rel - 12.0f * octF;                  // 0..12 inside the octave
        int i = 0;
        while (i < deg.n - 1 && deg.d[i + 1] <= x) ++i;
        const bool up = (deg.d[i + 1] - x) < (x - deg.d[i]);
        return (int) octF * deg.n + i + (up ? 1 : 0);
    }

    //==========================================================================
    struct VoiceDefaults { bool on; int semi, deg; float level, pan; };

    inline constexpr VoiceDefaults kVoiceDefaults[kNumVoices] = {
        {  true,   4,   2,  -6.0f, -35.0f },     // third
        {  true,   7,   4,  -6.0f,  35.0f },     // fifth
        { false, -12,  -7,  -9.0f, -20.0f },     // octave below
        { false,  12,   7,  -9.0f,  20.0f },     // octave above
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto unitTxt = [] (const char* unit, int decimals) {
            return [unit, decimals] (float v, int) { return String (v, decimals) + " " + String (unit); };
        };
        auto pctTxt = [] (float v, int) { return String (roundToInt (v)) + " %"; };
        auto dbTxt = [] (float v, int) -> String { return v <= -59.9f ? String ("-inf dB") : String (v, 1) + " dB"; };

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::mode, 1 }, "Interval Mode",
                        StringArray { "Scale Degrees", "Semitones" }, ModeDegrees));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::key, 1 }, "Key",
                        StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
        StringArray scales;
        for (auto& s : kScales) scales.add (s.name);
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::scale, 1 }, "Scale", scales, Major));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::glide, 1 }, "Glide",
            NormalisableRange<float> (0.0f, 300.0f, 0.1f, 0.5f), 30.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unitTxt ("ms", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pctTxt)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::inGain, 1 }, "Input",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unitTxt ("dB", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::outGain, 1 }, "Output",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unitTxt ("dB", 1))));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::bypass, 1 }, "Bypass", false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::simple, 1 }, "Simple", false,
            AudioParameterBoolAttributes().withAutomatable (false)));

        auto semiTxt = [] (int v, int) -> String { return String (v > 0 ? "+" : "") + String (v) + " st"; };
        auto degTxt  = [] (int v, int) -> String { return String (v > 0 ? "+" : "") + String (v) + " deg"; };
        auto ctTxt   = [] (float v, int) -> String { return String (v > 0.05f ? "+" : "") + String (v, 1) + " ct"; };
        auto panTxt  = [] (float v, int) -> String {
            const int i = roundToInt (v);
            if (i == 0) return String ("C");
            return i < 0 ? "L " + String (-i) : "R " + String (i);
        };

        for (int v = 0; v < kNumVoices; ++v)
        {
            const auto& d = kVoiceDefaults[v];
            const String n = "Voice " + String (v + 1) + " ";
            p.push_back (std::make_unique<AudioParameterBool> (ParameterID { voiceId (v, vOn), 1 }, n + "On", d.on));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vSemi), 1 }, n + "Semitones",
                            -24, 24, d.semi, AudioParameterIntAttributes().withStringFromValueFunction (semiTxt)));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vDeg), 1 }, n + "Degrees",
                            -14, 14, d.deg, AudioParameterIntAttributes().withStringFromValueFunction (degTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vLevel), 1 }, n + "Level",
                NormalisableRange<float> (-60.0f, 6.0f, 0.1f), d.level,
                AudioParameterFloatAttributes().withStringFromValueFunction (dbTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vPan), 1 }, n + "Pan",
                NormalisableRange<float> (-100.0f, 100.0f, 0.1f), d.pan,
                AudioParameterFloatAttributes().withStringFromValueFunction (panTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vDetune), 1 }, n + "Detune",
                NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withStringFromValueFunction (ctTxt)));
        }
        return { p.begin(), p.end() };
    }
}
