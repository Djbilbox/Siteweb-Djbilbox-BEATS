#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <atomic>
#include "PhraseBank.h"

namespace humpire
{
using SlotMidi = std::array<juce::MidiBuffer, kNumInstruments>;

// Phrase player (keys C0..B0) + 16-step sequencer. Everything is pre-allocated: no allocation in process().
class PhrasePlayer
{
public:
    // grid[inst][lane][step]: 0 off, 1 normal, 2 accent
    using Grid = std::array<std::array<std::array<std::atomic<std::uint8_t>, kSeqSteps>, kSeqLanes>, kSeqInstruments>;

    explicit PhrasePlayer (const Grid& g) : grid (g) {}

    void prepare (double sampleRate);
    void reset();

    // key = MIDI note of the phrase zone; sampleOffset = position inside the current block.
    void trigger (int key, const std::array<bool, kNumInstruments>& activeSlots, int sampleOffset);
    void release (int key);

    void process (SlotMidi& out, int numSamples, double bpm, bool hostPlaying, double hostPpq,
                  bool seqRunning, double swing, int stepsPerBeat, const std::array<bool, kNumInstruments>& activeSlots);

    std::atomic<int> seqDisplayStep { -1 };

private:
    struct Active { bool on = false; int slot = 0, key = 0; const Phrase* phrase = nullptr; double beat = 0.0; int startOffset = 0; bool pendingSync = false; };
    struct Off { bool used = false; int slot = 0, note = 0; juce::int64 at = 0; };

    void emit (SlotMidi& out, int slot, int note, int vel, int offset, double lenSamples);

    const Grid& grid;
    double sr = 44100.0;
    juce::int64 clock = 0;
    double seqBeat = 0.0;
    std::array<Active, 48> actives;
    std::array<Off, 256> offs;
};
} // namespace humpire
