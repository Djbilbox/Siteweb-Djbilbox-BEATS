#pragma once
#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE TWIN - identifiants de parametres et layout APVTS de l'auto-doubler vocal. */
namespace twin
{
    namespace id
    {
        inline constexpr const char* voices = "voices", *detune = "detune", *delay = "delay", *spread = "spread",
            *humanize = "humanize", *drift = "drift", *width = "width", *hpf = "hpf", *lpf = "lpf",
            *mix = "mix", *inGain = "inGain", *outGain = "outGain", *bypass = "bypass", *simple = "simple";
    }

    constexpr int kMaxVoices = 4;

    /** Affichage d'une frequence : "120 Hz" / "12.5 kHz". */
    inline juce::String hzText (float v)
    {
        return v >= 1000.0f ? juce::String (v * 0.001f, 1) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz";
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto txt = [] (const char* unit, int dec) {
            return [unit, dec] (float v, int) { return String (v, dec) + " " + unit; };
        };
        auto pct = [] (float v, int) { return String (roundToInt (v)) + " %"; };
        auto hz  = [] (float v, int) { return hzText (v); };

        // Nombre de doublures (1 a 4)
        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id::voices, 1 }, "Doublures", 1, kMaxVoices, 2,
            AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int) { return String (v) + (v > 1 ? " doublures" : " doublure"); })));

        // Micro-detune (cents) : doublure 1 = +detune, doublure 2 = -detune, 3/4 = +/-60 %
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::detune, 1 }, "Detune",
            NormalisableRange<float> (0.0f, 50.0f, 0.1f, 0.6f), 12.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ct", 1))));

        // Micro-delai de base (5-40 ms)
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::delay, 1 }, "Delay",
            NormalisableRange<float> (5.0f, 40.0f, 0.1f), 18.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 1))));

        // Ecart de delai entre doublures
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::spread, 1 }, "Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        // Humanisation : modulation lente et aleatoire du delai, du detune et du niveau
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::humanize, 1 }, "Humanize",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 40.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        // Vitesse de derive de l'humanisation
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::drift, 1 }, "Drift",
            NormalisableRange<float> (0.05f, 1.5f, 0.01f, 0.5f), 0.35f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("Hz", 2))));

        // Largeur stereo des doublures
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::width, 1 }, "Width",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 80.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));

        // Filtre des doublures (HP / LP, 12 dB/oct)
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::hpf, 1 }, "Low Cut",
            NormalisableRange<float> (20.0f, 1000.0f, 1.0f, 0.35f), 120.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (hz)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::lpf, 1 }, "High Cut",
            NormalisableRange<float> (1500.0f, 20000.0f, 1.0f, 0.4f), 14000.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (hz)));

        // Mix : 0 = voix seule, 50 = voix + doublures a l'unite, 100 = doublures seules
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
        return { p.begin(), p.end() };
    }
}
