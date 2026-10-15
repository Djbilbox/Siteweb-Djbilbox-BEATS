#pragma once
#include <array>
#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE SATURN - identifiants de parametres, gammes (en cents, maqams microtonaux inclus),
    calcul des degres de gamme et layout APVTS. */
namespace saturn
{
    constexpr int kNumVoices = 4;

    namespace id
    {
        inline constexpr const char* mode = "mode", *key = "key", *scale = "scale", *formant = "formant",
            *glide = "glide", *correction = "correction", *humanize = "humanize", *width = "width",
            *mix = "mix", *inGain = "inGain", *outGain = "outGain", *bypass = "bypass", *simple = "simple";
    }

    /** Parametres propres a chaque voix : "v1On", "v1Semi", "v1Deg", "v1Level", "v1Pan", "v1Detune", "v1Delay"... */
    enum VoiceParam { vOn, vSemi, vDeg, vLevel, vPan, vDetune, vDelay, kNumVoiceParams };

    inline juce::String voiceId (int voice, VoiceParam p)
    {
        static const char* suffix[kNumVoiceParams] = { "On", "Semi", "Deg", "Level", "Pan", "Detune", "Delay" };
        return "v" + juce::String (voice + 1) + suffix[(int) p];
    }

    enum Mode { ModeSemitones = 0, ModeDegrees = 1 };

    enum Scale { Chromatic, Major, Minor, HarmMinor, PentMajor, PentMinor, Blues, Dorian, Mixolydian,
                 Hijaz, HijazKar, Rast, Bayati, Kurd, Saba, NumScales };

    struct ScaleDef { const char* name; int n; float cents[12]; };

    // Degres en cents depuis la tonique (octave exclue). Rast / Bayati / Saba = quarts de ton (maqams),
    // memes definitions que HUMPIRE SOLAR + Mixolydien, Kurd et Saba.
    inline constexpr ScaleDef kScales[NumScales] = {
        { "Chromatique",      12, { 0,100,200,300,400,500,600,700,800,900,1000,1100 } },
        { "Majeur",            7, { 0,200,400,500,700,900,1100 } },
        { "Mineur",            7, { 0,200,300,500,700,800,1000 } },
        { "Mineur harmonique", 7, { 0,200,300,500,700,800,1100 } },
        { "Pentatonique maj.", 5, { 0,200,400,700,900 } },
        { "Pentatonique min.", 5, { 0,300,500,700,1000 } },
        { "Blues",             6, { 0,300,500,600,700,1000 } },
        { "Dorien",            7, { 0,200,300,500,700,900,1000 } },
        { "Mixolydien",        7, { 0,200,400,500,700,900,1000 } },
        { "Hijaz",             7, { 0,100,400,500,700,800,1000 } },
        { "Hijaz Kar",         7, { 0,100,400,500,700,800,1100 } },
        { "Maqam Rast",        7, { 0,200,350,500,700,900,1050 } },
        { "Maqam Bayati",      7, { 0,150,300,500,700,800,1000 } },
        { "Maqam Kurd",        7, { 0,100,300,500,700,800,1000 } },
        { "Maqam Saba",        7, { 0,150,300,400,700,800,1000 } },
    };

