#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Params.h"
#include "KVoice.h"
#include "DSP/Effects.h"
#include "Presets.h"

class NeonAudioProcessor : public juce::AudioProcessor
{
public:
    NeonAudioProcessor();
    void prepareToPlay (double sr, int block) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "NEON SYNTH 80s 2.0"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return (int) Neon::presets().size(); }
    int getCurrentProgram() override { return juce::jmax (0, presetIndex); }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return i >= 0 && i < getNumPrograms() ? Neon::presets()[(size_t) i].name : "Init Patch"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void applyPreset (int index);
    juce::String currentPresetName() const { return presetName; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    int presetIndex = -1;
    juce::String presetName { "Init Patch" };

    // derniers echantillons de sortie (mono) pour l'oscilloscope de l'interface ; ecrit par le thread audio, lu par le timer UI
    static constexpr int scopeSize = 1024;
    std::array<float, scopeSize> scope {};
    std::atomic<int> scopeWrite { 0 };
    std::atomic<float> peakL { 0.f }, peakR { 0.f };   // crete de sortie depuis la derniere lecture UI (vu-metre)

private:
    void allNotesOff();
    void monoOn (int note, float vel);
    void monoOff (int note);
    void handleEvent (const juce::MidiMessage&, int mode);

    Neon::ParamPtrs ptrs;
    Neon::SharedState shared;
    juce::Synthesiser synth;
    std::array<Neon::KVoice*, 16> voices {};
    int lastMode = -1;
    bool prepared = false;   // set by prepareToPlay: the voices and effects need a valid sample rate before any audio is rendered
    std::array<int, 32> noteStack {}; std::array<float, 32> velStack {}; int stackSize = 0;
    juce::MidiBuffer noMidi;

    NeonFx::DistortionEffect masterDistortion;
    NeonFx::DimensionChorus chorusFx;
    NeonFx::TapeDelay delayFx;
    NeonFx::AlgorithmicReverb reverbFx;
    NeonFx::MasterLimiter limiter;
    juce::SmoothedValue<float> volSm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NeonAudioProcessor)
};
