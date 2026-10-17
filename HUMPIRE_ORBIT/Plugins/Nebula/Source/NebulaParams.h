#pragma once
#include <memory>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE NEBULA - parameter identifiers, ranges and APVTS layout (voice multiplier 1 to 32). */
namespace nebula
{
    namespace id
    {
        inline constexpr const char* voices      = "voices";
        inline constexpr const char* detune      = "detune";
        inline constexpr const char* delay       = "delay";
        inline constexpr const char* delaySpread = "delaySpread";
        inline constexpr const char* spread      = "spread";
        inline constexpr const char* modRate     = "modRate";
        inline constexpr const char* modDepth    = "modDepth";
        inline constexpr const char* tone        = "tone";
        inline constexpr const char* mix         = "mix";
        inline constexpr const char* inGain      = "inGain";
        inline constexpr const char* outGain     = "outGain";
        inline constexpr const char* bypass      = "bypass";
        inline constexpr const char* simple      = "simple";
    }

    /** Smallest and largest number of voices. */
    inline constexpr int minVoices = 1;
    inline constexpr int maxVoices = 32;

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto txt = [] (const char* unit, int dec)
        {
            return [unit, dec] (float v, int) { return String (v, dec) + " " + unit; };
        };
        auto pct = [] (float v, int) { return String (roundToInt (v)) + " %"; };

        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id::voices, 1 }, "Voices",
            minVoices, maxVoices, 8,
            AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int)
            {
                return String (v) + (v == 1 ? " voice" : " voices");
            })));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::detune, 1 }, "Detune",
            NormalisableRange<float> (0.0f, 50.0f, 0.01f), 12.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ct", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::delay, 1 }, "Delay",
            NormalisableRange<float> (2.0f, 40.0f, 0.01f), 14.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::delaySpread, 1 }, "Delay Spread",
            NormalisableRange<float> (0.0f, 30.0f, 0.01f), 4.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("ms", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::spread, 1 }, "Spread",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 80.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::modRate, 1 }, "Mod Rate",
            NormalisableRange<float> (0.05f, 6.0f, 0.001f, 0.45f), 0.45f,
            AudioParameterFloatAttributes().withStringFromValueFunction (txt ("Hz", 2))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::modDepth, 1 }, "Mod Depth",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (pct)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::tone, 1 }, "Tone",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 70.0f,
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

        return { p.begin(), p.end() };
    }
}
