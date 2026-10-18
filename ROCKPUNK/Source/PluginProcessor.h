#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>
#include <memory>
#include <set>
#include <vector>

#include "Dsp/Amp.h"
#include "Dsp/Delay.h"
#include "Dsp/Drive.h"
#include "Dsp/GuitarVoice.h"
#include "Dsp/SampleBank.h"
#include "Dsp/SamplerVoice.h"
#include "Dsp/_archive_synth_engine/Filter.h"
#include "Dsp/_archive_synth_engine/Lfo.h"
#include "Params.h"

namespace rk
{

class PluginProcessor final : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    // The output is a single stereo bus and there is no input.
    bool isBusesLayoutSupported (const BusesLayout&) const override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    using juce::AudioProcessor::processBlock;
    using juce::AudioProcessor::processBlockBypassed;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // Host bypass. An instrument has no audio to pass through, but it must still
    // stop sounding and forget its notes: the note-offs that arrive while it is
    // bypassed are never delivered, and would leave notes hanging afterwards.
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ROCKPUNK GUITAR"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }

    // Delay feedback and reverb keep ringing after the last note.
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    GuitarParams params;

    // ---- preset library ----
    // Shared by the preset display and the cassette menu.
    juce::File getPresetDirectory() const;
    void seedFactoryPresets() const;
    void refreshPresetList();
    const juce::Array<juce::File>& getPresetFiles() const noexcept { return presetFiles; }
    juce::String getCurrentPresetName() const;
    int getCurrentPresetIndex() const;
    bool loadPreset (int index);
    bool loadPreset (const juce::File&);
    void stepPreset (int delta);
    bool savePreset (const juce::File&);
    void initPatch();                       // every parameter back to its default
    std::function<void()> onPresetListChanged;
    void prepareUserSampleFolder() const;

    // ---- sampled layer (the user's own files) ----
    // Re-reads the user's sample folder. The editor calls it when its menu opens,
    // so a file dropped during the session shows up without reloading the plug-in.
    void rescanSamples();
    juce::File getUserSampleDirectory() const { return SampleBank::getUserSampleDirectory(); }
    bool isUsingUserFolder() const { return bank.isUserFolder(); }

    juce::StringArray getSampleFiles() const { return bank.getNames(); }
    int getNumSampleFiles() const { return bank.getNumEntries(); }
    juce::String getCurrentSampleName() const;
    float getSampleRootNote() const;
    juce::String getSampleCategory (int index) const
    {
        SampleBank::Entry e;
        return bank.getEntryInfo (index, e) ? e.category : juce::String();
    }

    // Loads the selected file if it changed. Runs on the background loader
    // thread (and on the message thread while preparing), never on the audio
    // thread: decoding a file there would drop out the audio.
    void pollSampleSelection();

    // How many times the safety net had to throw away a non-finite sample. It
    // must stay at zero: the test bench reads it.
    int getNonFiniteEventCount() const noexcept { return nonFiniteEvents.load(); }

    // ---- UI keyboard / note tracking ----
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }
    std::set<int> getActiveNotesSnapshot() const;

