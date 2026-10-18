#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include "Instruments.h"

namespace humpire
{
struct PhraseEvent { float beat; int note; int vel; float len; };

struct Phrase
{
    juce::String name;
    float lengthBeats;                 // loop length in quarter notes
    std::vector<PhraseEvent> events;
};

// MIDI phrase bank per instrument. initPhraseBank() must be called off the audio thread.
void initPhraseBank();
const std::vector<Phrase>& phrasesFor (int instrument);

// Export as a .mid file (drag & drop into the DAW).
juce::File writePhraseMidiFile (const Phrase& p, double bpm);
} // namespace humpire
