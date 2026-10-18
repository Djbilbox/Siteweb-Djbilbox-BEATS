#pragma once
#include <juce_core/juce_core.h>
#include <cstdint>
#include "Model/ModelLibrary.h"

namespace humpire
{
inline constexpr int kNumInstruments = 8;
inline constexpr int kMaxMultiInstruments = 4;
inline constexpr int kMaxVoices = 64;

// Keyboard zones (MIDI note numbers)
inline constexpr int kKeyswitchLo = 12, kKeyswitchHi = 23;  // articulations (SFZ sw_last)
inline constexpr int kPhraseLo    = 24, kPhraseHi    = 35;  // phrase triggers
inline constexpr int kPlayLo      = 36;                      // start of the playing zone

struct InstrumentInfo
{
    const char* id;
    const char* name;      // display name (UTF-8)
    const char* folder;    // folder inside the sample library (unchanged: user libraries rely on it)
    const char* sfz;       // main SFZ file
    int defaultLo, defaultHi;
    bool percussive;
    std::uint32_t colour;
    const char* tagline;   // short description of the features (UTF-8)
};

inline constexpr InstrumentInfo kInstruments[kNumInstruments] = {
    { "mandole",  "Mandole",  "Mandole",  "Mandole.sfz",  50, 78, false, 0xffc1440e, "Plucked strings \xE2\x80\xA2 2 octaves \xE2\x80\xA2 2 velocities \xE2\x80\xA2 3 RR" },
    { "banjo",    "Banjo",    "Banjo",    "Banjo.sfz",    55, 91, false, 0xffd9902f, "Notes + chaabi patterns" },
    { "violon",   "Violin",   "Violon",   "Violon.sfz",   55, 96, false, 0xffa8553a, "Sustained notes \xE2\x80\xA2 vibrato \xE2\x80\xA2 phrases" },
    { "ney",      "Ney",      "Ney",      "Ney.sfz",      60, 93, false, 0xffe0b878, "Breath \xE2\x80\xA2 sustained notes \xE2\x80\xA2 phrases" },
    { "qanun",    "Qanun",    "Qanun",    "Qanun.sfz",    48, 96, false, 0xffb8862b, "Notes \xE2\x80\xA2 chords \xE2\x80\xA2 glissandos" },
    { "derbouka", "Derbouka", "Derbouka", "Derbouka.sfz", 36, 47, true,  0xfff5b544, "DUM \xE2\x80\xA2 TEK \xE2\x80\xA2 KA \xE2\x80\xA2 slap \xE2\x80\xA2 ghost" },
    { "tar",      "Tar",      "Tar",      "Tar.sfz",      48, 59, true,  0xffc1440e, "Strokes \xE2\x80\xA2 jingles (zil)" },
    { "bendir",   "Bendir",   "Bendir",   "Bendir.sfz",   36, 47, true,  0xffa8553a, "Strokes + phrases" },
};

// The built-in modelled library (Model/ModelLibrary.h) must describe the same instruments.
namespace detail
{
constexpr bool sameText (const char* a, const char* b)
{
    while (*a != '\0' && *a == *b) { ++a; ++b; }
    return *a == *b;
}
constexpr bool modelSpecMatchesInstruments()
{
    for (int i = 0; i < kNumInstruments; ++i)
    {
        const auto& m = model::kSpecs[i];
        const auto& k = kInstruments[i];
        if (! sameText (m.id, k.id) || ! sameText (m.folder, k.folder) || m.percussive != k.percussive) return false;
        // pitched instruments: the library covers exactly the default range; percussion: at least the default range
        if (! k.percussive && (m.lo != k.defaultLo || m.hi != k.defaultHi)) return false;
        if (k.percussive && (m.lo > k.defaultLo || m.hi < k.defaultHi)) return false;
    }
    return model::kKeyswitchLo == kKeyswitchLo && model::kKeyswitchHi == kKeyswitchHi
        && model::kPhraseLo == kPhraseLo && model::kPhraseHi == kPhraseHi && model::kPlayLo == kPlayLo
        && model::kNumModelInstruments == kNumInstruments;
}
} // namespace detail
static_assert (detail::modelSpecMatchesInstruments(), "Model/ModelLibrary.h kSpecs must match kInstruments");

inline juce::String instrumentName (int i) { return juce::String::fromUTF8 (kInstruments[i].name); }

// Percussion note map (sequencer lanes).
// Derbouka : 36 DUM, 38 TEK, 40 KA, 41 SLAP, 43 GHOST
// Tar      : 36 DUM, 38 TEK, 40 KA (rim), 42 ZIL, 45 GHOST
inline constexpr int kSeqLanes = 5, kSeqSteps = 16, kSeqInstruments = 2;
inline constexpr int kSeqNotes[kSeqInstruments][kSeqLanes] = { { 36, 38, 40, 41, 43 }, { 36, 38, 40, 42, 45 } };
inline constexpr int kSeqSlot[kSeqInstruments] = { 5, 6 };       // derbouka, tar
inline const char* const kSeqLaneNames[kSeqInstruments][kSeqLanes] =
    { { "DUM", "TEK", "KA", "SLAP", "GHOST" }, { "DUM", "TEK", "KA", "ZIL", "GHOST" } };
} // namespace humpire
