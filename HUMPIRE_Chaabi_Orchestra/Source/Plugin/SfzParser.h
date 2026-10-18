#pragma once
#include <juce_core/juce_core.h>
#include <vector>

// Minimal but robust SFZ parser (open, royalty-free format).
// Headers : <control> <global> <master> <group> <region>
// Opcodes supportes : sample, default_path, lokey/hikey/key, pitch_keycenter, lovel/hivel,
//   lorand/hirand, seq_length/seq_position, volume, pan, tune, transpose, pitch_keytrack,
//   loop_mode, loop_start, loop_end, offset, end, ampeg_attack/decay/sustain/release,
//   amp_veltrack, group, off_by, trigger (attack|release), sw_lokey/sw_hikey/sw_last/sw_default.
namespace humpire
{
struct SfzRegionDef
{
    juce::File sample;
    int lokey = 0, hikey = 127, keycenter = 60, lovel = 0, hivel = 127;
    float lorand = 0.f, hirand = 1.f;
    int seqLength = 1, seqPosition = 1;
    float volumeDb = 0.f, pan = 0.f, tuneCents = 0.f, pitchKeytrack = 100.f;
    int transpose = 0;
    enum class Loop { noLoop, oneShot, continuous, sustain } loop = Loop::noLoop;
    juce::int64 offset = 0, end = -1, loopStart = -1, loopEnd = -1;
    float ampAttack = 0.f, ampDecay = 0.f, ampSustain = 100.f, ampRelease = 0.03f, ampVeltrack = 100.f;
    int group = 0, offBy = 0;
    bool triggerRelease = false;
    int swLast = -1;
};

struct SfzFile
{
    std::vector<SfzRegionDef> regions;
    int swLo = -1, swHi = -1, swDefault = -1;
    juce::StringArray warnings;
};

SfzFile parseSfz (const juce::File& file);
int parseNoteOrInt (const juce::String& s, int fallback);
} // namespace humpire
