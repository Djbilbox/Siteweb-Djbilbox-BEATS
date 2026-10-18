#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>
#include <functional>

namespace Oc
{
enum P { rate, amount, pitch, glide, filter, volume, atk, dec, sus, rel,
         t1, t2, t3, t4, t5, fenv, fatk, fdec, fsus, frel, reso, velocity, reverb, chorus, dist, delay, dtime, fb,
         wave, lfosrc, dest, ftype, mono, bend, modw, COUNT };

inline const char* const ids[COUNT] = {
    "rate", "amount", "pitch", "glide", "filter", "volume", "attack", "decay", "sustain", "release",
    "t1", "t2", "t3", "t4", "t5", "fenv", "fattack", "fdecay", "fsustain", "frelease", "resonance", "velocity", "reverb", "chorus", "distortion", "delay", "time", "feedback",
    "waveform", "lfosource", "destination", "filtertype", "monopoly", "bend", "modwheel" };

inline const juce::StringArray engineNames { "Kraken Sub", "Bubble Pluck", "Coral Bells", "Abyss Pad", "Ink Reese", "Tentacle Lead", "Siren Choir", "Whale Song" };
inline const juce::StringArray lfoNames    { "Sine", "Triangle", "Saw", "Square", "Sample&Hold" };
inline const juce::StringArray destNames   { "Pitch", "Filter", "Amp", "Timbre" };
inline const juce::StringArray filterNames { "Low Pass", "High Pass", "Band Pass" };
inline const juce::StringArray monoNames   { "Poly", "Mono" };

// ---- parameter text: display strings and typed-entry parsing (units shown in hosts, popups and automation lanes) ----
namespace txt
{
struct Fmt
{
    std::function<juce::String (float, int)> toText;
    std::function<float (const juce::String&)> fromText;
};

inline Fmt percent()      // 0..1 shown as "35 %"
{
    return { [] (float v, int) { return juce::String (juce::roundToInt (v * 100.f)) + " %"; },
             [] (const juce::String& s) { return s.getFloatValue() / 100.f; } };
}
inline Fmt lfoHz()        // "4.00 Hz"
{
    return { [] (float v, int) { return juce::String (v, 2) + " Hz"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
inline Fmt cutoffHz()     // "440 Hz" / "9.00 kHz"
{
    return { [] (float v, int) { return v >= 1000.f ? juce::String (v / 1000.f, 2) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz"; },
             [] (const juce::String& s) { return s.getFloatValue() * (s.containsIgnoreCase ("k") ? 1000.f : 1.f); } };
}
inline Fmt seconds (float maxSeconds)   // "12 ms" / "1.50 s"; a bare number above maxSeconds is read as milliseconds
{
    return { [] (float v, int)
             {
                 if (v >= 1.f) return juce::String (v, 2) + " s";
                 const float ms = v * 1000.f;
                 return (ms < 10.f ? juce::String (ms, 1) : juce::String (juce::roundToInt (ms))) + " ms";
             },
             [maxSeconds] (const juce::String& s)
             {
                 const float n = s.getFloatValue(); const auto t = s.trim().toLowerCase();
                 if (t.endsWith ("ms")) return n / 1000.f;
                 if (t.endsWithChar ('s')) return n;
                 return n > maxSeconds ? n / 1000.f : n;
             } };
}
inline Fmt gainDb()       // linear gain 0..1 shown as "-3.1 dB"
{
    return { [] (float v, int) { return v <= 0.0001f ? juce::String ("-inf dB") : juce::String (20.f * std::log10 (v), 1) + " dB"; },
             [] (const juce::String& s)
             {
                 if (s.containsIgnoreCase ("inf")) return 0.f;
                 return std::pow (10.f, s.getFloatValue() / 20.f);
             } };
}
inline Fmt semitones()    // "+7 st"
{
    return { [] (float v, int) { const int i = juce::roundToInt (v); return (i > 0 ? "+" : "") + juce::String (i) + " st"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
inline Fmt bendCents()    // pitch-bend wheel -1..1 = +/-2 semitones, shown as "+100 ct"
{
    return { [] (float v, int) { const int c = juce::roundToInt (v * 200.f); return (c > 0 ? "+" : "") + juce::String (c) + " ct"; },
             [] (const juce::String& s) { return s.getFloatValue() / 200.f; } };
}
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using F = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    // every float parameter carries a text formatter (with unit) and a parser; interval 0 = continuous
    auto f = [&] (int i, const char* n, float lo, float hi, float def, float skew, const txt::Fmt& fm, float interval = 0.f)
    {
        l.add (std::make_unique<F> (juce::ParameterID { ids[i], 1 }, n, R (lo, hi, interval, skew), def,
                                    juce::AudioParameterFloatAttributes().withStringFromValueFunction (fm.toText)
                                                                         .withValueFromStringFunction (fm.fromText)));
    };
    auto c = [&] (int i, const char* n, const juce::StringArray& s, int def = 0)
    { l.add (std::make_unique<C> (juce::ParameterID { ids[i], 1 }, n, s, def)); };

    f (rate, "LFO Rate", 0.05f, 20.f, 4.f, 0.4f, txt::lfoHz());
    f (amount, "LFO Amount", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (pitch, "Transpose", -12.f, 12.f, 0.f, 1.f, txt::semitones(), 1.f);
    f (glide, "Glide", 0.f, 2.f, 0.f, 0.4f, txt::seconds (2.f));
    f (filter, "Filter", 40.f, 18000.f, 9000.f, 0.25f, txt::cutoffHz());
    f (volume, "Volume", 0.f, 1.f, 0.7f, 1.f, txt::gainDb());
    f (atk, "Attack", 0.001f, 5.f, 0.01f, 0.3f, txt::seconds (5.f));
    f (dec, "Decay", 0.01f, 5.f, 0.4f, 0.3f, txt::seconds (5.f));
    f (sus, "Sustain", 0.f, 1.f, 0.7f, 1.f, txt::percent());
    f (rel, "Release", 0.02f, 8.f, 0.4f, 0.3f, txt::seconds (8.f));
    f (t1, "Tone", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (t2, "Body", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (t3, "Brightness", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (t4, "Width", 0.f, 1.f, 0.3f, 1.f, txt::percent());
    f (t5, "Texture", 0.f, 1.f, 0.2f, 1.f, txt::percent());
    f (fenv, "Filter Env Amount", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (fatk, "Filter Attack", 0.002f, 3.f, 0.002f, 0.4f, txt::seconds (3.f));
    f (fdec, "Filter Decay", 0.02f, 4.f, 0.3f, 0.4f, txt::seconds (4.f));
    f (fsus, "Filter Sustain", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (frel, "Filter Release", 0.02f, 4.f, 0.3f, 0.4f, txt::seconds (4.f));
    f (reso, "Resonance", 0.f, 1.f, 0.15f, 1.f, txt::percent());
    f (velocity, "Velocity Sensitivity", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (reverb, "Reverb", 0.f, 1.f, 0.25f, 1.f, txt::percent());
    f (chorus, "Chorus", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (dist, "Distortion", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (delay, "Delay Mix", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (dtime, "Delay Time", 0.05f, 1.f, 0.35f, 1.f, txt::seconds (1.f));
    f (fb, "Delay Feedback", 0.f, 0.9f, 0.35f, 1.f, txt::percent());
    c (wave, "Waveform", engineNames);
    c (lfosrc, "LFO Source", lfoNames);
    c (dest, "Destination", destNames);
    c (ftype, "Filter Type", filterNames);
    c (mono, "Mono/Poly", monoNames);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f, 1.f, txt::bendCents());
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f, 1.f, txt::percent());
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
