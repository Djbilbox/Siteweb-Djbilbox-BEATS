#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

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

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using F = juce::AudioParameterFloat; using C = juce::AudioParameterChoice; using I = juce::AudioParameterInt;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&] (int i, const char* n, float lo, float hi, float def, float skew = 1.f)
    { l.add (std::make_unique<F> (juce::ParameterID { ids[i], 1 }, n, R (lo, hi, 0.f, skew), def)); };
    auto c = [&] (int i, const char* n, const juce::StringArray& s, int def = 0)
    { l.add (std::make_unique<C> (juce::ParameterID { ids[i], 1 }, n, s, def)); };

    f (rate, "LFO Rate", 0.05f, 20.f, 2.f, 0.4f);        f (amount, "LFO Amount", 0.f, 1.f, 0.f);
    f (pitch, "Pitch", -12.f, 12.f, 0.f);                f (glide, "Glide", 0.f, 1000.f, 0.f, 0.4f);
    f (filter, "Filter", 20.f, 20000.f, 8000.f, 0.25f);  f (reso, "Resonance", 0.5f, 20.f, 1.f, 0.5f);
    f (envdepth, "Filter Env Depth", 0.f, 8.f, 0.f);
    f (attack, "Attack", 0.5f, 3000.f, 2.f, 0.3f);       f (decay, "Decay", 5.f, 4000.f, 300.f, 0.3f);
    f (sustain, "Sustain", 0.f, 1.f, 0.7f);              f (release, "Release", 5.f, 5000.f, 250.f, 0.3f);
    f (fattack, "Filter Attack", 0.5f, 3000.f, 1.f, 0.3f); f (fdecay, "Filter Decay", 5.f, 4000.f, 300.f, 0.3f);
    f (fsustain, "Filter Sustain", 0.f, 1.f, 0.7f);      f (velocity, "Velocity", 0.f, 1.f, 0.4f);
    f (volume, "Volume", 0.f, 1.5f, 1.f);
    f (reverb, "Reverb", 0.f, 1.f, 0.1f);                f (chorus, "Chorus", 0.f, 1.f, 0.f);
    f (dist, "Distortion", 0.f, 1.f, 0.f);
    f (dtime, "Delay Time", 50.f, 1000.f, 250.f);         f (feedback, "Feedback", 0.f, 0.9f, 0.3f);
    f (dmix, "Delay Mix", 0.f, 1.f, 0.f);
    l.add (std::make_unique<I> (juce::ParameterID { ids[unison], 1 }, "Unison", 1, 7, 1));
    f (detune, "Unison Detune", 0.f, 60.f, 0.f);
    c (waveform, "Waveform", waveNames);  c (lfosrc, "LFO Source", lfoNames);  c (dest, "Destination", destNames);
    c (ftype, "Filter Type", filterNames); c (mono, "Mono/Poly", monoNames, 1);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f);  f (modw, "Mod Wheel", 0.f, 1.f, 0.f);
    f (dtone, "Distortion Tone", 100.f, 8000.f, 5000.f, 0.4f);
    f (crate, "Chorus Rate", 0.1f, 8.f, 1.5f, 0.5f);  f (camount, "Chorus Depth", 0.f, 1.f, 0.5f);
    f (rroom, "Reverb Room", 0.f, 1.f, 0.4f);  f (rdamp, "Reverb Damping", 0.f, 1.f, 0.5f);  f (rwidth, "Reverb Width", 0.f, 1.f, 1.f);
    f (uwidth, "Unison Width", 0.f, 1.f, 0.5f);
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
