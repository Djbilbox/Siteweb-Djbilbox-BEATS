#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

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

    f (lfoRate, "LFO Rate", 0.05f, 25.f, 4.5f, 0.4f);
    f (lfoAmount, "LFO Amount", 0.f, 1.f, 0.35f);
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { ids[lfoPitch], 1 }, "LFO Pitch Switch", false));
    f (lfoAtk, "LFO Attack", 0.001f, 5.f, 0.1f, 0.35f);
    f (lfoDec, "LFO Decay", 0.005f, 8.f, 0.5f, 0.35f);
    f (lfoSus, "LFO Sustain", 0.f, 1.f, 1.f);
    f (lfoRel, "LFO Release", 0.005f, 8.f, 0.4f, 0.35f);
    f (ampAtk, "Amp Attack", 0.001f, 5.f, 0.02f, 0.35f);
    f (ampDec, "Amp Decay", 0.005f, 8.f, 0.8f, 0.35f);
    f (ampSus, "Amp Sustain", 0.f, 1.f, 0.7f);
    f (velBank, "Velocity Bank", 0.f, 1.f, 0.75f);
    f (velAdsr, "Velocity ADSR", 0.f, 1.f, 0.4f);
    f (volume, "Volume", 0.f, 1.f, 0.8f);
    f (reverb, "Reverb", 0.f, 1.f, 0.35f);
    f (chorus, "Chorus", 0.f, 1.f, 0.45f);
    f (dist, "Distortion Master", 0.f, 1.f, 0.25f);
    f (glide, "Glide", 0.f, 1.f, 0.05f, 0.4f);
    f (cutoff, "Filter", 20.f, 20000.f, 3200.f, 0.25f);
    f (drive, "Distortion Drive", 0.f, 1.f, 0.15f);
    f (delayMix, "Delay Mix", 0.f, 1.f, 0.3f);
    f (delayTime, "Delay Time", 0.01f, 1.f, 0.375f);
    f (delayFb, "Delay Feedback", 0.f, 0.95f, 0.45f);
    c (wave, "Waveform", waveNames, 0);
    c (lfoSrc, "LFO Source", lfoNames, 1);
    c (lfoDst, "Destination", destNames, 0);
    c (ftype, "Filter Type", filterNames, 0);
    c (vmode, "Mono/Poly", modeNames, 0);
    f (bend, "Pitch Bend", -1.f, 1.f, 0.f);
    f (modw, "Mod Wheel", 0.f, 1.f, 0.f);
    f (res, "Filter Resonance", 0.f, 1.f, 0.2f);
    f (fenv, "Filter Env", 0.f, 1.f, 0.f);
    f (ampRel, "Amp Release", 0.01f, 8.f, 0.4f, 0.35f);
    f (detune, "Osc Detune", 0.f, 1.f, 0.4f);
    return l;
}

struct ParamPtrs
{
    std::array<std::atomic<float>*, COUNT> p {};
    void bind (juce::AudioProcessorValueTreeState& a) { for (int i = 0; i < COUNT; ++i) p[(size_t) i] = a.getRawParameterValue (ids[i]); }
    float operator() (int i) const { return p[(size_t) i]->load(); }
};
}
