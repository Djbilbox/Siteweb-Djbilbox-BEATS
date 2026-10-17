#pragma once
#include <array>
#include <atomic>
#include <cmath>
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

    /** Text <-> value helpers shared by the parameter layout. They run on the message / host UI
        thread only, never on the audio thread. */
    namespace text
    {
        /** Cuts a host display string to `maxLength` characters (0 = no limit). */
        inline juce::String fit (const juce::String& s, int maxLength)
        {
            return maxLength > 0 && s.length() > maxLength ? s.substring (0, maxLength) : s;
        }

        /** Number followed by a unit, e.g. "12.5 ms". Never prints "-0.0". */
        inline juce::String withUnit (float v, int decimals, const char* unit, int maxLength)
        {
            if (std::abs (v) < 0.5f * std::pow (10.0f, (float) -decimals)) v = 0.0f;
            return fit (juce::String (v, decimals) + " " + unit, maxLength);
        }

        /** Leading number of what the user typed ("12 ms", "-6 dB", "40 %", "1,5"). Anything
            unparsable gives 0. A 'k' (as in "1.5k" or "1.5 kHz") multiplies by 1000 when allowed. */
        inline float parse (const juce::String& typed, bool allowKilo)
        {
            const auto t = typed.trim().replaceCharacter (',', '.').toLowerCase();
            float v = t.getFloatValue();
            if (allowKilo && t.containsChar ('k')) v *= 1000.0f;
            return std::isfinite (v) ? v : 0.0f;
        }

        /** Frequency: "850 Hz" below 1 kHz, "1.50 kHz" above. */
        inline juce::String hz (float v, int maxLength)
        {
            if (v < 1000.0f) return fit (juce::String (juce::roundToInt (v)) + " Hz", maxLength);
            return fit (juce::String (v * 0.001f, v < 10000.0f ? 2 : 1) + " kHz", maxLength);
        }
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        using Float = AudioParameterFloatAttributes;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;

        // unit-suffixed number with a fixed number of decimals, e.g. "12.0 ct"
        auto unit = [] (const char* u, int decimals)
        {
            return Float().withStringFromValueFunction ([u, decimals] (float v, int maxLen) { return text::withUnit (v, decimals, u, maxLen); })
                          .withValueFromStringFunction ([] (const String& s) { return text::parse (s, false); });
        };
        // whole-number percentage, e.g. "40 %"
        auto percent = [] { return Float().withStringFromValueFunction ([] (float v, int maxLen) { return text::withUnit (v, 0, "%", maxLen); })
                                          .withValueFromStringFunction ([] (const String& s) { return text::parse (s, false); }); };
        // frequency in Hz / kHz; a bare number below 100 is read as kHz when `bareIsKilo` (low-pass: min is 1.5 kHz)
        auto freq = [] (bool bareIsKilo)
        {
            return Float().withStringFromValueFunction ([] (float v, int maxLen) { return text::hz (v, maxLen); })
                          .withValueFromStringFunction ([bareIsKilo] (const String& s)
                          {
                              float v = text::parse (s, true);
                              if (bareIsKilo && ! s.containsAnyOf ("kKhH") && v > 0.0f && v < 100.0f) v *= 1000.0f;
                              return v;
                          });
        };

        p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id::doubles, 1 }, "Doubles", 1, kMaxVoices, 2,
            AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int maxLen) { return text::fit (String (v), maxLen); })
                                         .withValueFromStringFunction ([] (const String& s) { return jlimit (1, kMaxVoices, s.getIntValue()); })));

        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::detune, 1 }, "Detune",
            NormalisableRange<float> (0.0f, 50.0f, 0.1f), 12.0f, unit ("ct", 1)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::delay, 1 }, "Delay",
            NormalisableRange<float> (5.0f, 40.0f, 0.1f), 18.0f, unit ("ms", 1)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::humanize, 1 }, "Humanize",
            NormalisableRange<float> (0.0f, 100.0f, 1.0f), 40.0f, percent()));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::drift, 1 }, "Drift",
            NormalisableRange<float> (0.05f, 1.5f, 0.01f, 0.5f), 0.35f, unit ("Hz", 2)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::width, 1 }, "Width",
            NormalisableRange<float> (0.0f, 100.0f, 1.0f), 80.0f, percent()));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::hpFreq, 1 }, "High-Pass",
            NormalisableRange<float> (20.0f, 800.0f, 1.0f, 0.4f), 120.0f, freq (false)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::lpFreq, 1 }, "Low-Pass",
            NormalisableRange<float> (1500.0f, 20000.0f, 1.0f, 0.4f), 12000.0f, freq (true)));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::mix, 1 }, "Mix",
            NormalisableRange<float> (0.0f, 100.0f, 1.0f), 40.0f, percent()));
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id::outGain, 1 }, "Output",
            NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, unit ("dB", 1)));

        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::bypass, 1 }, "Bypass", false));
        p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id::simple, 1 }, "Simple", false,
            AudioParameterBoolAttributes().withAutomatable (false)));

        return { p.begin(), p.end() };
    }
}
