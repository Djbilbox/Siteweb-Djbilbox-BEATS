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
    constexpr float kMinLevelDb = -60.0f;           // voice level at its minimum = silence ("-inf dB")

    namespace id
    {
        inline constexpr const char* mode = "mode", *key = "key", *scale = "scale", *glide = "glide",
            *mix = "mix", *inGain = "inGain", *outGain = "outGain", *bypass = "bypass", *simple = "simple";
    }

    /** Per-voice parameters: "v1On", "v1Semi", "v1Deg", "v1Level", "v1Pan", "v1Detune", "v2On", ...
        Builds a String: use it at construction time only, never on the audio thread. */
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
    /** Text conversion helpers of the parameters (message thread only: they build Strings). */
    namespace text
    {
        /** First number found in a typed text ("+4 st", "-12.5 dB", "35 %", "L 35"); `fallback` if there is none.
            A comma is accepted as the decimal separator. */
        inline float parseNumber (const juce::String& s, float fallback)
        {
            const auto kept = s.replaceCharacter (',', '.').retainCharacters ("0123456789.+-");
            if (! kept.containsAnyOf ("0123456789")) return fallback;
            return kept.getFloatValue();
        }

        /** "12.5 dB". Never prints a negative zero. Zero decimals really means an integer: juce::String (v, 0)
            would fall back to the general format and may print many decimals. */
        inline juce::String withUnit (float v, int decimals, const char* unit, bool plusSign = false)
        {
            if (std::abs (v) < 0.5f * std::pow (10.0f, (float) -decimals)) v = 0.0f;
            const juce::String number = decimals > 0 ? juce::String (v, decimals) : juce::String (juce::roundToInt (v));
            return juce::String ((plusSign && v > 0.0f) ? "+" : "") + number + " " + unit;
        }

        inline juce::String percent (float v)   { return juce::String (juce::roundToInt (v)) + " %"; }

        inline juce::String levelDb (float v)
        {
            return v <= kMinLevelDb + 0.05f ? juce::String ("-inf dB") : withUnit (v, 1, "dB");
        }

        inline float levelFromText (const juce::String& s)
        {
            if (s.containsIgnoreCase ("inf")) return kMinLevelDb;
            return parseNumber (s, 0.0f);
        }

        inline juce::String pan (float v)
        {
            const int i = juce::roundToInt (v);
            if (i == 0) return "C";
            return i < 0 ? "L " + juce::String (-i) : "R " + juce::String (i);
        }

        /** "L 35" / "R 20" / "C" / "-35" / "20". */
        inline float panFromText (const juce::String& s)
        {
            const auto t = s.trim();
            if (t.startsWithIgnoreCase ("c")) return 0.0f;
            const float n = std::abs (parseNumber (t, 0.0f));
            if (t.startsWithIgnoreCase ("l")) return -n;
            if (t.startsWithIgnoreCase ("r")) return n;
            return parseNumber (t, 0.0f);
        }

        inline bool boolFromText (const juce::String& s)
        {
            const auto t = s.trim().toLowerCase();
            return t == "on" || t == "1" || t == "true" || t == "yes";
        }
    }

    //==========================================================================
    struct VoiceDefaults { bool on; int semi, deg; float level, pan; };

    inline constexpr VoiceDefaults kVoiceDefaults[kNumVoices] = {
        {  true,   4,   2,  -6.0f, -35.0f },     // third
        {  true,   7,   4,  -6.0f,  35.0f },     // fifth
        { false, -12,  -7,  -9.0f, -20.0f },     // octave below
        { false,  12,   7,  -9.0f,  20.0f },     // octave above
    };

    /** Every float and int parameter has both a value-to-text and a text-to-value function. */
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto floatAttr = [] (std::function<String (float, int)> toText, std::function<float (const String&)> fromText)
        {
            return AudioParameterFloatAttributes().withStringFromValueFunction (std::move (toText))
                                                  .withValueFromStringFunction (std::move (fromText));
        };
        auto unitAttr = [floatAttr] (const char* unit, int decimals, bool plusSign = false)
        {
            return floatAttr ([unit, decimals, plusSign] (float v, int) { return text::withUnit (v, decimals, unit, plusSign); },
                              [] (const String& s) { return text::parseNumber (s, 0.0f); });
        };
        auto boolAttr = []
        {
            return AudioParameterBoolAttributes()
                       .withStringFromValueFunction ([] (bool v, int) -> String { return v ? "On" : "Off"; })
                       .withValueFromStringFunction ([] (const String& s) { return text::boolFromText (s); });
        };

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::mode, 1 }, "Interval Mode",
                        StringArray { "Scale Degrees", "Semitones" }, ModeDegrees));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::key, 1 }, "Key",
                        StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));
        StringArray scales;
        for (auto& s : kScales) scales.add (s.name);
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::scale, 1 }, "Scale", scales, Major));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::glide, 1 }, "Glide",
            NormalisableRange<float> (0.0f, 300.0f, 1.0f, 0.5f), 30.0f, unitAttr ("ms", 0)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f,
            floatAttr ([] (float v, int) { return text::percent (v); },
                       [] (const String& s) { return text::parseNumber (s, 100.0f); })));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::inGain, 1 }, "Input",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, unitAttr ("dB", 1)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::outGain, 1 }, "Output",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, unitAttr ("dB", 1)));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::bypass, 1 }, "Bypass", false, boolAttr()));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::simple, 1 }, "Simple View", false,
            boolAttr().withAutomatable (false)));

        auto intAttr = [] (const char* unit)
        {
            return AudioParameterIntAttributes()
                .withStringFromValueFunction ([unit] (int v, int) -> String { return String (v > 0 ? "+" : "") + String (v) + " " + unit; })
                .withValueFromStringFunction ([] (const String& s) { return juce::roundToInt (text::parseNumber (s, 0.0f)); });
        };

        for (int v = 0; v < kNumVoices; ++v)
        {
            const auto& d = kVoiceDefaults[v];
            const String n = "Voice " + String (v + 1) + " ";
            p.push_back (std::make_unique<AudioParameterBool> (ParameterID { voiceId (v, vOn), 1 }, n + "On", d.on, boolAttr()));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vSemi), 1 }, n + "Semitones",
                            -24, 24, d.semi, intAttr ("st")));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vDeg), 1 }, n + "Degrees",
                            -14, 14, d.deg, intAttr ("deg")));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vLevel), 1 }, n + "Level",
                NormalisableRange<float> (kMinLevelDb, 6.0f, 0.1f), d.level,
                floatAttr ([] (float x, int) { return text::levelDb (x); },
                           [] (const String& s) { return text::levelFromText (s); })));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vPan), 1 }, n + "Pan",
                NormalisableRange<float> (-100.0f, 100.0f, 1.0f), d.pan,
                floatAttr ([] (float x, int) { return text::pan (x); },
                           [] (const String& s) { return text::panFromText (s); })));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vDetune), 1 }, n + "Detune",
                NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f, unitAttr ("ct", 1, true)));
        }
        return { p.begin(), p.end() };
    }
}
