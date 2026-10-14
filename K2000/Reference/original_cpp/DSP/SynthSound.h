#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace K2000Audio
{
    class SynthSound : public juce::SynthesiserSound
    {
    public:
        SynthSound() = default;
        bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
        bool appliesToChannel(int /*midiChannel*/) override { return true; }
    };
}
