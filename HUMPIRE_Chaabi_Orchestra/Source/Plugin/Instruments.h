#pragma once
#include <juce_core/juce_core.h>
#include <cstdint>

namespace humpire
{
inline constexpr int kNumInstruments = 8;
inline constexpr int kMaxMultiInstruments = 4;
inline constexpr int kMaxVoices = 64;

// Zones clavier (numeros MIDI)
inline constexpr int kKeyswitchLo = 12, kKeyswitchHi = 23;  // articulations SFZ (sw_last)
inline constexpr int kPhraseLo    = 24, kPhraseHi    = 35;  // declencheurs de phrases
inline constexpr int kPlayLo      = 36;                      // debut de la zone de jeu

struct InstrumentInfo
{
    const char* id;
    const char* name;      // UTF-8
    const char* folder;    // dossier dans la bibliotheque de samples
    const char* sfz;       // fichier SFZ principal
    int defaultLo, defaultHi;
    bool percussive;
    std::uint32_t colour;
    const char* tagline;
};

inline constexpr InstrumentInfo kInstruments[kNumInstruments] = {
    { "mandole",  "Mandole alg\xC3\xA9rien", "Mandole",  "Mandole.sfz",  50, 78, false, 0xffc1440e, "Cordes pinc\xC3\xA9" "es \xE2\x80\xA2 2 octaves \xE2\x80\xA2 2 vitesses \xE2\x80\xA2 3 RR" },
    { "banjo",    "Banjo t\xC3\xA9nor",      "Banjo",    "Banjo.sfz",    55, 91, false, 0xffd9902f, "Notes + patterns chaabi" },
    { "violon",   "Violon (alto)",           "Violon",   "Violon.sfz",   55, 96, false, 0xffa8553a, "Notes tenues \xE2\x80\xA2 vibrato \xE2\x80\xA2 phrases" },
    { "ney",      "Ney",                     "Ney",      "Ney.sfz",      60, 93, false, 0xffe0b878, "Souffle \xE2\x80\xA2 notes tenues \xE2\x80\xA2 phrases" },
    { "qanun",    "Qan\xC3\xBBn",            "Qanun",    "Qanun.sfz",    48, 96, false, 0xffb8862b, "Notes \xE2\x80\xA2 accords \xE2\x80\xA2 glissandos" },
    { "derbouka", "Derbouka",                "Derbouka", "Derbouka.sfz", 36, 47, true,  0xfff5b544, "DUM \xE2\x80\xA2 TEK \xE2\x80\xA2 KA \xE2\x80\xA2 slap \xE2\x80\xA2 ghost" },
    { "tar",      "Tar",                     "Tar",      "Tar.sfz",      48, 59, true,  0xffc1440e, "Frappes \xE2\x80\xA2 cymbalettes (zil)" },
    { "bendir",   "Bendir",                  "Bendir",   "Bendir.sfz",   36, 47, true,  0xffa8553a, "Frappes + phrases" },
};

inline juce::String instrumentName (int i) { return juce::String::fromUTF8 (kInstruments[i].name); }

// Mapping des notes percussives (lignes du sequenceur).
// Derbouka : 36 DUM, 38 TEK, 40 KA, 41 SLAP, 43 GHOST
// Tar      : 36 DUM, 38 TEK, 40 KA(rim), 42 ZIL, 45 GHOST
inline constexpr int kSeqLanes = 5, kSeqSteps = 16, kSeqInstruments = 2;
inline constexpr int kSeqNotes[kSeqInstruments][kSeqLanes] = { { 36, 38, 40, 41, 43 }, { 36, 38, 40, 42, 45 } };
inline constexpr int kSeqSlot[kSeqInstruments] = { 5, 6 };       // derbouka, tar
inline const char* const kSeqLaneNames[kSeqInstruments][kSeqLanes] =
    { { "DUM", "TEK", "KA", "SLAP", "GHOST" }, { "DUM", "TEK", "KA", "ZIL", "GHOST" } };
} // namespace humpire
