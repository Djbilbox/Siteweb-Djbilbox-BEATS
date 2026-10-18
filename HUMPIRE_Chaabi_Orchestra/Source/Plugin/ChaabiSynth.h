#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <memory>
#include "SampleBank.h"
#include "Instruments.h"

namespace humpire
{
// Parameters shared instrument -> voices (written by the audio thread at the start of each block).
struct SynthParams
{
    std::atomic<float> pitchSemis { 0.f }, attack { 0.f }, release { 0.f }, authenticity { 0.f }, bendRange { 2.f };
};

// Sampler voice: Hermite interpolation, loop, ADSR, pan, humanisation.
class ChaabiVoice : public juce::SynthesiserVoice
{
public:
    explicit ChaabiVoice (const SynthParams& p);

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<Region*> (s) != nullptr; }
    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int pitchWheel) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int newValue) override;
    void controllerMoved (int, int) override {}
    void aftertouchChanged (int v) override { pressureGain = 1.0f + 0.5f * (float) v / 127.0f; }
    void channelPressureChanged (int v) override { pressureGain = 1.0f + 0.5f * (float) v / 127.0f; }
    using juce::SynthesiserVoice::renderNextBlock;
    void renderNextBlock (juce::AudioBuffer<float>&, int startSample, int numSamples) override;
    void setCurrentPlaybackSampleRate (double newRate) override;

    void detach() noexcept { region = nullptr; data = nullptr; }      // before a bank change
    void quickRelease();                                              // off_by / group
    const Region* getRegion() const noexcept { return region; }

private:
    void finish();

    const SynthParams& sp;
    Region* region = nullptr;
    const SampleData* data = nullptr;
    juce::ADSR env;
    juce::Random rng;

    double pos = 0.0, step = 1.0, endPos = 0.0, loopStart = 0.0, loopEndF = 0.0;
    double bendRatio = 1.0;
    bool looping = false, oneShot = false, released = false;
    float gain = 1.f, gL = 1.f, gR = 1.f, pressureGain = 1.f;
    int delayLeft = 0;
};

// Synthesiser: one per instrument. SFZ region selection (RR, velocity, keyswitch, release triggers).
class ChaabiSynth : public juce::Synthesiser
{
public:
    ChaabiSynth();

    void noteOn (int channel, int note, float velocity) override;
    void noteOff (int channel, int note, float velocity, bool allowTailOff) override;

    // Audio thread only: stops the voices, installs the new bank, returns the old one.
    std::shared_ptr<Bank> swapBank (std::shared_ptr<Bank> newBank);

    void setPolyLimit (int n) noexcept { polyLimit = juce::jlimit (1, kMaxVoices, n); }
    SynthParams params;

private:
    void startRegion (Region*, int channel, int note, float velocity);
    ChaabiVoice* pickVoice();

    std::shared_ptr<Bank> bank;
    int polyLimit = 32;
    int activeSwitch = -1;
    int seqCounter[128] {};
    float lastVelocity[128] {};
    juce::Random rng;
};
} // namespace humpire
