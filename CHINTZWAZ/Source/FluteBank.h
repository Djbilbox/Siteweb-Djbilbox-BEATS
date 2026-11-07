#pragma once
// The real flute of the Hippie Flute engine: 32 recorded notes (VSCO 2 Community Edition, CC0), with and without
// vibrato, soft and loud, each looped in its steady middle (Tools/make_flute.py). Decoded once into memory.
#include <juce_audio_formats/juce_audio_formats.h>
#include "BinaryData.h"
#include "FluteMap.h"

namespace Cz
{
class FluteBank
{
public:
    struct Note { const FluteNote* info; juce::AudioBuffer<float> audio; };

    // created on the message thread (processor constructor) before any voice plays
    static const FluteBank& get() { static const FluteBank bank; return bank; }

    // the recording to play for a key: the closest recorded key of the wanted layer (vibrato or straight), the soft
    // or loud take after the velocity; takes of the same key alternate (round robin)
    const Note* pick (int midiNote, bool vibrato, float velocity) const
    {
        const Note* best = nullptr; int bestDist = 1000, takes = 0;
        for (auto& n : notes)
        {
            if (n.info->vibrato != vibrato) continue;
            if (! vibrato && ((velocity < 0.6f) != (n.info->velocityLayer == 1))) continue;
            const int d = std::abs (n.info->key - midiNote);
            if (d < bestDist) { bestDist = d; best = &n; takes = 1; }
            else if (d == bestDist && best != nullptr && n.info->key == best->info->key && (roundRobin.fetch_add (1) & 1) != 0) { best = &n; ++takes; }
        }
        return best;
    }

private:
    std::vector<Note> notes;
    mutable std::atomic<unsigned> roundRobin { 0 };

    FluteBank()
    {
        juce::FlacAudioFormat flac;
        for (const auto& info : fluteNotes)
        {
            int size = 0;
            const char* data = BinaryData::getNamedResource (info.resource, size);
            if (data == nullptr) continue;
            std::unique_ptr<juce::AudioFormatReader> r (flac.createReaderFor (new juce::MemoryInputStream (data, (size_t) size, false), true));
            if (r == nullptr) continue;
            Note n { &info, juce::AudioBuffer<float> (1, (int) r->lengthInSamples) };
            r->read (&n.audio, 0, (int) r->lengthInSamples, 0, true, false);
            notes.push_back (std::move (n));
        }
    }
};
}
