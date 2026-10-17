#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include <cmath>

namespace rk
{

//==============================================================================
// Typed access to APVTS raw (denormalised, real-unit) values.
// getRawParameterValue() returns the parameter's actual value (Hz / dB / ms / %),
// not a normalised 0..1 value, so we read it directly and normalise only when a
// 0..1 fraction is needed (get01).
//==============================================================================
struct Param
{
    std::atomic<float>* v = nullptr;
    juce::NormalisableRange<float> r;

    float get01() const noexcept { return v != nullptr ? r.convertTo0to1 (v->load()) : 0.0f; }
    float get() const noexcept   { return v != nullptr ? v->load() : 0.0f; }
    int   getInt() const noexcept { return (int) std::round (get()); }
    bool  getBool() const noexcept { return get() >= 0.5f; }
};

//==============================================================================
// ROCKPUNK GUITAR - hybrid guitar engine parameters.
// Dual-layer plucked-string engine (Layer A / Layer B) + "Urban Slayer" amp.
//==============================================================================
struct GuitarParams
{
    // engine / common
    Param layerMix, masterVol, outWidth, outLimiter;

    // sampled-guitar layer (Player sample library WAV, pitch-shifted across the keyboard)
    Param smpOn, smpSel, smpRoot, smpLevel, smpLoop, smpAttack, smpRelease;

    // layer A (rhythm / clean reference)
    Param laOn, laBlend, laTone, laBody, laSustain, laPick, laLevel;

    // layer B (lead / hot reference)
    Param lbOn, lbBlend, lbTone, lbBody, lbSustain, lbPick, lbLevel;

    // amp "Urban Slayer"
    Param ampOn, ampGain, ampDist, ampTone, ampLevel;

    // fx - drive (ROCKPUNK signature)
    Param driveAmt, driveType, driveTone;

    // fx - delay (ROCKPUNK signature)
    Param delayOn, delayTime, delaySync, delayDiv, delayFb, delayMix, delayLP;

    // enveloppe, glissando et mode de jeu : ce que la facade annonce sous
    // ATTACK / DECAY / SUSTAIN / RELEASE, GLIDE et [MONO/POLY]
    Param envA, envD, envS, envR, glideMs, monoMode;

    // REVERB et CHORUS : deux vrais effets, et non le dosage du delay et la
    // largeur stereo qui portaient ces noms sur l'image
    Param revMix, revSize, chorAmt;

    // LFO de la facade : RATE / AMOUNT / PITCH, sa forme ([LFO SOURCE]) et sa
    // destination ([DESTINATION])
    Param lfoRate, lfoAmount, lfoPitch, lfoWave, lfoDest;

    // Filtre : le potard FILTER et le selecteur [FILTER TYPE], avec sa propre
    // enveloppe sur les faders A S D R de droite
    Param fltType, fltCutoff, fltReso, fenvA, fenvD, fenvS, fenvR;

    // Reste de la facade : fader VELOCITY, molettes, et [WAVEFORM]
    Param velAmt, pbend, modDepth, pickShape;
};

//==============================================================================
inline float midiToFreq (float note) noexcept
{
    return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
}

inline float dbToGain (float db) noexcept { return std::pow (10.0f, db / 20.0f); }

// Tempo-synced divisions. Index: 0=1/16, 1=1/8T, 2=1/8, 3=1/4T, 4=1/4, 5=1/2, 6=1 bar, 7=2 bar, 8=4 bar.
inline const float* divisionMultipliers() noexcept
{
    static const float m[9] = { 0.25f, 1.0f / 3.0f, 0.5f, 2.0f / 3.0f, 1.0f, 2.0f, 4.0f, 8.0f, 16.0f };
    return m;
}

inline float divisionToMs (float bpm, int div) noexcept
{
    const int d = juce::jlimit (0, 8, div);
    return (60000.0f / std::max (1.0f, bpm)) * divisionMultipliers()[(size_t) d];
}

//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createGuitarParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using API = juce::AudioParameterInt;
    using APB = juce::AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloat = [&layout] (const juce::String& id, const juce::String& name,
                               float min, float max, float def, float skew, const juce::String& unit)
    {
        layout.add (std::make_unique<APF> (id, name,
                                           juce::NormalisableRange<float> (min, max, 0.0001f, skew),
                                           def, unit));
    };

