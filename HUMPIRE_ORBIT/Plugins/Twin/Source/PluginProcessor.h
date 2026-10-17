#pragma once
#include <atomic>
#include <cstdint>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "TwinParams.h"
#include "TwinDoubler.h"

/** HUMPIRE TWIN - vocal auto-doubler (1 to 4 detuned, delayed and panned doubles). */
class TwinAudioProcessor : public juce::AudioProcessor
{
public:
    TwinAudioProcessor();
    ~TwinAudioProcessor() override = default;

    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;

    /** Handles bypass itself: because getBypassParameter() is exposed, the VST3 wrapper always
        calls processBlock and never processBlockBypassed. */
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>& b, juce::MidiBuffer& m) override { processBlock (b, m); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HUMPIRE TWIN"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }   // longest delay (40 ms) + range + filters
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    /** UI thread: asks the audio thread to re-seed the random humanize generators (lock-free). */
    void requestReseed() noexcept { reseedCounter.fetch_add (1u, std::memory_order_relaxed); }

    // Audio thread -> UI
    twin::UiState ui;
    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f };
    WaveformFifo waveform;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

private:
    twin::DoublerSettings readSettings() const noexcept;
    float targetMix() const noexcept;
    float targetOutGain() const noexcept;
    float targetBypass() const noexcept;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pDoubles = nullptr, *pDetune = nullptr, *pDelay = nullptr, *pHumanize = nullptr,
                       *pDrift = nullptr, *pWidth = nullptr, *pHp = nullptr, *pLp = nullptr,
                       *pMix = nullptr, *pOutGain = nullptr, *pBypass = nullptr;

    twin::Doubler doubler;
    /** mix / output gain / "processed vs dry" (bypass) smoothers; bypass = 0 means untouched input. */
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, outSm, activeSm;
    std::atomic<std::uint32_t> reseedCounter { 0u };
    std::uint32_t lastReseed = 0u;
    double sr = 44100.0;
    std::atomic<bool> prepared { false };
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwinAudioProcessor)
};
