#pragma once
#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE SOLAR - identifiants de parametres, gammes (en cents, microtonales incluses) et layout APVTS. */
namespace solar
{
    namespace id
    {
        inline constexpr const char* mode = "mode", *key = "key", *scale = "scale", *retune = "retune",
            *flex = "flex", *humanize = "humanize", *amount = "amount", *refA = "refA",
            *vibDepth = "vibDepth", *vibRate = "vibRate", *vibDelay = "vibDelay",
            *portamento = "portamento", *mix = "mix", *inGain = "inGain", *outGain = "outGain",
            *bypass = "bypass", *simple = "simple";
    }
    inline juce::String noteId (int i) { return "note" + juce::String (i); }

    enum Scale { Chromatic, Major, Minor, HarmMinor, PentMajor, PentMinor, Blues, Dorian,
                 Hijaz, HijazKar, Rast, Bayati, NumScales };

    struct ScaleDef { const char* name; int n; float cents[12]; };

    // Degres en cents depuis la tonique (octave exclue). Rast / Bayati = quarts de ton (maqams).
    inline constexpr ScaleDef kScales[NumScales] = {
        { "Chromatique",      12, { 0,100,200,300,400,500,600,700,800,900,1000,1100 } },
        { "Majeur",            7, { 0,200,400,500,700,900,1100 } },
        { "Mineur",            7, { 0,200,300,500,700,800,1000 } },
        { "Mineur harmonique", 7, { 0,200,300,500,700,800,1100 } },
        { "Pentatonique maj.", 5, { 0,200,400,700,900 } },
        { "Pentatonique min.", 5, { 0,300,500,700,1000 } },
        { "Blues",             6, { 0,300,500,600,700,1000 } },
        { "Dorien",            7, { 0,200,300,500,700,900,1000 } },
        { "Hijaz",             7, { 0,100,400,500,700,800,1000 } },
        { "Hijaz Kar",         7, { 0,100,400,500,700,800,1100 } },
        { "Maqam Rast",        7, { 0,200,350,500,700,900,1050 } },
        { "Maqam Bayati",      7, { 0,150,300,500,700,800,1000 } },
    };

    inline const char* noteName (int pc)
    {
        static const char* n[12] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
        return n[((pc % 12) + 12) % 12];
    }

    //==========================================================================
    /** Degres autorises d'une gamme transposee, en demi-tons depuis la tonique ; d[n] = d[0] + 12. */
    struct Degrees
    {
        float d[14] {};
        int n = 0;
    };

    /** key = tonique (0..11), mask = bits des 12 notes autorisees (classe de hauteur absolue). */
    inline Degrees buildDegrees (int key, int scale, unsigned mask)
    {
        Degrees out;
        const auto& s = kScales[juce::jlimit (0, (int) NumScales - 1, scale)];
        for (int i = 0; i < s.n; ++i)
        {
            const int pc = (key + (int) std::lround (s.cents[i] / 100.0f)) % 12;
            if (mask & (1u << pc)) out.d[out.n++] = s.cents[i] * 0.01f;
        }
        if (out.n == 0)                       // tout desactive -> chromatique (securite)
            for (int i = 0; i < 12; ++i) out.d[out.n++] = (float) i;
        out.d[out.n] = out.d[0] + 12.0f;
        return out;
    }

    /** Notes autorisees encadrant `midi` (valeurs MIDI absolues). */
    inline void bracket (float midi, int key, const Degrees& deg, float& below, float& above)
    {
        const float rel = midi - (float) key;
        const float oct = std::floor (rel / 12.0f);
        const float x = rel - 12.0f * oct;                       // 0..12
        int i = 0;
        while (i < deg.n - 1 && deg.d[i + 1] <= x) ++i;
        below = (float) key + 12.0f * oct + deg.d[i];
        above = (float) key + 12.0f * oct + deg.d[i + 1];
    }

    inline float snapMidi (float midi, int key, const Degrees& deg)
    {
        float lo, hi;
        bracket (midi, key, deg, lo, hi);
        return (midi - lo <= hi - midi) ? lo : hi;
    }

    //==========================================================================
    constexpr int kGraphPoints = 96;   // 8 points par demi-ton, sur une octave (classe de hauteur 0..12)

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto txt = [] (const char* unit, int dec) {
            return [unit, dec] (float v, int) { return String (v, dec) + " " + unit; };
        };
        auto pct = [] (float v, int) { return String (roundToInt (v)) + " %"; };

        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::mode, 1 }, "Mode", StringArray { "Auto", "Graph" }, 0));
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::key, 1 }, "Tonalite",
                        StringArray { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" }, 0));
        StringArray scales;
        for (auto& s : kScales) scales.add (s.name);
        p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id::scale, 1 }, "Gamme", scales, Minor));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::retune, 1 }, "Retune",
            NormalisableRange<float> (0.0f, 400.0f, 0.1f, 0.4f), 25.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::flex, 1 }, "Flex-Tune",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ct", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::humanize, 1 }, "Humanize",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 20.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::amount, 1 }, "Amount",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::refA, 1 }, "Ref A",
            NormalisableRange<float> (415.0f, 465.0f, 0.1f), 440.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("Hz", 1))));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::vibDepth, 1 }, "Vib Depth",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ct", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::vibRate, 1 }, "Vib Rate",
            NormalisableRange<float> (0.5f, 9.0f, 0.01f), 5.5f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("Hz", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::vibDelay, 1 }, "Vib Delay",
            NormalisableRange<float> (0.0f, 1500.0f, 1.0f, 0.6f), 300.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::portamento, 1 }, "Portamento",
            NormalisableRange<float> (0.0f, 400.0f, 0.1f, 0.5f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 0))));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f,
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

        for (int i = 0; i < 12; ++i)
            p.push_back (std::make_unique<AudioParameterBool> (ParameterID { noteId (i).toRawUTF8(), 1 },
                                                               String ("Note ") + noteName (i), true));
        return { p.begin(), p.end() };
    }
}