    auto addInt = [&layout] (const juce::String& id, const juce::String& name, int min, int max, int def)
    {
        layout.add (std::make_unique<API> (id, name, min, max, def));
    };

    auto addBool = [&layout] (const juce::String& id, const juce::String& name, bool def)
    {
        layout.add (std::make_unique<APB> (id, name, def));
    };

    // ---- engine / common ----
    addFloat ("layerMix", "Layer Mix", 0.0f, 100.0f, 50.0f, 1.0f, " %");
    addFloat ("masterVol", "Volume", -60.0f, 6.0f, -3.0f, 0.3f, " dB");
    addFloat ("outWidth", "Width", 0.0f, 100.0f, 80.0f, 1.0f, " %");
    addBool  ("outLimiter", "Limiter", true);

    // ---- sampled-guitar layer ----
    addBool  ("smpOn", "Sample On", false);
    // La banque interne depasse 128 entrees (Electric + Acoustic + Player).
    addInt   ("smpSel", "Sample Index", 0, 255, 0);
    addInt   ("smpRoot", "Sample Root Offset", -12, 12, 0);
    addFloat ("smpLevel", "Sample Level", 0.0f, 1.0f, 0.8f, 1.0f, "");
    addBool  ("smpLoop", "Sample Loop", false);
    addFloat ("smpAttack", "Sample Attack", 0.0f, 500.0f, 5.0f, 1.0f, " ms");
    addFloat ("smpRelease", "Sample Release", 0.0f, 3000.0f, 250.0f, 1.0f, " ms");

    // ---- layer A ----
    addBool  ("laOn", "Layer A On", true);
    addFloat ("laBlend", "Layer A Blend", 0.0f, 100.0f, 80.0f, 1.0f, " %");
    addFloat ("laTone", "Layer A Tone", 0.0f, 100.0f, 60.0f, 1.0f, " %");
    addFloat ("laBody", "Layer A Body", 0.0f, 100.0f, 30.0f, 1.0f, " %");
    addFloat ("laSustain", "Layer A Sustain", 0.0f, 100.0f, 35.0f, 1.0f, " %");
    addFloat ("laPick", "Layer A Pick", 0.0f, 100.0f, 30.0f, 1.0f, " %");
    addFloat ("laLevel", "Layer A Level", 0.0f, 1.0f, 1.0f, 1.0f, "");

    // ---- layer B ----
    addBool  ("lbOn", "Layer B On", true);
    addFloat ("lbBlend", "Layer B Blend", 0.0f, 100.0f, 90.0f, 1.0f, " %");
    addFloat ("lbTone", "Layer B Tone", 0.0f, 100.0f, 25.0f, 1.0f, " %");
    addFloat ("lbBody", "Layer B Body", 0.0f, 100.0f, 25.0f, 1.0f, " %");
    addFloat ("lbSustain", "Layer B Sustain", 0.0f, 100.0f, 55.0f, 1.0f, " %");
    addFloat ("lbPick", "Layer B Pick", 0.0f, 100.0f, 70.0f, 1.0f, " %");
    addFloat ("lbLevel", "Layer B Level", 0.0f, 1.0f, 1.0f, 1.0f, "");

    // ---- amp "Urban Slayer" ----
    addBool  ("ampOn", "Amp On", true);
    addFloat ("ampGain", "Amp Gain", 0.0f, 24.0f, 6.0f, 0.4f, " dB");
    addFloat ("ampDist", "Amp Distortion", 0.0f, 100.0f, 55.0f, 1.0f, " %");
    addFloat ("ampTone", "Amp Tone", 100.0f, 10000.0f, 2500.0f, 0.3f, " Hz");
    addFloat ("ampLevel", "Amp Level", -12.0f, 12.0f, 0.0f, 1.0f, " dB");

