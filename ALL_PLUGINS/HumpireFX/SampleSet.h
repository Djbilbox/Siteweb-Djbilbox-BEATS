#pragma once
// Real recorded instruments for the DJBILBOX BEATS synths (shared by all of them).
// A SampleSet is the notes of one instrument, embedded as FLAC (BinaryData) and decoded once into memory:
// each note knows its key, its tuning, its velocity layer and, for held instruments, a sustain loop baked in its
// steady middle (ALL_PLUGINS/HumpireFX/tools/make_samples.py). A SamplePlayer plays one of them in a voice.
#include <juce_audio_formats/juce_audio_formats.h>
#include <atomic>
#include <vector>

namespace hfx
{
struct SampleNote
{
    const char* resource;   // BinaryData name of the FLAC
    int key;                // MIDI key of the recording
    float cents;            // its exact tuning
    int layer;              // articulation / colour (0 = main, 1, 2... as the instrument defines)
    int velocity;           // velocity layer, 1 = softest
    int loopStart, loopEnd; // sustain loop in samples (loopEnd < 0: plays once to its end)
    int length;
    int lokey = -1, hikey = -1;   // drums: the keys that play this stroke, at its own pitch (-1: tuned instrument)
};

class SampleSet
{
public:
    struct Note { const SampleNote* info; juce::AudioBuffer<float> audio; };

    // decode every note of a table; call on the message thread (processor constructor), never while playing
    template <size_t N>
    void load (const SampleNote (&table)[N], const char* (*getResource) (const char*, int&))
    {
        juce::FlacAudioFormat flac;
        notes.clear();
        for (const auto& info : table)
        {
            int size = 0;
            const char* data = getResource (info.resource, size);
            if (data == nullptr) continue;
            std::unique_ptr<juce::AudioFormatReader> r (flac.createReaderFor (new juce::MemoryInputStream (data, (size_t) size, false), true));
            if (r == nullptr) continue;
            Note n { &info, juce::AudioBuffer<float> (1, (int) r->lengthInSamples) };
            r->read (&n.audio, 0, (int) r->lengthInSamples, 0, true, false);
            notes.push_back (std::move (n));
            maxVelocity = juce::jmax (maxVelocity, info.velocity);
        }
    }

    bool isEmpty() const { return notes.empty(); }

    // the closest recorded key of a layer; the velocity layer after the note's velocity; takes of the same key and
    // velocity alternate (round robin)
    const Note* pick (int midiNote, float velocity, int layer = 0) const
    {
        const int wantVel = juce::jlimit (1, juce::jmax (1, maxVelocity), 1 + (int) (velocity * (float) maxVelocity * 0.999f));
        const Note* best = nullptr; int bestScore = 1 << 30;
        const bool flip = (roundRobin.fetch_add (1) & 1) != 0;
        for (auto& n : notes)
        {
            if (n.info->layer != layer) continue;
            const bool drum = n.info->lokey >= 0;
            if (drum && (midiNote < n.info->lokey || midiNote > n.info->hikey)) continue;
            const int score = (drum ? 0 : std::abs (n.info->key - midiNote) * 16) + std::abs (n.info->velocity - wantVel);
            if (score < bestScore || (score == bestScore && flip)) { bestScore = score; best = &n; }
        }
        if (best == nullptr && layer != 0) return pick (midiNote, velocity, 0);
        return best;
    }

private:
    std::vector<Note> notes;
    int maxVelocity = 1;
    mutable std::atomic<unsigned> roundRobin { 0 };
};

// One recording playing in a voice: transposed to the wanted frequency, looped while held (or to its end once).
struct SamplePlayer
{
    const SampleSet::Note* note = nullptr;
    double pos = 0.0;

    void start (const SampleSet::Note* n) { note = n; pos = 0.0; }
    bool isDrum() const { return note != nullptr && note->info->lokey >= 0; }
    bool active() const { return note != nullptr; }

    float next (double freq, double sampleRate, double fileRate = 44100.0)
    {
        if (note == nullptr) return 0.f;
        const auto& in = *note->info;
        const int len = note->audio.getNumSamples();
        if (in.loopEnd < 0 && pos >= (double) (len - 1)) return 0.f;
        const float* b = note->audio.getReadPointer (0);
        const int i0 = (int) pos;
        const float fr = (float) (pos - i0);
        const float s0 = b[juce::jmin (i0, len - 1)], s1 = b[juce::jmin (i0 + 1, len - 1)];
        const double root = 440.0 * std::exp2 (((double) in.key - 69.0) / 12.0 + (double) in.cents / 1200.0);
        pos += (in.lokey >= 0 ? 1.0 : freq / root) * fileRate / sampleRate;   // drums keep their own pitch
        if (in.loopEnd > 0 && pos >= (double) in.loopEnd) pos -= (double) (in.loopEnd - in.loopStart);
        return s0 + (s1 - s0) * fr;
    }
};
}
