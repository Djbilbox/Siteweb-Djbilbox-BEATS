#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"
#include "KVoice.h"
#include "DSP/Effects.h"
#include "Presets.h"

class K2000AudioProcessor : public juce::AudioProcessor
{
public:
    K2000AudioProcessor();
    void prepareToPlay (double sr, int block) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "K2000"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return (int) K2k::presets().size(); }
    int getCurrentProgram() override { return juce::jmax (0, presetIndex); }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return i >= 0 && i < getNumPrograms() ? K2k::presets()[(size_t) i].name : "K2000 - Init Patch"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void applyPreset (int index);
    juce::String currentPresetName() const { return presetName; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    int presetIndex = -1;
    juce::String presetName { "K2000 - Init Patch" };

private:
    void allNotesOff();
    void monoOn (int note, float vel);
    void monoOff (int note);
    void handleEvent (const juce::MidiMessage&, int mode);

    K2k::ParamPtrs ptrs;
    K2k::SharedState shared;
    juce::Synthesiser synth;
    std::array<K2k::KVoice*, 16> voices {};
    int lastMode = -1;
    std::array<int, 32> noteStack {}; std::array<float, 32> velStack {}; int stackSize = 0;
    juce::MidiBuffer noMidi;

    K2000Audio::DistortionEffect masterDistortion;
    K2000Audio::DimensionChorus chorusFx;
    K2000Audio::TapeDelay delayFx;
    K2000Audio::AlgorithmicReverb reverbFx;
    K2000Audio::MasterLimiter limiter;
    juce::SmoothedValue<float> volSm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (K2000AudioProcessor)
};
