#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <cmath>
#include <functional>

// Parametres K2000 : ids et plages conformes a PARAMETER_MAP.md,
// + 4 controles ajoutes (filter_res, filter_env, amp_release, osc_detune) absents de la maquette.
namespace K2k
{
enum P { lfoRate, lfoAmount, lfoPitch, lfoAtk, lfoDec, lfoSus, lfoRel,
         ampAtk, ampDec, ampSus, velBank, velAdsr,
         volume, reverb, chorus, dist, glide, cutoff, drive, delayMix, delayTime, delayFb,
         wave, lfoSrc, lfoDst, ftype, vmode, bend, modw,
         res, fenv, ampRel, detune, COUNT };

inline const char* const ids[COUNT] = {
    "lfo_rate", "lfo_amount", "lfo_pitch", "lfo_attack", "lfo_decay", "lfo_sustain", "lfo_release",
    "amp_attack", "amp_decay", "amp_sustain", "velocity_bank", "velocity_adsr",
    "master_volume", "fx_reverb", "fx_chorus", "fx_distortion", "glide_time", "filter_cutoff", "filter_drive",
    "fx_delay_mix", "fx_delay_time", "fx_delay_fb",
    "osc_waveform", "lfo_source", "lfo_dest", "filter_type", "voice_mode", "pitch_bend", "mod_wheel",
    "filter_res", "filter_env", "amp_release", "osc_detune" };

inline const juce::StringArray waveNames   { "Sawtooth", "Square", "Triangle", "Sine", "SuperSaw", "80s Brass" };
inline const juce::StringArray lfoNames    { "Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random S&H" };
inline const juce::StringArray destNames   { "Cutoff", "Pitch", "Pulse Width", "Volume", "Chorus", "Pan" };
inline const juce::StringArray filterNames { "LowPass 24dB", "LowPass 12dB", "BandPass", "HighPass", "Acid LowPass" };
inline const juce::StringArray modeNames   { "Poly 8", "Poly 16", "Mono Retrig", "Mono Legato" };

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

    f (lfoRate, "LFO Rate", 0.05f, 25.f, 4.5f, 0.4f, txt::lfoHz());
    f (lfoAmount, "LFO Amount", 0.f, 1.f, 0.35f, 1.f, txt::percent());
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { ids[lfoPitch], 1 }, "LFO Pitch Switch", false));
    f (lfoAtk, "LFO Attack", 0.001f, 5.f, 0.1f, 0.35f, txt::seconds (5.f));
    f (lfoDec, "LFO Decay", 0.005f, 8.f, 0.5f, 0.35f, txt::seconds (8.f));
    f (lfoSus, "LFO Sustain", 0.f, 1.f, 1.f, 1.f, txt::percent());
    f (lfoRel, "LFO Release", 0.005f, 8.f, 0.4f, 0.35f, txt::seconds (8.f));
    f (ampAtk, "Amp Attack", 0.001f, 5.f, 0.02f, 0.35f, txt::seconds (5.f));
    f (ampDec, "Amp Decay", 0.005f, 8.f, 0.8f, 0.35f, txt::seconds (8.f));
    f (ampSus, "Amp Sustain", 0.f, 1.f, 0.7f, 1.f, txt::percent());
    f (velBank, "Velocity Bank", 0.f, 1.f, 0.75f, 1.f, txt::percent());
    f (velAdsr, "Velocity ADSR", 0.f, 1.f, 0.4f, 1.f, txt::percent());
    f (volume, "Volume", 0.f, 1.f, 0.8f, 1.f, txt::gainDb());
    f (reverb, "Reverb", 0.f, 1.f, 0.35f, 1.f, txt::percent());
    f (chorus, "Chorus", 0.f, 1.f, 0.45f, 1.f, txt::percent());
    f (dist, "Distortion Master", 0.f, 1.f, 0.25f, 1.f, txt::percent());
    f (glide, "Glide", 0.f, 1.f, 0.05f, 0.4f, txt::seconds (1.f));
    f (cutoff, "Filter", 20.f, 20000.f, 3200.f, 0.25f, txt::cutoffHz());
    f (drive, "Distortion Drive", 0.f, 1.f, 0.15f, 1.f, txt::percent());
    f (delayMix, "Delay Mix", 0.f, 1.f, 0.3f, 1.f, txt::percent());
    f (delayTime, "Delay Time", 0.01f, 1.f, 0.375f, 1.f, txt::seconds (1.f));
    f (delayFb, "Delay Feedback", 0.f, 0.95f, 0.45f, 1.f, txt::percent());
    c (wave, "Waveform", waveNames, 0);
    c (lfoSrc, "LFO Source", lfoNames, 1);
    c (lfoDst, "Destination", destNames, 0);
    c (ftype, "Filter Type", filterNames, 0);
    c (vmode, "Mono/Poly", modeNames, 0);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f, 1.f, txt::bendCents());
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (res, "Filter Resonance", 0.f, 1.f, 0.2f, 1.f, txt::percent());
    f (fenv, "Filter Env Amount", 0.f, 1.f, 0.f, 1.f, txt::percent());
    f (ampRel, "Amp Release", 0.01f, 8.f, 0.4f, 0.35f, txt::seconds (8.f));
    f (detune, "Osc Detune", 0.f, 1.f, 0.4f, 1.f, txt::percent());
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