private:
    // Longest stretch rendered with one set of control values. Vibrato, LFO and
    // pitch follow the controls at this rate whatever buffer size the host picks.
    static constexpr int kSubBlock = 64;
    static constexpr int kHostMonoChunk = 2048;

    void setupEngine (double sampleRate);
    void silenceEngine();
    void renderBlock (float* left, float* right, int numSamples, juce::MidiBuffer& midi);
    void updateVoices();
    void scanMidiControllers (const juce::MidiBuffer&);
    void trackNotes (const juce::MidiBuffer&);
    void applyMonoMode (juce::MidiBuffer&);
    void applyMasterFx (float* left, float* right, int numSamples);
    static void sliceMidi (const juce::MidiBuffer& src, juce::MidiBuffer& dst, int start, int length,
                           bool isFirst, bool isLast);

    void loadSample();
    void updateSamplerVoices();
    bool anyNoteActive() const noexcept
    {
        return noteBits[0].load (std::memory_order_relaxed) != 0 || noteBits[1].load (std::memory_order_relaxed) != 0;
    }
    void setNoteBit (int note, bool on) noexcept;

    // Two separate Synthesiser objects rather than one carrying two sounds.
    //
    // juce::Synthesiser::noteOn loops over the sounds; for each one it starts by
    // stopping any voice already playing that note, without checking that it
    // belongs to the same sound. With strings and sampler in the same Synthesiser,
    // the second sound cut the string at the very instant the first one started
    // it.
    juce::Synthesiser synth;         // modelled strings
    juce::Synthesiser sampleSynth;   // sampled layer
    std::vector<GuitarVoice*> guitarVoices;
    std::vector<SamplerVoice*> samplerVoices;

    Amp   amp;
    Drive drive;
    Delay delay;

    juce::Reverb reverb;
    std::vector<float> chorusL, chorusR;
    int   chorusWrite = 0;
    float chorusPhase = 0.0f;

    // The rate retained by prepareToPlay. AudioProcessor::getSampleRate() is only
    // set by the host, so it reads 0 outside one.
    double preparedRate = 48000.0;

    // DC blocker after the amp and the drive: both push the positive half harder
    // than the negative one, which produces a DC component that eats headroom
    // before the limiter and goes through the filter untouched.
    float dcInL = 0.0f, dcOutL = 0.0f, dcInR = 0.0f, dcOutR = 0.0f;
    float dcCoeff = 0.9974f;

    Lfo lfo;
    SvfFilter filterL, filterR;
    juce::ADSR filterEnv;
    bool anyNoteHeld = false;
    float lfoValue = 0.0f;

    // Controller state kept at processor level: a mod wheel or pitch wheel moved
    // before a note starts must still apply to it.
    float midiMod = 0.0f;       // CC1, 0..1
    float midiBend = 0.0f;      // pitch wheel, -1..1 (two semitones at full travel)

    // ---- sampled layer ----
    // `bank`, the published sample and its description are shared between the
    // message thread, the loader thread and (through a try-lock) the audio thread.
    SampleBank bank;
    mutable juce::CriticalSection sampleLock;
    juce::CriticalSection loadLock;
    std::shared_ptr<juce::AudioBuffer<float>> sampleData;
    std::vector<std::shared_ptr<juce::AudioBuffer<float>>> sampleHistory;   // keeps old buffers alive off the audio thread
    double sampleFileRate = 44100.0;
    float  sampleBaseRoot = 60.0f;
    float  sampleRootNote = 60.0f;
    juce::String currentSampleName;
    int sampleVersion = 0;
    std::atomic<int> lastSampleSel { -1 };
    std::atomic<int> lastSampleRoot { 0 };

    std::shared_ptr<juce::AudioBuffer<float>> audioSample;   // audio-thread copy
    double audioFileRate = 44100.0;
    float  audioRoot = 60.0f;
    int    audioSampleVersion = -1;

    struct SampleLoader final : public juce::Thread
    {
        explicit SampleLoader (PluginProcessor& p) : juce::Thread ("ROCKPUNK sample loader"), owner (p) {}
        void run() override
        {
            while (! threadShouldExit())
            {
                owner.pollSampleSelection();
                wait (60);
            }
        }
        PluginProcessor& owner;
    };
    SampleLoader loader { *this };

    // ---- presets ----
    juce::Array<juce::File> presetFiles;
    int currentPreset = -1;

    // ---- notes ----
    juce::MidiKeyboardState keyboardState;
    std::array<std::atomic<juce::uint64>, 2> noteBits {};
    int monoNote = -1;
    juce::MidiBuffer chunkMidi, monoMidi, hostSliceMidi;
    std::vector<float> hostMonoL, hostMonoR;
    bool bypassFlushed = false;
    std::atomic<int> nonFiniteEvents { 0 };

    juce::SmoothedValue<float> masterVolSmooth;
    juce::SmoothedValue<float> widthSmooth;
    juce::SmoothedValue<float> lfoGainSmooth;
    juce::SmoothedValue<float> ampGainSmooth, ampDistSmooth, ampLevelSmooth, driveSmooth;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> cutoffSmooth;

    float currentBpm = 120.0f;
    float delayTimeMs = 350.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

} // namespace rk
