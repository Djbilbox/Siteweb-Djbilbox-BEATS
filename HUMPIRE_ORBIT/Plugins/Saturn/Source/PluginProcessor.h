#pragma once
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "SaturnParams.h"
#include "HarmonyEngine.h"

/** HUMPIRE SATURN - four-voice harmony generator (semitone or scale-degree intervals, maqam scales). */
class SaturnAudioProcessor : public juce::AudioProcessor
{
public:
    SaturnAudioProcessor();
    ~SaturnAudioProcessor() override = default;

    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HUMPIRE SATURN"; }
    bool acceptsMidi() const override { return false; }
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

    //==========================================================================
    // Display data (audio thread -> UI, lock-free)
    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f };
    std::atomic<float> uiDetMidi { -1.0f };                       // detected input pitch (MIDI-like), -1 = unvoiced
    std::atomic<float> uiTrackedMidi { -1.0f };                   // tracked scale note (MIDI-like), -1 = none
    std::array<std::atomic<float>, saturn::kNumVoices> uiVoiceShift {}, uiVoiceLevel {};
    WaveformFifo waveform;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

private:
    void processInternal (juce::AudioBuffer<float>&, bool hostBypassed);
    void setupSmoothers (double sampleRate);
    saturn::EngineSettings readSettings() const noexcept;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pMode = nullptr, *pKey = nullptr, *pScale = nullptr, *pGlide = nullptr,
                       *pMix = nullptr, *pIn = nullptr, *pOut = nullptr, *pBypass = nullptr;
    std::array<std::array<std::atomic<float>*, saturn::kNumVoiceParams>, saturn::kNumVoices> pVoice {};

    saturn::HarmonyEngine engine;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, inSm, outSm;
    double sr = 44100.0;
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnAudioProcessor)
};
