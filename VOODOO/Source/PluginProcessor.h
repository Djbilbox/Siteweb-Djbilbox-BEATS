#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"
#include "VoodooVoice.h"
#include "Presets.h"

class VoodooProcessor : public juce::AudioProcessor
{
public:
    VoodooProcessor();
    void prepareToPlay (double sr, int block) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "VOODOO"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return (int) Voodoo::presets().size(); }
    int getCurrentProgram() override { return juce::jmax (0, presetIndex); }
    void setCurrentProgram (int i) override { applyPreset (i); }
    const juce::String getProgramName (int i) override { return i >= 0 && i < getNumPrograms() ? Voodoo::presets()[(size_t) i].name : "Init Patch"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    void applyPreset (int index);
    juce::String currentPresetName() const { return presetName; }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    int presetIndex = -1;
    juce::String presetName { "VOODOO - Init Patch" };

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

    Voodoo::ParamPtrs ptrs;
    Voodoo::GlideState glideState;
    juce::Synthesiser synth;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    StereoDelay dly;
    double sampleRate = 44100.0;
    int lastMonoNote = -1;
    juce::SmoothedValue<float> gainSm;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoodooProcessor)
};
