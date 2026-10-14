#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Voice.h"
#include "DSP/SynthSound.h"
#include "DSP/Effects.h"

class K2000AudioProcessor : public juce::AudioProcessor
{
public:
    K2000AudioProcessor();
    ~K2000AudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Preset Data struct
    struct Preset
    {
        juce::String name;
        std::map<juce::String, float> params;
    };
    const std::vector<Preset>& getPresets() const { return factoryPresets; }
    void loadPreset(int index);

private:
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState apvts;

    // Master FX
    K2000Audio::DistortionEffect masterDistortion;
    K2000Audio::DimensionChorus chorus;
    K2000Audio::TapeDelay delay;
    K2000Audio::AlgorithmicReverb reverb;
    K2000Audio::MasterLimiter limiter;

    std::vector<Preset> factoryPresets;
    int currentProgramIndex = 0;

    void initPresets();
    void updateParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(K2000AudioProcessor)
};
