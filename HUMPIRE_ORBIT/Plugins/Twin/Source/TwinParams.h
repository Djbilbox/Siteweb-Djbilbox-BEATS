#pragma once
#include <array>
#include <atomic>
#include <memory>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

/** HUMPIRE TWIN - parameter identifiers, ranges, live UI state and the APVTS layout. */
namespace twin
{
    /** Maximum number of doubles generated for one vocal. */
    constexpr int kMaxVoices = 4;

    namespace id
    {
        inline constexpr const char* doubles  = "doubles";
        inline constexpr const char* detune   = "detune";
        inline constexpr const char* delay    = "delay";
        inline constexpr const char* humanize = "humanize";
        inline constexpr const char* drift    = "drift";
        inline constexpr const char* width    = "width";
        inline constexpr const char* hpFreq   = "hpFreq";
        inline constexpr const char* lpFreq   = "lpFreq";
        inline constexpr const char* mix      = "mix";
        inline constexpr const char* outGain  = "outGain";
        inline constexpr const char* bypass   = "bypass";
        inline constexpr const char* simple   = "simple";
    }

    /** Live values published by the audio thread for the UI (lock-free, read at ~60 Hz). */
    struct UiState
    {
        std::atomic<int> voices { 0 };
        std::array<std::atomic<float>, kMaxVoices> delayMs {}, detuneCt {}, pan {};
    };

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        auto unit = [] (const char* u, int decimals)
        {
            return [u, decimals] (float v, int) { return String (v, decimals) + " " + u; };
        };
        auto percent = [] (float v, int) { return String (roundToInt (v)) + " %"; };

        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id::doubles, 1 }, "Doubles", 1, kMaxVoices, 2));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::detune, 1 }, "Detune",
            NormalisableRange<float> (0.0f, 50.0f, 0.1f), 12.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("ct", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::delay, 1 }, "Delay",
            NormalisableRange<float> (5.0f, 40.0f, 0.1f), 18.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("ms", 1))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::humanize, 1 }, "Humanize",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 40.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::drift, 1 }, "Drift",
            NormalisableRange<float> (0.05f, 1.5f, 0.01f, 0.5f), 0.35f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("Hz", 2))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::width, 1 }, "Width",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 80.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::hpFreq, 1 }, "High-Pass",
            NormalisableRange<float> (20.0f, 800.0f, 1.0f, 0.4f), 120.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("Hz", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::lpFreq, 1 }, "Low-Pass",
            NormalisableRange<float> (1500.0f, 20000.0f, 1.0f, 0.4f), 12000.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("Hz", 0))));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 40.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::outGain, 1 }, "Output",
            NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f,
            AudioParameterFloatAttributes().withStringFromValueFunction (unit ("dB", 1))));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::bypass, 1 }, "Bypass", false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::simple, 1 }, "Simple", false,
            AudioParameterBoolAttributes().withAutomatable (false)));

        return { p.begin(), p.end() };
    }
}
