#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Params/Params.h"
#include "Params/PresetData.h"
#include "Params/ExpansionPack.h"
#include "DSP/SynthEngine.h"

class WarzoneProcessor : public juce::AudioProcessor
{
public:
    struct Entry { juce::String bank, name; std::vector<std::pair<juce::String, float>> v; };

    WarzoneProcessor();
    void prepareToPlay (double sr, int block) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "WARZONE808"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return (int) entries.size(); }
    int getCurrentProgram() override { return juce::jmax (0, presetIndex); }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return i >= 0 && i < (int) entries.size() ? entries[(size_t) i].name : "Init Patch"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void applyPreset (int index);
    void loadExpansion (const ExpansionPack& pack);
    bool loadExpansionFile (const juce::File& f);
    static juce::File userDir (const juce::String& sub);

    const std::vector<Entry>& getEntries() const { return entries; }
    juce::String expansionName { "Expansion Vol.1" };

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    int presetIndex = -1;
    juce::String presetName { "WARZONE808 - Init Patch" };

private:
    void syncMode();
    void pushParamsToEngine (int numSamples);
    void handleMidi (const juce::MidiMessage&);

    Wz::ParamPtrs ptrs;
    SynthEngine engine;
    std::vector<Entry> entries;
    std::vector<int> sustainedNotes;   // note-off differes par la pedale
    bool sustainDown = false, lastPoly = false;
    float midiBend = 0.f, midiMod = 0.f;
    juce::SmoothedValue<float> volSm, cutSm, pitchSm;
    juce::MidiBuffer keyMidi;
    float dcXl = 0.f, dcYl = 0.f, dcXr = 0.f, dcYr = 0.f, dcR = 0.999f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarzoneProcessor)
};
