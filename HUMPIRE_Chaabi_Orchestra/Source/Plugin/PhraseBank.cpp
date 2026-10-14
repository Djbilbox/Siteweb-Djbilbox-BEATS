#include "PhraseBank.h"
#include <array>

namespace humpire
{
namespace
{
std::array<std::vector<Phrase>, kNumInstruments> bank;
bool initialised = false;

// Percussions : D=DUM T=TEK K=KA S=SLAP G=GHOST (tar : Z = zil sur 42)
enum : int { D = 36, T = 38, K = 40, S = 41, G = 43, Z = 42 };

Phrase perc (const char* name, float len, std::initializer_list<PhraseEvent> ev)
{
    return { juce::String::fromUTF8 (name), len, ev };
}

// Courses melodiques : notes successives de duree "step"
Phrase run (const char* name, std::initializer_list<int> notes, float step, float len, int vel = 96)
{
    Phrase p { juce::String::fromUTF8 (name), 0.f, {} };
    float t = 0.f;
    for (int n : notes) { p.events.push_back ({ t, n, vel, step * len }); t += step; }
    p.lengthBeats = juce::jmax (1.0f, std::ceil (t));
    return p;
}

void build()
{
    // Gamme Hijaz sur re : D4 Eb4 F#4 G4 A4 Bb4 C5 D5
    const std::initializer_list<int> up   { 62, 63, 66, 67, 69, 70, 72, 74 };
    const std::initializer_list<int> down { 74, 72, 70, 69, 67, 66, 63, 62 };

    bank[0] = { run ("Hijaz montée", up, 0.5f, 0.9f), run ("Hijaz descente", down, 0.5f, 0.9f),
                run ("Trémolo la", { 69, 69, 69, 69, 69, 69, 69, 69 }, 0.25f, 0.8f, 88),
                run ("Arpège ré-la", { 62, 69, 74, 69, 62, 69, 74, 69 }, 0.5f, 0.95f) };
    bank[1] = { run ("Roulé banjo", { 62, 66, 69, 66, 62, 66, 69, 74 }, 0.25f, 0.7f, 100),
                run ("Montée rapide", up, 0.25f, 0.7f), run ("Descente rapide", down, 0.25f, 0.7f),
                run ("Motif chaabi", { 69, 69, 67, 66, 67, 69, 66, 62 }, 0.5f, 0.8f) };
    bank[2] = { run ("Taqsim lent", { 62, 63, 66, 67 }, 2.0f, 1.0f, 90), run ("Descente tenue", { 74, 72, 70, 69 }, 2.0f, 1.0f, 90),
                run ("Phrase hijaz", { 62, 66, 67, 69, 67, 66, 63, 62 }, 1.0f, 0.95f, 92) };
    bank[3] = { run ("Appel du ney", { 62, 66, 69 }, 1.5f, 1.0f, 85), run ("Souffle long", { 69, 67, 66, 62 }, 2.0f, 1.0f, 80),
                run ("Ornement", { 69, 70, 69, 67, 66, 67, 69, 62 }, 0.5f, 0.9f, 88) };
    bank[4] = { run ("Glissando montant", { 62, 63, 66, 67, 69, 70, 72, 74, 75, 78, 79, 81 }, 0.125f, 1.4f, 90),
                run ("Arpège accord", { 62, 66, 69, 74, 69, 66, 62 }, 0.5f, 1.6f, 94),
                run ("Mélodie qanûn", { 69, 70, 69, 67, 66, 67, 69, 62 }, 0.5f, 1.1f) };

    // Derbouka (4/4 en noires, 6/8 sur 3 noires)
    bank[5] = { perc ("Maqsum", 4.f, { { 0.f, D, 110, .2f }, { .5f, T, 90, .2f }, { 1.5f, T, 90, .2f }, { 2.f, D, 105, .2f }, { 3.f, T, 90, .2f } }),
                perc ("Maqsum ghost", 4.f, { { 0.f, D, 112, .2f }, { .5f, T, 90, .2f }, { 1.f, G, 45, .2f }, { 1.5f, T, 88, .2f }, { 2.f, D, 105, .2f }, { 2.5f, G, 45, .2f }, { 3.f, T, 90, .2f }, { 3.5f, K, 70, .2f } }),
                perc ("Saïdi", 4.f, { { 0.f, D, 112, .2f }, { .5f, D, 95, .2f }, { 1.5f, T, 90, .2f }, { 2.f, D, 105, .2f }, { 3.f, T, 90, .2f } }),
                perc ("Chaabi 6/8", 3.f, { { 0.f, D, 112, .2f }, { 1.f, T, 90, .2f }, { 1.5f, K, 75, .2f }, { 2.f, T, 92, .2f }, { 2.5f, G, 45, .2f } }),
                perc ("Chaabi 6/8 slap", 3.f, { { 0.f, D, 112, .2f }, { .5f, G, 45, .2f }, { 1.f, S, 100, .2f }, { 1.5f, K, 75, .2f }, { 2.f, T, 92, .2f }, { 2.5f, S, 95, .2f } }) };

    // Tar (D=36 T=38 K=40 Z=42 zil)
    bank[6] = { perc ("Tar maqsum", 4.f, { { 0.f, D, 108, .2f }, { .5f, T, 88, .2f }, { 1.f, Z, 70, .2f }, { 1.5f, T, 88, .2f }, { 2.f, D, 104, .2f }, { 3.f, T, 90, .2f }, { 3.5f, Z, 70, .2f } }),
                perc ("Tar zil", 4.f, { { 0.f, D, 110, .2f }, { .5f, Z, 60, .2f }, { 1.f, Z, 60, .2f }, { 1.5f, T, 85, .2f }, { 2.f, D, 105, .2f }, { 2.5f, Z, 60, .2f }, { 3.f, T, 88, .2f }, { 3.5f, Z, 60, .2f } }),
                perc ("Tar 6/8", 3.f, { { 0.f, D, 110, .2f }, { 1.f, T, 90, .2f }, { 1.5f, Z, 65, .2f }, { 2.f, T, 88, .2f }, { 2.5f, K, 72, .2f } }) };

    // Bendir
    bank[7] = { perc ("Bendir 4/4", 4.f, { { 0.f, D, 108, .2f }, { 1.f, T, 85, .2f }, { 2.f, D, 100, .2f }, { 2.5f, T, 80, .2f }, { 3.f, T, 85, .2f } }),
                perc ("Bendir roulé", 4.f, { { 0.f, D, 108, .2f }, { .75f, K, 70, .2f }, { 1.f, T, 85, .2f }, { 2.f, D, 100, .2f }, { 2.75f, K, 70, .2f }, { 3.f, T, 88, .2f }, { 3.5f, K, 72, .2f } }),
                perc ("Bendir 6/8", 3.f, { { 0.f, D, 110, .2f }, { 1.f, T, 85, .2f }, { 1.5f, K, 70, .2f }, { 2.f, T, 85, .2f } }) };
}
} // namespace

void initPhraseBank()
{
    if (! initialised) { build(); initialised = true; }
}

const std::vector<Phrase>& phrasesFor (int instrument)
{
    return bank[(size_t) juce::jlimit (0, kNumInstruments - 1, instrument)];
}

juce::File writePhraseMidiFile (const Phrase& p, double bpm)
{
    constexpr int ppq = 480;
    juce::MidiMessageSequence seq;
    seq.addEvent (juce::MidiMessage::tempoMetaEvent ((int) (60000000.0 / juce::jmax (30.0, bpm))), 0.0);
    for (auto& e : p.events)
    {
        seq.addEvent (juce::MidiMessage::noteOn (1, e.note, (juce::uint8) e.vel), e.beat * ppq);
        seq.addEvent (juce::MidiMessage::noteOff (1, e.note), (e.beat + e.len) * ppq);
    }
    seq.addEvent (juce::MidiMessage::endOfTrack(), p.lengthBeats * ppq);
    seq.updateMatchedPairs();

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (ppq);
    mf.addTrack (seq);

    auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("HUMPIRE_Phrases");
    dir.createDirectory();
    auto file = dir.getChildFile (juce::File::createLegalFileName (p.name) + ".mid");
    file.deleteFile();
    if (auto os = file.createOutputStream()) mf.writeTo (*os);
    return file;
}
} // namespace humpire
