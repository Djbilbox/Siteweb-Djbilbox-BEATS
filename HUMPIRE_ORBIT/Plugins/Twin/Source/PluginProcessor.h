#pragma once
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "TwinParams.h"
#include "TwinDoubler.h"

/** HUMPIRE TWIN - auto-doubler vocal (1 a 4 doublures, micro-detune, micro-delai humanise, largeur, filtre). */
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
    void processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&) override         { process (b, false); }
    void processBlockBypassed (juce::AudioBuffer<float>& b, juce::MidiBuffer&) override { process (b, true); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HUMPIRE TWIN"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.08; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==========================================================================
    // Donnees d'affichage (thread audio -> UI, lock-free)
    struct VoiceUi
    {
        std::atomic<float> delayMs { 0.0f }, cents { 0.0f }, pan { 0.0f }, env { 0.0f }, level { 0.0f };
    };
    std::array<VoiceUi, twin::kMaxVoices> voiceUi;
    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f }, correlation { 1.0f };
    WaveformFifo waveform;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

private:
    void prepareInternal (double sampleRate);
    void process (juce::AudioBuffer<float>&, bool forceBypass) noexcept;
    twin::DoublerEngine::Settings readSettings() const noexcept;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pVoices = nullptr, *pDetune = nullptr, *pDelay = nullptr, *pSpread = nullptr, *pHumanize = nullptr,
                       *pDrift = nullptr, *pWidth = nullptr, *pHpf = nullptr, *pLpf = nullptr, *pMix = nullptr,
                       *pIn = nullptr, *pOut = nullptr, *pBypass = nullptr;

    twin::DoublerEngine engine;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inSm, outSm, drySm, wetSm, activeSm;
    double sr = 44100.0;
    double corrLR = 0.0, corrLL = 0.0, corrRR = 0.0;
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwinAudioProcessor)
};
