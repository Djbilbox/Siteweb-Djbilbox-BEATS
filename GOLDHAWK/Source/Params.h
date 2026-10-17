#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace Gh
{
enum P { rate, amount, pitch, glide, filter, volume, atk, dec, sus, rel,
         t1, t2, t3, t4, t5, fenv, fatk, fdec, fsus, reso, velocity, reverb, chorus, dist, delay, dtime, fb,
         wave, lfosrc, dest, ftype, mono, bend, modw, COUNT };

inline const char* const ids[COUNT] = {
    "rate", "amount", "pitch", "glide", "filter", "volume", "attack", "decay", "sustain", "release",
    "t1", "t2", "t3", "t4", "t5", "fenv", "fattack", "fdecay", "fsustain", "resonance", "velocity", "reverb", "chorus", "distortion", "delay", "time", "feedback",
    "waveform", "lfosource", "destination", "filtertype", "monopoly", "bend", "modwheel" };

inline const juce::StringArray engineNames { "Hawk Lead", "Golden Brass", "Royal Pad", "Falcon Pluck", "Sky Bells", "Feather Strings", "Talon Bass", "Eagle Cry" };
inline const juce::StringArray lfoNames    { "Sine", "Triangle", "Saw", "Square", "Sample&Hold" };
inline const juce::StringArray destNames   { "Pitch", "Filter", "Amp", "Timbre" };
inline const juce::StringArray filterNames { "Low Pass", "High Pass", "Band Pass" };
inline const juce::StringArray monoNames   { "Poly", "Mono" };

// ---- parameter text: units + sensible decimals for host automation lists, hover popups and typed values ----
enum class Unit { percent, cutoffHz, rateHz, time, gainDb, semitones, bendSemis };

inline juce::String unitText (Unit u, float v)
{
    switch (u)
    {
        case Unit::percent:   return juce::String (juce::roundToInt (v * 100.f)) + " %";
        case Unit::cutoffHz:  return v >= 1000.f ? juce::String (v / 1000.f, 2) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz";
        case Unit::rateHz:    return juce::String (v, 2) + " Hz";
        case Unit::time:      return v < 0.01f ? juce::String (v * 1000.f, 1) + " ms"
                                    : v < 0.9995f ? juce::String (juce::roundToInt (v * 1000.f)) + " ms"
                                    : juce::String (v, 2) + " s";
        case Unit::gainDb:    return v < 0.0005f ? juce::String ("-inf dB") : juce::String (juce::Decibels::gainToDecibels (v), 1) + " dB";
        case Unit::semitones: { const int s = juce::roundToInt (v); return juce::String (s > 0 ? "+" : "") + juce::String (s) + " st"; }
        case Unit::bendSemis: { const float s = v * 2.f; return juce::String (s > 0.005f ? "+" : "") + juce::String (s, 2) + " st"; }   // wheel range = +/-2 semitones
    }
    return juce::String (v, 2);
}

inline float unitValue (Unit u, const juce::String& text)
{
    const auto t = text.trim().toLowerCase();
    const float x = t.getFloatValue();   // leading number; any trailing unit is ignored
    switch (u)
    {
        case Unit::percent:   return x * 0.01f;
        case Unit::cutoffHz:  return t.contains ("k") ? x * 1000.f : x;
        case Unit::rateHz:    return x;
        case Unit::time:      return t.contains ("ms") ? x * 0.001f : (t.endsWithChar ('s') || x <= 10.f) ? x : x * 0.001f;   // plain numbers: seconds up to 10, else ms
        case Unit::gainDb:    return t.contains ("inf") ? 0.f : juce::Decibels::decibelsToGain (x);
        case Unit::semitones: return x;
        case Unit::bendSemis: return x * 0.5f;
    }
    return x;
}

inline juce::AudioParameterFloatAttributes unitAttributes (Unit u)
{
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction ([u] (float v, int) { return unitText (u, v); })
        .withValueFromStringFunction ([u] (const juce::String& s) { return unitValue (u, s); });
}

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using F = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&] (int i, const char* n, float lo, float hi, float def, Unit u, float skew = 1.f, float step = 0.f)
    { l.add (std::make_unique<F> (juce::ParameterID { ids[i], 1 }, n, R (lo, hi, step, skew), def, unitAttributes (u))); };
    auto c = [&] (int i, const char* n, const juce::StringArray& s, int def = 0)
    { l.add (std::make_unique<C> (juce::ParameterID { ids[i], 1 }, n, s, def)); };

    f (rate, "LFO Rate", 0.05f, 20.f, 4.f, Unit::rateHz, 0.4f);
    f (amount, "LFO Amount", 0.f, 1.f, 0.f, Unit::percent);
    f (pitch, "Transpose", -12.f, 12.f, 0.f, Unit::semitones, 1.f, 1.f);
    f (glide, "Glide", 0.f, 2.f, 0.f, Unit::time, 0.4f);
    f (filter, "Filter Cutoff", 40.f, 18000.f, 9000.f, Unit::cutoffHz, 0.25f);
    f (volume, "Volume", 0.f, 1.f, 0.7f, Unit::gainDb);
    f (atk, "Attack", 0.001f, 5.f, 0.01f, Unit::time, 0.3f);
    f (dec, "Decay", 0.01f, 5.f, 0.4f, Unit::time, 0.3f);
    f (sus, "Sustain", 0.f, 1.f, 0.7f, Unit::percent);
    f (rel, "Release", 0.02f, 8.f, 0.4f, Unit::time, 0.3f);
    f (t1, "Tone", 0.f, 1.f, 0.5f, Unit::percent);
    f (t2, "Body", 0.f, 1.f, 0.5f, Unit::percent);
    f (t3, "Brightness", 0.f, 1.f, 0.5f, Unit::percent);
    f (t4, "Width", 0.f, 1.f, 0.3f, Unit::percent);
    f (t5, "Texture", 0.f, 1.f, 0.2f, Unit::percent);
    f (fenv, "Filter Env", 0.f, 1.f, 0.f, Unit::percent);
    f (fatk, "Filter Attack", 0.002f, 3.f, 0.002f, Unit::time, 0.4f);
    f (fdec, "Filter Decay/Release", 0.02f, 4.f, 0.3f, Unit::time, 0.4f);
    f (fsus, "Filter Sustain", 0.f, 1.f, 0.f, Unit::percent);
    f (reso, "Resonance", 0.f, 1.f, 0.15f, Unit::percent);
    f (velocity, "Velocity", 0.f, 1.f, 0.5f, Unit::percent);
    f (reverb, "Reverb", 0.f, 1.f, 0.25f, Unit::percent);
    f (chorus, "Chorus", 0.f, 1.f, 0.f, Unit::percent);
    f (dist, "Distortion", 0.f, 1.f, 0.f, Unit::percent);
    f (delay, "Delay", 0.f, 1.f, 0.f, Unit::percent);
    f (dtime, "Delay Time", 0.05f, 1.f, 0.35f, Unit::time);
    f (fb, "Feedback", 0.f, 0.9f, 0.35f, Unit::percent);
    c (wave, "Waveform", engineNames);
    c (lfosrc, "LFO Source", lfoNames);
    c (dest, "Destination", destNames);
    c (ftype, "Filter Type", filterNames);
    c (mono, "Mono/Poly", monoNames);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f, Unit::bendSemis);
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f, Unit::percent);
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
