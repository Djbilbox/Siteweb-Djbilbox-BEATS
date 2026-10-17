#pragma once
#include <array>
#include <algorithm>
#include <atomic>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Core/PresetManager.h"
#include "Core/WaveformDisplay.h"
#include "SaturnParams.h"
#include "HarmonyEngine.h"

namespace saturn
{
    /** Stereo integer delay of the bypass path: the untouched input, delayed by exactly the announced latency,
        so that bypassing does not shift the audio against the rest of the project. Allocates in prepare() only. */
    class BypassDelay
    {
    public:
        void prepare (int delaySamples)
        {
            delay = std::max (0, delaySamples);
            int sz = 2;
            while (sz < delay + 2) sz <<= 1;
            mask = sz - 1;
            for (auto& b : buf) b.assign ((size_t) sz, 0.0f);
            wr = 0;
        }

        void reset() noexcept
        {
            for (auto& b : buf) std::fill (b.begin(), b.end(), 0.0f);
            wr = 0;
        }

        /** Pushes one stereo frame and returns the frame pushed `delay` samples ago. */
        void process (float inL, float inR, float& outL, float& outR) noexcept
        {
            buf[0][(size_t) wr] = inL;
            buf[1][(size_t) wr] = inR;
            const size_t rd = (size_t) ((wr - delay) & mask);
            outL = buf[0][rd];
            outR = buf[1][rd];
            wr = (wr + 1) & mask;
        }

    private:
        std::array<std::vector<float>, 2> buf;
        int delay = 0, mask = 1, wr = 0;
    };
}

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
    double getTailLengthSeconds() const override { return 0.05; }   // the delayed signal drains from the shifter
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
    void resetDisplayData() noexcept;
    saturn::EngineSettings readSettings() const noexcept;

    juce::AudioParameterBool* bypassParam = nullptr;
    std::atomic<float> *pMode = nullptr, *pKey = nullptr, *pScale = nullptr, *pGlide = nullptr,
                       *pMix = nullptr, *pIn = nullptr, *pOut = nullptr, *pBypass = nullptr;
    std::array<std::array<std::atomic<float>*, saturn::kNumVoiceParams>, saturn::kNumVoices> pVoice {};

    saturn::HarmonyEngine engine;
    saturn::BypassDelay bypassDelay;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSm, inSm, outSm, bypassSm;
    double sr = 44100.0;
    int waveCounter = 0;
    float wavePeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SaturnAudioProcessor)
};
