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
    float lengthBeats;                 // longueur de boucle en noires
    std::vector<PhraseEvent> events;
};

// Banque de phrases MIDI par instrument. init() doit etre appelee hors thread audio.
void initPhraseBank();
const std::vector<Phrase>& phrasesFor (int instrument);

// Export en fichier .mid (drag & drop vers le DAW).
juce::File writePhraseMidiFile (const Phrase& p, double bpm);
} // namespace humpire
