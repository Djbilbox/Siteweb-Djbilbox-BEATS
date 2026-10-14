#pragma once
#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "SolarParams.h"
#include "PitchDetector.h"
#include "PitchShifter.h"
#include "PitchCorrector.h"

/** HUMPIRE SOLAR - autotune temps reel (Auto + courbe graphique). */
class SolarAudioProcessor : public juce::AudioProcessor
{
public:
    SolarAudioProcessor();
    ~SolarAudioProcessor() override = default;

    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>& b, juce::MidiBuffer& m) override { processBlock (b, m); }

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HUMPIRE SOLAR"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.05; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    //==========================================================================
    // Courbe graphique (delta en demi-tons par classe de hauteur, 8 points / demi-ton)
    std::array<std::atomic<float>, solar::kGraphPoints> graphDelta;
    void commitGraphToState();          // thread UI : ecrit la courbe dans l'etat sauvegarde
    void loadGraphFromState();          // thread UI : relit la courbe depuis l'etat
    void resetGraph();
    void snapGraphToScale();

    // Donnees d'affichage (thread audio -> UI)
    static constexpr int historySize = 256;
    struct Frame { std::atomic<float> det { -1.0f }, out { -1.0f }; };
    std::array<Frame, historySize> history;
    std::atomic<int> historyHead { 0 };
    std::atomic<float> uiDetMidi { -1.0f }, uiTargetMidi { -1.0f }, uiOffset { 0.0f };
    std::atomic<float> inLevel { 0.0f }, outLevel { 0.0f };
    WaveformFifo waveform;

    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;
    PresetManager presets;

private:
    CorrectionParams readParams() const;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pMode, *pKey, *pScale, *pRetune, *pFlex, *pHumanize, *pAmount, *pRefA,
                       *pVibDepth, *pVibRate, *pVibDelay, *pPortamento, *pMix, *pIn, *pOut, *pBypass;
    std::array<std::atomic<float>*, 12> pNotes {};

    PitchDetector detector;
    PitchShifter shifter;
    PitchCorrector corrector;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, inSm, outSm;
    double sr = 44100.0;
    float smoothedSemis = 0.0f, vibPhase = 0.0f, vibEnv = 0.0f, ratioCoef = 0.01f, vibEnvCoef = 0.001f;
    float blockPeakIn = 0.0f;
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolarAudioProcessor)
};
