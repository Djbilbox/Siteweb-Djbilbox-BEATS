#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace Sh
{
enum P { rate, amount, pitch, glide, filter, volume, atk, dec, sus, rel,
         t1, t2, t3, t4, t5, fenv, fatk, fdec, fsus, frel, reso, velocity, reverb, chorus, dist, delay, dtime, fb,
         wave, lfosrc, dest, ftype, mono, bend, modw, COUNT };

inline const char* const ids[COUNT] = {
    "rate", "amount", "pitch", "glide", "filter", "volume", "attack", "decay", "sustain", "release",
    "t1", "t2", "t3", "t4", "t5", "fenv", "fattack", "fdecay", "fsustain", "frelease", "resonance", "velocity", "reverb", "chorus", "distortion", "delay", "time", "feedback",
    "waveform", "lfosource", "destination", "filtertype", "monopoly", "bend", "modwheel" };

inline const juce::StringArray engineNames { "Didgeridoo", "Tribal Drum", "Jaw Harp", "Spirit Flute", "Chant Choir", "Fire Pad", "Bone Chimes", "Earth Drone" };
inline const juce::StringArray lfoNames    { "Sine", "Triangle", "Saw", "Square", "Sample&Hold" };
inline const juce::StringArray destNames   { "Pitch", "Filter", "Amp", "Timbre" };
inline const juce::StringArray filterNames { "Low Pass", "High Pass", "Band Pass" };
inline const juce::StringArray monoNames   { "Poly", "Mono" };

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using F = juce::AudioParameterFloat;
    using C = juce::AudioParameterChoice;
    using R = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto f = [&] (int i, const char* n, float lo, float hi, float def, float skew = 1.f)
    { l.add (std::make_unique<F> (juce::ParameterID { ids[i], 1 }, n, R (lo, hi, 0.f, skew), def)); };
    auto c = [&] (int i, const char* n, const juce::StringArray& s, int def = 0)
    { l.add (std::make_unique<C> (juce::ParameterID { ids[i], 1 }, n, s, def)); };

    f (rate, "LFO Rate", 0.05f, 20.f, 4.f, 0.4f);
    f (amount, "LFO Amount", 0.f, 1.f, 0.f);
    f (pitch, "Pitch", -12.f, 12.f, 0.f);
    f (glide, "Glide", 0.f, 2.f, 0.f, 0.4f);
    f (filter, "Filter", 40.f, 18000.f, 9000.f, 0.25f);
    f (volume, "Volume", 0.f, 1.f, 0.7f);
    f (atk, "Attack", 0.001f, 5.f, 0.01f, 0.3f);
    f (dec, "Decay", 0.01f, 5.f, 0.4f, 0.3f);
    f (sus, "Sustain", 0.f, 1.f, 0.7f);
    f (rel, "Release", 0.02f, 8.f, 0.4f, 0.3f);
    f (t1, "Tone", 0.f, 1.f, 0.5f);
    f (t2, "Body", 0.f, 1.f, 0.5f);
    f (t3, "Brightness", 0.f, 1.f, 0.5f);
    f (t4, "Width", 0.f, 1.f, 0.3f);
    f (t5, "Texture", 0.f, 1.f, 0.2f);
    f (fenv, "Filter Env", 0.f, 1.f, 0.f);
    f (fatk, "Filter Attack", 0.002f, 3.f, 0.002f, 0.4f);
    f (fdec, "Filter Decay", 0.02f, 4.f, 0.3f, 0.4f);
    f (fsus, "Filter Sustain", 0.f, 1.f, 0.f);
    f (frel, "Filter Release", 0.02f, 4.f, 0.3f, 0.4f);
    f (reso, "Resonance", 0.f, 1.f, 0.15f);
    f (velocity, "Velocity", 0.f, 1.f, 0.5f);
    f (reverb, "Reverb", 0.f, 1.f, 0.25f);
    f (chorus, "Chorus", 0.f, 1.f, 0.f);
    f (dist, "Distortion", 0.f, 1.f, 0.f);
    f (delay, "Delay", 0.f, 1.f, 0.f);
    f (dtime, "Delay Time", 0.05f, 1.f, 0.35f);
    f (fb, "Feedback", 0.f, 0.9f, 0.35f);
    c (wave, "Waveform", engineNames);
    c (lfosrc, "LFO Source", lfoNames);
    c (dest, "Destination", destNames);
    c (ftype, "Filter Type", filterNames);
    c (mono, "Mono/Poly", monoNames);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f);
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f);
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
