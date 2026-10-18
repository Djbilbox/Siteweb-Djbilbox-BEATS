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

    // sampled-guitar layer (the WAV files the user drops in the Samples folder, pitch-shifted across the keyboard)
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

    // envelope, glide and play mode: what the front panel announces as
    // ATTACK / DECAY / SUSTAIN / RELEASE, GLIDE and [MONO/POLY]
    Param envA, envD, envS, envR, glideMs, monoMode;

    // REVERB and CHORUS: two real effects
    Param revMix, revSize, chorAmt;

    // front-panel LFO: RATE / AMOUNT / PITCH, its shape ([LFO SOURCE]) and its
    // destination ([DESTINATION])
    Param lfoRate, lfoAmount, lfoPitch, lfoWave, lfoDest;

    // Filter: the FILTER knob and the [FILTER TYPE] selector, with its own
    // envelope on the A S D R faders on the right
    Param fltType, fltCutoff, fltReso, fenvA, fenvD, fenvS, fenvR;

    // Rest of the front panel: VELOCITY fader, wheels and [WAVEFORM]
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
// Names of the discrete choices. Shared by the parameter text (what a host
// shows) and by the editor (what the drop-down menus list), so the two can
// never disagree.
//==============================================================================
namespace names
{
    inline const juce::StringArray& drive()   { static const juce::StringArray a { "Soft", "Hard", "Asym", "Fuzz" }; return a; }
    inline const juce::StringArray& filter()  { static const juce::StringArray a { "Low-pass", "Band-pass", "High-pass", "Off" }; return a; }
    inline const juce::StringArray& lfoWave() { static const juce::StringArray a { "Sine", "Triangle", "Square", "S&H" }; return a; }
    inline const juce::StringArray& lfoDest() { static const juce::StringArray a { "Pitch", "Filter", "Volume", "Drive" }; return a; }
    inline const juce::StringArray& pick()    { static const juce::StringArray a { "Soft", "Round", "Bright", "Sharp" }; return a; }
    inline const juce::StringArray& delayDiv() { static const juce::StringArray a { "1/16", "1/8T", "1/8", "1/4T", "1/4", "1/2", "1 bar", "2 bar", "4 bar" }; return a; }
    inline const juce::StringArray& voiceMode() { static const juce::StringArray a { "Poly", "Mono" }; return a; }
    inline const juce::StringArray& onOff()   { static const juce::StringArray a { "Off", "On" }; return a; }
}

//==============================================================================
// Parameter text: unit and decimals that read well in a host's automation lane,
// and a parser that accepts what a person would type back ("2.5k", "-3 dB",
// "40%", "1/8T", "fuzz").
//==============================================================================
enum class Unit { percent, level01, decibel, hertz, lfoHertz, milliseconds, semitones, plain };

inline juce::String formatValue (Unit u, float v)
{
    switch (u)
    {
        case Unit::percent:      return juce::String (juce::roundToInt (v)) + " %";
        case Unit::level01:      return juce::String (juce::roundToInt (v * 100.0f)) + " %";
        case Unit::decibel:      return juce::String (v, 1) + " dB";
        case Unit::hertz:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 2) + " kHz";
            if (v >= 100.0f)  return juce::String (juce::roundToInt (v)) + " Hz";
            return juce::String (v, 1) + " Hz";
        case Unit::lfoHertz:     return juce::String (v, 2) + " Hz";
        case Unit::milliseconds:
            if (v >= 1000.0f) return juce::String (v / 1000.0f, 2) + " s";
            if (v >= 100.0f)  return juce::String (juce::roundToInt (v)) + " ms";
            return juce::String (v, 1) + " ms";
        case Unit::semitones:    return juce::String (v, 2) + " st";
        case Unit::plain:        break;
    }
    return juce::String (v, 2);
}

inline float parseValue (Unit u, const juce::String& text)
{
    const auto t = text.trim();
    const float n = t.getFloatValue();
    const bool hasK = t.containsIgnoreCase ("k");
    const bool hasMs = t.containsIgnoreCase ("ms");

    if (u == Unit::level01)      return n / 100.0f;
    if (u == Unit::hertz)        return hasK ? n * 1000.0f : n;
    if (u == Unit::milliseconds) return (! hasMs && t.containsIgnoreCase ("s")) ? n * 1000.0f : n;
    return n;
}

inline int parseChoice (const juce::StringArray& list, const juce::String& text, int fallback)
{
    const auto t = text.trim();
    for (int i = 0; i < list.size(); ++i)
        if (list[i].equalsIgnoreCase (t))
            return i;
    for (int i = 0; i < list.size(); ++i)
        if (list[i].startsWithIgnoreCase (t) && t.isNotEmpty())
            return i;
    if (t.containsOnly ("0123456789-+"))
        return t.getIntValue();
    return fallback;
}

