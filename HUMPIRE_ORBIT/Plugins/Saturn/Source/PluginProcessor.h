#pragma once
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "SaturnParams.h"
#include "HarmonyEngine.h"

/** HUMPIRE SATURN - generateur d'harmonies 4 voix (demi-tons ou degres de gamme, maqams inclus). */
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
    void processBlockBypassed (juce::AudioBuffer<float>& b, juce::MidiBuffer& m) override { processBlock (b, m); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HUMPIRE SATURN"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.1; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==========================================================================
    // Donnees d'affichage (thread audio -> UI)
    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f };
    std::atomic<float> uiInMidi { -1.0f };          // hauteur detectee (MIDI), -1 = non voise
    std::atomic<float> uiTrackedNote { -1.0f };     // degre de gamme suivi (MIDI)
    std::array<std::atomic<float>, saturn::kNumVoices> uiVoiceShift, uiVoiceLevel;
    WaveformFifo waveform;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

private:
    saturn::EngineSettings readSettings() const noexcept;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pMode = nullptr, *pKey = nullptr, *pScale = nullptr, *pFormant = nullptr, *pGlide = nullptr,
                       *pCorrection = nullptr, *pHumanize = nullptr, *pWidth = nullptr, *pMix = nullptr,
                       *pIn = nullptr, *pOut = nullptr, *pBypass = nullptr;
    std::array<std::array<std::atomic<float>*, saturn::kNumVoiceParams>, saturn::kNumVoices> pVoice {};

    saturn::HarmonyEngine engine;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, inSm, outSm;
    double sr = 48000.0;
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnAudioProcessor)
};
