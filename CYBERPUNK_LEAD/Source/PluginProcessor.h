#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
#include "CpVoice.h"
#include "Presets.h"

// Synthesiser that also remembers the latest mod wheel (CC1) value when no voice is playing,
// so a note started later begins with the real wheel position instead of a stale/zero value.
struct CpModSynth : public juce::Synthesiser
{
    explicit CpModSynth (Cp::GlideState& s) : shared (s) {}
    void handleController (int ch, int num, int val) override
    {
        if (num == 1) shared.lastMod.store ((float) val / 127.f);
        else if (num == 121) shared.lastMod.store (0.f);   // reset all controllers
        juce::Synthesiser::handleController (ch, num, val);
    }
    Cp::GlideState& shared;
};

class CpProcessor : public juce::AudioProcessor
{
public:
    CpProcessor();
    void prepareToPlay (double sr, int block) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    using juce::AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "CYBERPUNK LEAD"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 12.0; }

    int getNumPrograms() override { return (int) Cp::presets().size(); }
    int getCurrentProgram() override { return juce::jmax (0, presetIndex); }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return i >= 0 && i < getNumPrograms() ? Cp::presets()[(size_t) i].name : "Init Patch"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void applyPreset (int index);
    juce::String currentPresetName() const { return presetName; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    int presetIndex = -1;
    juce::String presetName { "CYBERPUNK LEAD - Init Patch" };

private:
    struct StereoDelay
    {
        std::vector<float> bl, br; int w = 0; float smoothLen = 0.f;
        void prepare (double sr) { bl.assign ((size_t) (sr * 1.2) + 4, 0.f); br = bl; w = 0; smoothLen = (float) (sr * 0.3); }
        float read (const std::vector<float>& b, float len) const
        {
            const int n = (int) b.size(); float rp = (float) w - len; while (rp < 0) rp += (float) n;
            const int i0 = (int) rp % n, i1 = (i0 + 1) % n; const float f = rp - std::floor (rp);
            return b[(size_t) i0] * (1 - f) + b[(size_t) i1] * f;
        }
    };

    Cp::ParamPtrs ptrs;
    Cp::GlideState glideState;
    CpModSynth synth;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    StereoDelay dly;
    double sampleRate = 44100.0;
    int maxBlock = 512;   // largest block given to prepareToPlay (chorus is processed in chunks of at most this size)
    int lastMonoNote = -1;
    juce::MidiBuffer monoMidi;
    float dcXl = 0.f, dcYl = 0.f, dcXr = 0.f, dcYr = 0.f, dcRm = 0.999f;
    juce::SmoothedValue<float> distSm, delaySm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CpProcessor)
};
