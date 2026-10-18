#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>
#include <functional>

namespace Wz
{
enum P { rate, amount, pitch, glide, filter, reso, envdepth, attack, decay, sustain, release,
         fattack, fdecay, fsustain, velocity, volume, reverb, chorus, dist, dtime, feedback, dmix,
         unison, detune, waveform, lfosrc, dest, ftype, mono, bend, modw,
         dtone, crate, camount, rroom, rdamp, rwidth, uwidth, COUNT };

inline const char* const ids[COUNT] = {
    "rate", "amount", "pitch", "glide", "filter", "resonance", "envdepth", "attack", "decay", "sustain", "release",
    "fattack", "fdecay", "fsustain", "velocity", "volume", "reverb", "chorus", "distortion", "time", "feedback", "delaymix",
    "unison", "detune", "waveform", "lfosource", "destination", "filtertype", "monopoly", "bend", "modwheel",
    "disttone", "chorusrate", "chorusamount", "reverbroom", "reverbdamp", "reverbwidth", "unisonwidth" };

inline const juce::StringArray waveNames   { "Sine", "Square", "Sawtooth", "Triangle", "Noise" };
inline const juce::StringArray lfoNames    { "Sine", "Square", "Sawtooth", "Triangle", "Noise" };
inline const juce::StringArray destNames   { "Pitch", "Filter", "Amplitude" };
inline const juce::StringArray filterNames { "Low Pass", "Band Pass", "High Pass" };
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
inline Fmt hz (int decimals)   // "2.00 Hz"
{
    return { [decimals] (float v, int) { return juce::String (v, decimals) + " Hz"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
inline Fmt cutoffHz()     // "440 Hz" / "9.00 kHz"
{
    return { [] (float v, int) { return v >= 1000.f ? juce::String (v / 1000.f, 2) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz"; },
             [] (const juce::String& s) { return s.getFloatValue() * (s.containsIgnoreCase ("k") ? 1000.f : 1.f); } };
}
inline Fmt millis()       // value in ms: "12 ms" / "1.50 s"
{
    return { [] (float v, int)
             {
                 if (v >= 1000.f) return juce::String (v / 1000.f, 2) + " s";
                 return (v < 10.f ? juce::String (v, 1) : juce::String (juce::roundToInt (v))) + " ms";
             },
             [] (const juce::String& s)
             {
                 const float n = s.getFloatValue(); const auto t = s.trim().toLowerCase();
                 if (t.endsWith ("ms")) return n;
                 if (t.endsWithChar ('s')) return n * 1000.f;
                 return n;
             } };
}
inline Fmt gainDb()       // linear gain shown as "-3.1 dB"
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
inline Fmt cents()        // value in cents: "+25 ct"
{
    return { [] (float v, int) { const int c = juce::roundToInt (v); return (c > 0 ? "+" : "") + juce::String (c) + " ct"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
inline Fmt bendCents()    // pitch-bend wheel -1..1 = +/-2 semitones, shown as "+100 ct"
{
    return { [] (float v, int) { const int c = juce::roundToInt (v * 200.f); return (c > 0 ? "+" : "") + juce::String (c) + " ct"; },
             [] (const juce::String& s) { return s.getFloatValue() / 200.f; } };
}
inline Fmt octaves()      // "2.50 oct"
{
    return { [] (float v, int) { return juce::String (v, 2) + " oct"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
inline Fmt qFactor()      // "1.00 Q"
{
    return { [] (float v, int) { return juce::String (v, 2) + " Q"; },
             [] (const juce::String& s) { return s.getFloatValue(); } };
}
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using F = juce::AudioParameterFloat; using C = juce::AudioParameterChoice; using I = juce::AudioParameterInt;
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

    f (rate, "LFO Rate", 0.05f, 20.f, 2.f, 0.4f, txt::hz (2));
    f (amount, "LFO Amount", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (pitch, "Transpose", -12.f, 12.f, 0.f, 1.f, txt::semitones(), 1.f);
    f (glide, "Glide", 0.f, 1000.f, 0.f, 0.4f, txt::millis());
    f (filter, "Filter", 20.f, 20000.f, 8000.f, 0.25f, txt::cutoffHz());
    f (reso, "Resonance", 0.5f, 20.f, 1.f, 0.5f, txt::qFactor());
    f (envdepth, "Filter Env Depth", 0.f, 8.f, 0.f, 1.f, txt::octaves());
    f (attack, "Attack", 0.5f, 3000.f, 2.f, 0.3f, txt::millis());
    f (decay, "Decay", 5.f, 4000.f, 300.f, 0.3f, txt::millis());
    f (sustain, "Sustain", 0.f, 1.f, 0.7f, 1.f, txt::percent());
    f (release, "Release", 5.f, 5000.f, 250.f, 0.3f, txt::millis());
    f (fattack, "Filter Attack", 0.5f, 3000.f, 1.f, 0.3f, txt::millis());
    f (fdecay, "Filter Decay", 5.f, 4000.f, 300.f, 0.3f, txt::millis());
    f (fsustain, "Filter Sustain", 0.f, 1.f, 0.7f, 1.f, txt::percent());
    f (velocity, "Velocity Sensitivity", 0.f, 1.f, 0.4f, 1.f, txt::percent());
    f (volume, "Volume", 0.f, 1.5f, 1.f, 1.f, txt::gainDb());
    f (reverb, "Reverb", 0.f, 1.f, 0.1f, 1.f, txt::percent());
    f (chorus, "Chorus", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (dist, "Distortion", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (dtime, "Delay Time", 50.f, 1000.f, 250.f, 1.f, txt::millis());
    f (feedback, "Delay Feedback", 0.f, 0.9f, 0.3f, 1.f, txt::percent());
    f (dmix, "Delay Mix", 0.f, 1.f, 0.f, 1.f, txt::percent());
    l.add (std::make_unique<I> (juce::ParameterID { ids[unison], 1 }, "Unison", 1, 7, 1));
    f (detune, "Unison Detune", 0.f, 60.f, 0.f, 1.f, txt::cents());
    c (waveform, "Waveform", waveNames);  c (lfosrc, "LFO Source", lfoNames);  c (dest, "Destination", destNames);
    c (ftype, "Filter Type", filterNames); c (mono, "Mono/Poly", monoNames, 1);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f, 1.f, txt::bendCents());
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (dtone, "Distortion Tone", 100.f, 8000.f, 5000.f, 0.4f, txt::cutoffHz());
    f (crate, "Chorus Rate", 0.1f, 8.f, 1.5f, 0.5f, txt::hz (2));
    f (camount, "Chorus Depth", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (rroom, "Reverb Room", 0.f, 1.f, 0.4f, 1.f, txt::percent());
    f (rdamp, "Reverb Damping", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    f (rwidth, "Reverb Width", 0.f, 1.f, 1.f, 1.f, txt::percent());
    f (uwidth, "Unison Width", 0.f, 1.f, 0.5f, 1.f, txt::percent());
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
