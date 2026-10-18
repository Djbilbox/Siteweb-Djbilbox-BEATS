#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Instruments.h"

// Parameter identifiers + APVTS layout (every parameter goes through the APVTS).
namespace humpire::pid
{
inline const juce::String selected   { "selected" };
inline const juce::String multi      { "multi" };
inline const juce::String poly       { "poly" };
inline const juce::String authent    { "authenticity" };
inline const juce::String master     { "master" };
inline const juce::String bypass     { "bypass" };
inline const juce::String revSize    { "rev_size" };
inline const juce::String revDamp    { "rev_damp" };
inline const juce::String dlyTime    { "dly_time" };
inline const juce::String dlyFb      { "dly_fb" };
inline const juce::String tempo      { "tempo" };
inline const juce::String bendRange  { "bend_range" };
inline const juce::String seqOn      { "seq_on" };
inline const juce::String seqSwing   { "seq_swing" };
inline const juce::String seqRate    { "seq_rate" };

// State properties (not parameters)
inline const juce::Identifier seqGrid     { "seqGrid" };
inline const juce::Identifier libraryRoot { "libraryRoot" };
inline juce::Identifier sfzOverride (int i) { return juce::Identifier ("sfz_" + juce::String (i)); }

inline juce::String inst (int i, const char* suffix) { return "i" + juce::String (i) + "_" + suffix; }

inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    using namespace juce;
    using F = AudioParameterFloat;
    using FA = AudioParameterFloatAttributes;
    AudioProcessorValueTreeState::ParameterLayout l;

    StringArray names;
    for (int i = 0; i < kNumInstruments; ++i) names.add (instrumentName (i));

    l.add (std::make_unique<AudioParameterChoice> (ParameterID { selected, 1 }, "Instrument", names, 0));
    l.add (std::make_unique<AudioParameterBool>   (ParameterID { multi, 1 }, "Multi Mode", false));
    l.add (std::make_unique<AudioParameterInt>    (ParameterID { poly, 1 }, "Polyphony", 8, kMaxVoices, 32));
    l.add (std::make_unique<F> (ParameterID { authent, 1 }, "Authenticity", NormalisableRange<float> (0.f, 1.f, 0.001f), 0.25f,
                                FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 100.f)) + " %"; })));
    l.add (std::make_unique<F> (ParameterID { master, 1 }, "Master", NormalisableRange<float> (-24.f, 6.f, 0.01f), 0.f,
                                FA().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dB"; })));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { bypass, 1 }, "Bypass", false));
    l.add (std::make_unique<F> (ParameterID { revSize, 1 }, "Reverb Size", NormalisableRange<float> (0.f, 1.f, 0.001f), 0.55f));
    l.add (std::make_unique<F> (ParameterID { revDamp, 1 }, "Reverb Damping", NormalisableRange<float> (0.f, 1.f, 0.001f), 0.45f));
    l.add (std::make_unique<F> (ParameterID { dlyTime, 1 }, "Delay Time", NormalisableRange<float> (40.f, 1000.f, 1.f, 0.5f), 330.f,
                                FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v)) + " ms"; })));
    l.add (std::make_unique<F> (ParameterID { dlyFb, 1 }, "Delay Feedback", NormalisableRange<float> (0.f, 0.9f, 0.001f), 0.35f));
    l.add (std::make_unique<F> (ParameterID { tempo, 1 }, "Tempo (standalone)", NormalisableRange<float> (50.f, 200.f, 0.1f), 90.f,
                                FA().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " bpm"; })));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { bendRange, 1 }, "Pitch Bend", 1, 12, 2));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { seqOn, 1 }, "Sequencer", false));
    l.add (std::make_unique<F> (ParameterID { seqSwing, 1 }, "Swing", NormalisableRange<float> (0.f, 0.5f, 0.001f), 0.f));
    l.add (std::make_unique<AudioParameterChoice> (ParameterID { seqRate, 1 }, "Step Rate", StringArray { "1/8", "1/16" }, 1));

    auto noteText = [] (int v, int) { return MidiMessage::getMidiNoteName (v, true, true, 5) + " (" + String (v) + ")"; };
    auto noteFrom = [] (const String& t) { return t.fromLastOccurrenceOf ("(", false, false).getIntValue(); };

    for (int i = 0; i < kNumInstruments; ++i)
    {
        const String n = instrumentName (i) + " ";
        const auto& info = kInstruments[i];
        l.add (std::make_unique<F> (ParameterID { inst (i, "vol"), 1 }, n + "Volume", NormalisableRange<float> (-60.f, 12.f, 0.01f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return v <= -59.9f ? String ("-inf") : String (v, 1) + " dB"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "pan"), 1 }, n + "Pan", NormalisableRange<float> (-1.f, 1.f, 0.001f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return std::abs (v) < 0.01f ? String ("C") : String (juce::roundToInt (std::abs (v) * 100.f)) + (v < 0 ? " L" : " R"); })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "rev"), 1 }, n + "Reverb", NormalisableRange<float> (0.f, 1.f, 0.001f), info.percussive ? 0.10f : 0.18f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 100.f)) + " %"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "dly"), 1 }, n + "Delay", NormalisableRange<float> (0.f, 1.f, 0.001f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 100.f)) + " %"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "filter"), 1 }, n + "Filter", NormalisableRange<float> (80.f, 20000.f, 1.f, 0.25f), 20000.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v)) + " Hz"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "pitch"), 1 }, n + "Pitch", NormalisableRange<float> (-12.f, 12.f, 0.01f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (v, 2) + " st"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "atk"), 1 }, n + "Attack", NormalisableRange<float> (0.f, 2.f, 0.001f, 0.35f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 1000.f)) + " ms"; })));
        l.add (std::make_unique<F> (ParameterID { inst (i, "rel"), 1 }, n + "Release", NormalisableRange<float> (0.f, 4.f, 0.001f, 0.35f), 0.f,
                                    FA().withStringFromValueFunction ([] (float v, int) { return String (juce::roundToInt (v * 1000.f)) + " ms"; })));
        l.add (std::make_unique<AudioParameterInt> (ParameterID { inst (i, "lo"), 1 }, n + "Zone Start", 0, 127, info.defaultLo,
                                                    AudioParameterIntAttributes().withStringFromValueFunction (noteText).withValueFromStringFunction (noteFrom)));
        l.add (std::make_unique<AudioParameterInt> (ParameterID { inst (i, "hi"), 1 }, n + "Zone End", 0, 127, info.defaultHi,
                                                    AudioParameterIntAttributes().withStringFromValueFunction (noteText).withValueFromStringFunction (noteFrom)));
        l.add (std::make_unique<AudioParameterBool> (ParameterID { inst (i, "on"), 1 }, n + "Active (multi)", i == 0));
    }
    return l;
}
} // namespace humpire::pid