//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createGuitarParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    using API = juce::AudioParameterInt;
    using APB = juce::AudioParameterBool;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Version hint 1 on every parameter: AU hosts need it, and it is the number
    // to raise if a parameter is ever added in an update.
    auto pid = [] (const juce::String& id) { return juce::ParameterID { id, 1 }; };

    auto addFloat = [&] (const juce::String& id, const juce::String& name,
                         float min, float max, float def, float skew, Unit unit)
    {
        layout.add (std::make_unique<APF> (
            pid (id), name,
            juce::NormalisableRange<float> (min, max, 0.0001f, skew), def,
            juce::AudioParameterFloatAttributes()
                .withStringFromValueFunction ([unit] (float v, int) { return formatValue (unit, v); })
                .withValueFromStringFunction ([unit] (const juce::String& t) { return parseValue (unit, t); })));
    };

    auto addInt = [&] (const juce::String& id, const juce::String& name, int min, int max, int def,
                       const juce::StringArray* choices = nullptr, const juce::String& suffix = {})
    {
        auto attr = juce::AudioParameterIntAttributes();
        if (choices != nullptr)
        {
            const juce::StringArray list = *choices;
            attr = attr.withStringFromValueFunction ([list, min] (int v, int)
                       { return list[juce::jlimit (0, list.size() - 1, v - min)]; })
                       .withValueFromStringFunction ([list, min, def] (const juce::String& t)
                       { return min + parseChoice (list, t, def - min); });
        }
        else
        {
            attr = attr.withStringFromValueFunction ([suffix] (int v, int) { return juce::String (v) + suffix; })
                       .withValueFromStringFunction ([def] (const juce::String& t)
                       { return t.trim().isEmpty() ? def : t.getIntValue(); });
        }
        layout.add (std::make_unique<API> (pid (id), name, min, max, def, attr));
    };

    auto addBool = [&] (const juce::String& id, const juce::String& name, bool def)
    {
        layout.add (std::make_unique<APB> (
            pid (id), name, def,
            juce::AudioParameterBoolAttributes()
                .withStringFromValueFunction ([] (bool v, int) { return v ? "On" : "Off"; })
                .withValueFromStringFunction ([] (const juce::String& t)
                {
                    const auto s = t.trim().toLowerCase();
                    return s == "on" || s == "1" || s == "true" || s == "yes";
                })));
    };

    // ---- engine / common ----
    addFloat ("layerMix", "Layer Mix", 0.0f, 100.0f, 50.0f, 1.0f, Unit::percent);
    addFloat ("masterVol", "Volume", -60.0f, 6.0f, -3.0f, 0.3f, Unit::decibel);
    addFloat ("outWidth", "Width", 0.0f, 100.0f, 80.0f, 1.0f, Unit::percent);
    addBool  ("outLimiter", "Limiter", true);

    // ---- sampled-guitar layer (plays the files the user drops in the Samples folder) ----
    addBool  ("smpOn", "Sample On", false);
    addInt   ("smpSel", "Sample Index", 0, 255, 0, nullptr, {});
    addInt   ("smpRoot", "Sample Root Offset", -12, 12, 0, nullptr, " st");
    addFloat ("smpLevel", "Sample Level", 0.0f, 1.0f, 0.8f, 1.0f, Unit::level01);
    addBool  ("smpLoop", "Sample Loop", false);
    addFloat ("smpAttack", "Sample Attack", 0.0f, 500.0f, 5.0f, 1.0f, Unit::milliseconds);
    addFloat ("smpRelease", "Sample Release", 0.0f, 3000.0f, 250.0f, 1.0f, Unit::milliseconds);

    // ---- layer A ----
    addBool  ("laOn", "Layer A On", true);
    addFloat ("laBlend", "Layer A Blend", 0.0f, 100.0f, 80.0f, 1.0f, Unit::percent);
    addFloat ("laTone", "Layer A Tone", 0.0f, 100.0f, 60.0f, 1.0f, Unit::percent);
    addFloat ("laBody", "Layer A Body", 0.0f, 100.0f, 30.0f, 1.0f, Unit::percent);
    addFloat ("laSustain", "Layer A Sustain", 0.0f, 100.0f, 35.0f, 1.0f, Unit::percent);
    addFloat ("laPick", "Layer A Pick", 0.0f, 100.0f, 30.0f, 1.0f, Unit::percent);
    addFloat ("laLevel", "Layer A Level", 0.0f, 1.0f, 1.0f, 1.0f, Unit::level01);

    // ---- layer B ----
    addBool  ("lbOn", "Layer B On", true);
    addFloat ("lbBlend", "Layer B Blend", 0.0f, 100.0f, 90.0f, 1.0f, Unit::percent);
    addFloat ("lbTone", "Layer B Tone", 0.0f, 100.0f, 25.0f, 1.0f, Unit::percent);
    addFloat ("lbBody", "Layer B Body", 0.0f, 100.0f, 25.0f, 1.0f, Unit::percent);
    addFloat ("lbSustain", "Layer B Sustain", 0.0f, 100.0f, 55.0f, 1.0f, Unit::percent);
    addFloat ("lbPick", "Layer B Pick", 0.0f, 100.0f, 70.0f, 1.0f, Unit::percent);
    addFloat ("lbLevel", "Layer B Level", 0.0f, 1.0f, 1.0f, 1.0f, Unit::level01);

    // ---- amp "Urban Slayer" ----
    addBool  ("ampOn", "Amp On", true);
    addFloat ("ampGain", "Amp Gain", 0.0f, 24.0f, 6.0f, 0.4f, Unit::decibel);
    addFloat ("ampDist", "Amp Distortion", 0.0f, 100.0f, 55.0f, 1.0f, Unit::percent);
    addFloat ("ampTone", "Amp Tone", 100.0f, 10000.0f, 2500.0f, 0.3f, Unit::hertz);
    addFloat ("ampLevel", "Amp Level", -12.0f, 12.0f, 0.0f, 1.0f, Unit::decibel);

    // ---- drive (ROCKPUNK signature) ----
    addFloat ("driveAmt", "Drive", 0.0f, 100.0f, 20.0f, 1.0f, Unit::percent);
    addInt   ("driveType", "Drive Type", 0, 3, 0, &names::drive());
    addFloat ("driveTone", "Drive Tone", 100.0f, 10000.0f, 2500.0f, 0.3f, Unit::hertz);

    // ---- delay (ROCKPUNK signature) ----
    addBool  ("delayOn", "Delay On", false);
    addFloat ("delayTime", "Delay Time", 10.0f, 2000.0f, 350.0f, 0.3f, Unit::milliseconds);
    addBool  ("delaySync", "Delay Sync", false);
    addInt   ("delayDiv", "Delay Division", 0, 8, 2, &names::delayDiv());
    addFloat ("delayFb", "Delay Feedback", 0.0f, 95.0f, 40.0f, 1.0f, Unit::percent);
    addFloat ("delayMix", "Delay Mix", 0.0f, 100.0f, 25.0f, 1.0f, Unit::percent);
    addFloat ("delayLP", "Delay LP", 100.0f, 20000.0f, 5000.0f, 0.3f, Unit::hertz);

    // ---- envelope / playing ----
    addFloat ("envA", "Attack", 0.5f, 2000.0f, 3.0f, 0.35f, Unit::milliseconds);
    addFloat ("envD", "Decay", 1.0f, 4000.0f, 600.0f, 0.35f, Unit::milliseconds);
    addFloat ("envS", "Sustain", 0.0f, 100.0f, 70.0f, 1.0f, Unit::percent);
    addFloat ("envR", "Release", 1.0f, 5000.0f, 220.0f, 0.35f, Unit::milliseconds);
    addFloat ("glideMs", "Glide", 0.0f, 1000.0f, 0.0f, 0.4f, Unit::milliseconds);
    addBool  ("monoMode", "Mono", false);

    // ---- reverb / chorus ----
    addFloat ("revMix", "Reverb", 0.0f, 100.0f, 18.0f, 1.0f, Unit::percent);
    addFloat ("revSize", "Reverb Size", 0.0f, 100.0f, 60.0f, 1.0f, Unit::percent);
    addFloat ("chorAmt", "Chorus", 0.0f, 100.0f, 0.0f, 1.0f, Unit::percent);

    // ---- LFO ----
    addFloat ("lfoRate", "LFO Rate", 0.05f, 20.0f, 5.0f, 0.35f, Unit::lfoHertz);
    addFloat ("lfoAmount", "LFO Amount", 0.0f, 100.0f, 0.0f, 1.0f, Unit::percent);
    addFloat ("lfoPitch", "LFO Pitch", 0.0f, 100.0f, 0.0f, 1.0f, Unit::percent);
    addInt   ("lfoWave", "LFO Wave", 0, 3, 0, &names::lfoWave());          // sine / triangle / square / S&H
    addInt   ("lfoDest", "LFO Destination", 0, 3, 1, &names::lfoDest());   // pitch / filter / volume / drive

    // ---- filter ----
    addInt   ("fltType", "Filter Type", 0, 3, 3, &names::filter());        // low-pass / band-pass / high-pass / off
    addFloat ("fltCutoff", "Filter", 60.0f, 18000.0f, 14000.0f, 0.3f, Unit::hertz);
    addFloat ("fltReso", "Filter Resonance", 0.5f, 8.0f, 0.9f, 0.5f, Unit::plain);
    addFloat ("fenvA", "Filter Attack", 0.5f, 2000.0f, 5.0f, 0.35f, Unit::milliseconds);
    addFloat ("fenvD", "Filter Decay", 1.0f, 4000.0f, 400.0f, 0.35f, Unit::milliseconds);
    addFloat ("fenvS", "Filter Sustain", 0.0f, 100.0f, 100.0f, 1.0f, Unit::percent);
    addFloat ("fenvR", "Filter Release", 1.0f, 5000.0f, 300.0f, 0.35f, Unit::milliseconds);

    // ---- the rest of the front panel ----
    addFloat ("velAmt", "Velocity", 0.0f, 100.0f, 60.0f, 1.0f, Unit::percent);
    addFloat ("pbend", "Pitch Bend", -2.0f, 2.0f, 0.0f, 1.0f, Unit::semitones);
    addFloat ("modDepth", "Modulation", 0.0f, 100.0f, 0.0f, 1.0f, Unit::percent);
    addInt   ("pickShape", "Pick Shape", 0, 3, 1, &names::pick());          // soft / round / bright / sharp

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