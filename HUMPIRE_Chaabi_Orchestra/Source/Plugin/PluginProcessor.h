#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include "HumpireCore/HumpirePresetManager.h"
#include "Params.h"
#include "ChaabiSynth.h"
#include "PhrasePlayer.h"

namespace humpire
{
// Etat de chargement d'un instrument (lu par l'UI)
struct SlotStatus { int state = 0; float progress = 0.f; juce::String message; }; // 0 vide, 1 charge, 2 pret, 3 erreur

class ChaabiProcessor : public juce::AudioProcessor,
                        private juce::AsyncUpdater,
                        private juce::ValueTree::Listener,
                        private juce::AudioProcessorValueTreeState::Listener
{
public:
    ChaabiProcessor();
    ~ChaabiProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "HUMPIRE CHAABI ORCHESTRA"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter (pid::bypass); }

    // ---- API pour l'UI (thread message)
    SlotStatus getSlotStatus (int slot) const;
    bool isSlotActive (int slot) const;
    juce::File getLibraryRoot() const;
    void setLibraryRoot (const juce::File&);
    juce::File resolveSfz (int slot) const;
    int getSeqStep (int inst, int lane, int step) const;
    void setSeqStep (int inst, int lane, int step, int value);
    double getCurrentBpm() const noexcept { return lastBpm.load(); }

    juce::UndoManager undoManager;                     // doit preceder apvts
    juce::AudioProcessorValueTreeState apvts;
    juce::MidiKeyboardState keyboardState;
    std::unique_ptr<PresetManager> presets;

    std::atomic<float> meterL { 0.f }, meterR { 0.f }, midiActivity { 0.f };
    PhrasePlayer::Grid seqGrid;
    std::unique_ptr<PhrasePlayer> phrasePlayer;

private:
    struct SlotParams
    {
        std::atomic<float> *vol, *pan, *rev, *dly, *filter, *pitch, *atk, *rel, *lo, *hi, *on;
    };

    struct Slot
    {
        ChaabiSynth synth;
        SlotParams p {};
        juce::AudioBuffer<float> buf;
        juce::dsp::StateVariableTPTFilter<float> filter;
        juce::SmoothedValue<float> vol, pan, rev, dly;
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> cutoff;
        bool wasActive = false;

        // transmission de banque (thread de chargement -> thread audio)
        juce::SpinLock pendingLock;
        std::shared_ptr<Bank> pending, retired;
        std::atomic<int> state { 0 };
        std::atomic<float> progress { 0.f };
        std::atomic<int> generation { 0 };
        mutable juce::CriticalSection msgLock;
        juce::String message, loadedPath;
    };

    void handleAsyncUpdate() override;
    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
    void parameterChanged (const juce::String&, float) override;

    void refreshLoads();
    void requestLoad (int slot, const juce::File& sfz);
    void pickupBank (Slot&);
    void decodeSeqGrid (const juce::String&);
    static juce::String defaultSeqGrid();

    std::array<Slot, kNumInstruments> slots;
    juce::ThreadPool loaderPool { 1 };

    juce::Reverb reverb;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> delayL { 96000 * 2 }, delayR { 96000 * 2 };
    juce::SmoothedValue<float> masterGain, delaySamples, delayFb;
    juce::AudioBuffer<float> bus;           // 0-1 dry, 2-3 reverb, 4 delay
    SlotMidi slotMidi;
    double sr = 44100.0;
    int maxBlock = 0;
    std::atomic<double> lastBpm { 90.0 };
    std::array<bool, kNumInstruments> activeNow {};

    std::atomic<float> *pSelected, *pMulti, *pPoly, *pAuth, *pMaster, *pRevSize, *pRevDamp, *pDlyTime, *pDlyFb,
                       *pTempo, *pBend, *pSeqOn, *pSeqSwing, *pSeqRate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChaabiProcessor)
};
} // namespace humpire
