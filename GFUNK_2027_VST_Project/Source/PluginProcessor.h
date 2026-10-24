#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <vector>
#include "Generated/GFunkSpec.h"

// GFUNK 2027 - HUMPIRE by Djbilbox BEATS
// 8-voice West Coast G-Funk synthesizer: 2 oscillators + sub + noise with unison,
// multimode filter, 2 envelopes, 2 LFOs, arpeggiator, tape, phaser, chorus, delay, reverb.
class GFunkProcessor : public juce::AudioProcessor
{
public:
    GFunkProcessor();
    ~GFunkProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    #ifdef GFUNK_FREE
    const juce::String getName() const override { return "GFUNK 2027 Free"; }
#else
    const juce::String getName() const override { return "GFUNK 2027"; }
#endif
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return GFunkSpec::numPresets; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::String formatValue (int paramIndex, float value);

    // Data for the user interface (all lock-free)
    void copyScope (float* dest) const;     // 1024 decimated samples, oldest first
    void copyFft (float* dest) const;       // 2048 full-rate samples, oldest first
    float getOutputLevel() const { return outputLevel.load(); }
    bool isNoteLit (int note) const
    {
        return note >= 0 && note < 128
               && ((note < 64 ? activeLo.load() : activeHi.load()) >> (note & 63)) & 1ull;
    }

    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyState;

    std::atomic<float> funk { 0.0f };       // "how funky are you playing", 0..1
    std::atomic<float> hit { 0.0f };        // flash on every note, decays
    std::atomic<float> uiPitchBend { 0.0f }, uiModWheel { 0.0f };
    std::atomic<int> blingTheme { 0 };      // 0 gold, 1 candy, 2 neon, 3 all
    std::atomic<int> blingLevel { 1 };      // 0 off, 1 normal, 2 max
    std::atomic<int> skin { 0 };            // interface skin (0..5)
    std::atomic<int> scrollInvert { 1 };    // browser wheel direction

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    struct Env
    {
        enum St { Idle, Atk, Dec, Sus, Rel } st = Idle;
        float v = 0.0f;
    };
    struct EnvCoef { float atk, dec, sus, rel; };
    static float tickEnv (Env& e, const EnvCoef& c);

    struct Svf { float ic1 = 0.0f, ic2 = 0.0f; };
    struct Voice
    {
        bool active = false, gate = false;
        int note = -1;
        unsigned age = 0;
        float vel = 0.0f, freq = 440.0f, target = 440.0f, subPh = 0.0f;
        float ph1[7] {}, ph2[7] {};
        Env amp, flt;
        Svf f[2][2];
        float a1 = 0, a2 = 0, a3 = 0, k = 1, b1 = 0, b2 = 0, b3 = 0;
        unsigned rng = 12345u;
    };

    struct Params
    {
        float v[GFunkSpec::NumParams];
        float operator[] (int i) const { return v[i]; }
    };

    void handleMidi (const juce::MidiMessage& m);
    void noteOnInput (int note, float vel);
    void noteOffInput (int note);
    void voiceNoteOn (int note, float vel);
    void voiceNoteOff (int note);
    void startVoice (Voice& v, int note, float vel, bool retrigger);
    void releaseVoice (Voice& v);
    void allNotesOff();
    void arpNoteOn (int note, float vel);
    void arpNoteOff (int note);
    void arpTick();
    void renderFx (juce::AudioBuffer<float>& buffer, int n);

    Params P {};
    std::array<Voice, 8> voices;
    std::vector<int> heldNotes, pendingOff, arpHeld, arpSeq;
    bool sustainDown = false, arpDirty = true;
    unsigned ageCounter = 0;
    float midiBend = 0.0f, midiMod = 0.0f;
    float glideCoef = 0.0f;
    int curMode = 0, curArp = 0, lastArp = 0;
    int arpStep = 0, arpCounter = 0, arpGateCount = 0, arpPlaying = -1, arpStepLen = 12000;
    float arpVel = 0.8f;
    EnvCoef ampC {}, fltC {};
    float uniMul[7] {}, uniL[7] {}, uniR[7] {};
    int uniN = 1;
    float lfo1Phase = 0.0f, lfo2Phase = 0.0f, shValue = 0.0f;
    juce::Random rnd;

    // FX state
    std::array<std::vector<float>, 2> wobbleBuf;
    int wobbleWrite = 0;
    float wobPhase = 0.0f, flutterPhase = 0.0f, noiseState = 0.0f;
    float phPhase = 0.0f, apZ[2][4] {}, apFb[2] {};
    std::array<std::vector<float>, 2> dlyBuf;
    int dlyWrite = 0;
    float dlyCur = 12000.0f, dlyLp[2] {}, toneLp[2] {};
    double bpm = 120.0;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;

    // UI feeds
    static constexpr int scopeSize = 2048, fftSize = 2048;
    std::array<float, scopeSize> scopeRing {};
    std::array<float, fftSize> fftRing {};
    std::atomic<int> scopeWrite { 0 }, fftWrite { 0 };
    float scopeAcc = 0.0f;
    int scopeCount = 0;
    std::atomic<float> outputLevel { 0.0f };
    std::atomic<uint64_t> activeLo { 0 }, activeHi { 0 };
    float funkEnergy = 0.0f, funkAcc = 0.0f, hitLocal = 0.0f;

    std::atomic<float>* raw[GFunkSpec::NumParams] {};
    double sr = 44100.0;
    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GFunkProcessor)
};