    inline const char* noteName (int pc)
    {
        static const char* n[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
        return n[((pc % 12) + 12) % 12];
    }

    //==========================================================================
    /** Degres d'une gamme en demi-tons (flottants : quarts de ton) depuis la tonique ; d[n] = 12 (octave). */
    struct Degrees
    {
        float d[13] {};
        int n = 0;
    };

    inline Degrees buildDegrees (int scale) noexcept
    {
        Degrees out;
        const auto& s = kScales[juce::jlimit (0, (int) NumScales - 1, scale)];
        for (int i = 0; i < s.n; ++i) out.d[out.n++] = s.cents[i] * 0.01f;
        out.d[out.n] = 12.0f;
        return out;
    }

    /** Division / modulo "mathematiques" (vers -infini) pour les index de degres negatifs. */
    inline int floorDiv (int a, int b) noexcept { return (a >= 0) ? a / b : -((-a + b - 1) / b); }
    inline int floorMod (int a, int b) noexcept { return a - b * floorDiv (a, b); }

    /** Index absolu de degre -> note MIDI (flottante). Index 0 = tonique a l'octave MIDI 0 (midi = key). */
    inline float degreeToMidi (int key, const Degrees& deg, int index) noexcept
    {
        const int oct = floorDiv (index, deg.n);
        return (float) key + 12.0f * (float) oct + deg.d[floorMod (index, deg.n)];
    }

    /** Index absolu du degre de gamme le plus proche de `midi`. */
    inline int nearestDegree (float midi, int key, const Degrees& deg) noexcept
    {
        const float rel = midi - (float) key;
        const float octF = std::floor (rel / 12.0f);
        const float x = rel - 12.0f * octF;                 // 0..12
        int i = 0;
        while (i < deg.n - 1 && deg.d[i + 1] <= x) ++i;
        const int oct = (int) octF;
        const bool up = (deg.d[i + 1] - x) < (x - deg.d[i]);
        return oct * deg.n + i + (up ? 1 : 0);
    }

    //==========================================================================
    struct VoiceDefaults { bool on; int semi, deg; float level, pan; };
    inline constexpr VoiceDefaults kVoiceDefaults[kNumVoices] = {
        { true,   4,  2, -6.0f, -35.0f },     // tierce
        { true,   7,  4, -6.0f,  35.0f },     // quinte
        { false, -12, -7, -9.0f,   0.0f },    // octave basse
        { false,  12,  7, -9.0f,   0.0f },    // octave haute
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto txt = [] (const char* unit, int dec) {
            return [unit, dec] (float v, int) { return String (v, dec) + " " + unit; };
        };
        auto pct = [] (float v, int) { return String (roundToInt (v)) + " %"; };

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::mode, 1 }, "Intervalles",
                        StringArray { "Demi-tons", "Degres gamme" }, ModeDegrees));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::key, 1 }, "Tonalite",
                        StringArray { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 0));
        StringArray scales;
        for (auto& s : kScales) scales.add (s.name);
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::scale, 1 }, "Gamme", scales, Major));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::formant, 1 }, "Formant", true));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::glide, 1 }, "Glide",
            NormalisableRange<float> (0.0f, 300.0f, 0.1f, 0.5f), 25.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::correction, 1 }, "Correction",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::humanize, 1 }, "Humanize",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::width, 1 }, "Largeur",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::inGain, 1 }, "Input",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("dB", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::outGain, 1 }, "Output",
            NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("dB", 1))));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::bypass, 1 }, "Bypass", false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::simple, 1 }, "Simple", false,
            AudioParameterBoolAttributes().withAutomatable (false)));

        auto semiTxt = [] (int v, int) { return (v > 0 ? "+" : "") + String (v) + " dt"; };
        auto degTxt  = [] (int v, int) { return (v > 0 ? "+" : "") + String (v) + " deg"; };
        auto dbTxt   = [] (float v, int) { return v <= -59.9f ? String ("-inf dB") : String (v, 1) + " dB"; };
        auto panTxt  = [] (float v, int) {
            const int i = roundToInt (v);
            return i == 0 ? String ("C") : (i < 0 ? String (-i) + " G" : String (i) + " D");
        };
        auto ctTxt   = [] (float v, int) { return (v > 0.05f ? "+" : "") + String (v, 1) + " ct"; };

        for (int v = 0; v < kNumVoices; ++v)
        {
            const auto& d = kVoiceDefaults[v];
            const String n = "Voix " + String (v + 1) + " ";
            p.push_back (std::make_unique<AudioParameterBool> (ParameterID { voiceId (v, vOn), 1 }, n + "On", d.on));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vSemi), 1 }, n + "Demi-tons",
                            -24, 24, d.semi, AudioParameterIntAttributes().withStringFromValueFunction (semiTxt)));
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { voiceId (v, vDeg), 1 }, n + "Degres",
                            -14, 14, d.deg, AudioParameterIntAttributes().withStringFromValueFunction (degTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vLevel), 1 }, n + "Niveau",
                NormalisableRange<float> (-60.0f, 6.0f, 0.1f, 2.2f), d.level,
                AudioParameterFloatAttributes().withStringFromValueFunction (dbTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vPan), 1 }, n + "Pan",
                NormalisableRange<float> (-100.0f, 100.0f, 0.1f), d.pan,
                AudioParameterFloatAttributes().withStringFromValueFunction (panTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vDetune), 1 }, n + "Detune",
                NormalisableRange<float> (-50.0f, 50.0f, 0.1f), 0.0f,
                AudioParameterFloatAttributes().withStringFromValueFunction (ctTxt)));
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { voiceId (v, vDelay), 1 }, n + "Delai",
                NormalisableRange<float> (0.0f, 60.0f, 0.1f, 0.6f), 0.0f,
                AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 1))));
        }
        return { p.begin(), p.end() };
    }
}