    // ---- drive (ROCKPUNK signature) ----
    addFloat ("driveAmt", "Drive", 0.0f, 100.0f, 20.0f, 1.0f, " %");
    addInt   ("driveType", "Drive Type", 0, 3, 0);
    addFloat ("driveTone", "Drive Tone", 100.0f, 10000.0f, 2500.0f, 0.3f, " Hz");

    // ---- delay (ROCKPUNK signature) ----
    addBool  ("delayOn", "Delay On", false);
    addFloat ("delayTime", "Delay Time", 10.0f, 2000.0f, 350.0f, 0.3f, " ms");
    addBool  ("delaySync", "Delay Sync", false);
    addInt   ("delayDiv", "Delay Division", 0, 8, 2);
    addFloat ("delayFb", "Delay Feedback", 0.0f, 95.0f, 40.0f, 1.0f, " %");
    addFloat ("delayMix", "Delay Mix", 0.0f, 100.0f, 25.0f, 1.0f, " %");
    addFloat ("delayLP", "Delay LP", 100.0f, 20000.0f, 5000.0f, 0.3f, " Hz");

    // ---- enveloppe / jeu ----
    // La facade annonce une enveloppe, un glissando et un mode mono : ces
    // controles pilotaient jusqu'ici des reglages sans rapport avec leur nom.
    addFloat ("envA", "Attack", 0.5f, 2000.0f, 3.0f, 0.35f, " ms");
    addFloat ("envD", "Decay", 1.0f, 4000.0f, 600.0f, 0.35f, " ms");
    addFloat ("envS", "Sustain", 0.0f, 100.0f, 70.0f, 1.0f, " %");
    addFloat ("envR", "Release", 1.0f, 5000.0f, 220.0f, 0.35f, " ms");
    addFloat ("glideMs", "Glide", 0.0f, 1000.0f, 0.0f, 0.4f, " ms");
    addBool  ("monoMode", "Mono", false);

    // ---- reverb / chorus ----
    addFloat ("revMix", "Reverb", 0.0f, 100.0f, 18.0f, 1.0f, " %");
    addFloat ("revSize", "Reverb Size", 0.0f, 100.0f, 60.0f, 1.0f, " %");
    addFloat ("chorAmt", "Chorus", 0.0f, 100.0f, 0.0f, 1.0f, " %");

    // ---- LFO ----
    addFloat ("lfoRate", "LFO Rate", 0.05f, 20.0f, 5.0f, 0.35f, " Hz");
    addFloat ("lfoAmount", "LFO Amount", 0.0f, 100.0f, 0.0f, 1.0f, " %");
    addFloat ("lfoPitch", "LFO Pitch", 0.0f, 100.0f, 0.0f, 1.0f, " %");
    addInt   ("lfoWave", "LFO Wave", 0, 3, 0);      // sinus / triangle / carre / S&H
    addInt   ("lfoDest", "LFO Destination", 0, 3, 1); // pitch / filtre / volume / drive

    // ---- filtre ----
    addInt   ("fltType", "Filter Type", 0, 3, 3);   // passe-bas / bande / passe-haut / hors service
    addFloat ("fltCutoff", "Filter", 60.0f, 18000.0f, 14000.0f, 0.3f, " Hz");
    addFloat ("fltReso", "Filter Resonance", 0.5f, 8.0f, 0.9f, 0.5f, "");
    addFloat ("fenvA", "Filter Attack", 0.5f, 2000.0f, 5.0f, 0.35f, " ms");
    addFloat ("fenvD", "Filter Decay", 1.0f, 4000.0f, 400.0f, 0.35f, " ms");
    addFloat ("fenvS", "Filter Sustain", 0.0f, 100.0f, 100.0f, 1.0f, " %");
    addFloat ("fenvR", "Filter Release", 1.0f, 5000.0f, 300.0f, 0.35f, " ms");

