#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <HumpireCore/LcdScreen.h>
#include <HumpireCore/PresetManager.h>
#include "GlitchParams.h"
#include "GlitchDsp.h"

class GlitchAudioProcessor : public juce::AudioProcessor
{
public:
    GlitchAudioProcessor();
    ~GlitchAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HUMPIRE GLITCH"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- partage avec l'editeur (lock-free)
    juce::UndoManager undo;                          // doit preceder apvts
    juce::AudioProcessorValueTreeState apvts;
    humpire::PresetManager presets;
    std::atomic<float> inPeak { 0.0f }, outPeak { 0.0f };
    humpire::ScopeBuffer scope;

private:
    glitch::GlitchDsp dsp;
    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pBits, *pRate, *pDrive, *pSpeaker, *pTone, *pDither, *pMix, *pOut;
    int scopeStride = 960;
    bool prepared = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlitchAudioProcessor)
};