    // ---- reste de la facade ----
    addFloat ("velAmt", "Velocity", 0.0f, 100.0f, 60.0f, 1.0f, " %");
    addFloat ("pbend", "Pitch Bend", -2.0f, 2.0f, 0.0f, 1.0f, " demi-tons");
    addFloat ("modDepth", "Modulation", 0.0f, 100.0f, 0.0f, 1.0f, " %");
    addInt   ("pickShape", "Pick Shape", 0, 3, 1);  // doux / rond / brillant / tranchant

    return layout;
}

//==============================================================================
inline void fillGuitarParams (GuitarParams& p, juce::AudioProcessorValueTreeState& apvts)
{
    auto set = [&apvts] (Param& dest, const juce::String& id)
    {
        auto* par = apvts.getParameter (id);
        if (par != nullptr)
        {
            dest.r = par->getNormalisableRange();
            dest.v = apvts.getRawParameterValue (id);
        }
    };

    set (p.layerMix, "layerMix");   set (p.masterVol, "masterVol");
    set (p.outWidth, "outWidth");   set (p.outLimiter, "outLimiter");

    set (p.smpOn, "smpOn");         set (p.smpSel, "smpSel");
    set (p.smpRoot, "smpRoot");     set (p.smpLevel, "smpLevel");
    set (p.smpLoop, "smpLoop");     set (p.smpAttack, "smpAttack");
    set (p.smpRelease, "smpRelease");

    set (p.laOn, "laOn");           set (p.laBlend, "laBlend");
    set (p.laTone, "laTone");       set (p.laBody, "laBody");
    set (p.laSustain, "laSustain"); set (p.laPick, "laPick");
    set (p.laLevel, "laLevel");

    set (p.lbOn, "lbOn");           set (p.lbBlend, "lbBlend");
    set (p.lbTone, "lbTone");       set (p.lbBody, "lbBody");
    set (p.lbSustain, "lbSustain"); set (p.lbPick, "lbPick");
    set (p.lbLevel, "lbLevel");

    set (p.ampOn, "ampOn");         set (p.ampGain, "ampGain");
    set (p.ampDist, "ampDist");     set (p.ampTone, "ampTone");
    set (p.ampLevel, "ampLevel");

    set (p.driveAmt, "driveAmt");   set (p.driveType, "driveType");
    set (p.driveTone, "driveTone");

    set (p.delayOn, "delayOn");     set (p.delayTime, "delayTime");
    set (p.delaySync, "delaySync"); set (p.delayDiv, "delayDiv");
    set (p.delayFb, "delayFb");     set (p.delayMix, "delayMix");
    set (p.delayLP, "delayLP");

    set (p.envA, "envA");           set (p.envD, "envD");
    set (p.envS, "envS");           set (p.envR, "envR");
    set (p.glideMs, "glideMs");     set (p.monoMode, "monoMode");

    set (p.revMix, "revMix");       set (p.revSize, "revSize");
    set (p.chorAmt, "chorAmt");

    set (p.lfoRate, "lfoRate");     set (p.lfoAmount, "lfoAmount");
    set (p.lfoPitch, "lfoPitch");   set (p.lfoWave, "lfoWave");
    set (p.lfoDest, "lfoDest");

    set (p.fltType, "fltType");     set (p.fltCutoff, "fltCutoff");
    set (p.fltReso, "fltReso");
    set (p.fenvA, "fenvA");         set (p.fenvD, "fenvD");
    set (p.fenvS, "fenvS");         set (p.fenvR, "fenvR");

    set (p.velAmt, "velAmt");       set (p.pbend, "pbend");
    set (p.modDepth, "modDepth");   set (p.pickShape, "pickShape");
}

} // namespace rk