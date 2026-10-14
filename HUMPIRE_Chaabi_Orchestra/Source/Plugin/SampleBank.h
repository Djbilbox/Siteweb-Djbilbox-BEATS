#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <functional>
#include <memory>
#include "SfzParser.h"

namespace humpire
{
// Donnees audio partagees entre regions (un fichier = un SampleData).
struct SampleData
{
    juce::AudioBuffer<float> audio;     // 1 ou 2 canaux
    double sampleRate = 44100.0;
    int numFrames = 0;
};

// Une region SFZ jouable. Derive de SynthesiserSound pour etre acceptee par juce::Synthesiser::startVoice.
class Region : public juce::SynthesiserSound
{
public:
    Region (SfzRegionDef d, std::shared_ptr<SampleData> s) : def (std::move (d)), data (std::move (s)) {}

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }

    // Selection de region : cle, velocite, aleatoire, round-robin, keyswitch, declencheur attack/release.
    bool matches (int note, int vel127, float rnd, int seqIndex, int activeSwitch, bool release) const noexcept
    {
        if (def.triggerRelease != release) return false;
        if (note < def.lokey || note > def.hikey) return false;
        if (vel127 < def.lovel || vel127 > def.hivel) return false;
        if (rnd < def.lorand || (rnd >= def.hirand && def.hirand < 1.0f)) return false;
        if (def.seqLength > 1 && (seqIndex % def.seqLength) + 1 != def.seqPosition) return false;
        if (def.swLast >= 0 && def.swLast != activeSwitch) return false;
        return true;
    }

    SfzRegionDef def;
    std::shared_ptr<SampleData> data;
};

// Banque = contenu d'un fichier SFZ charge en memoire.
class Bank
{
public:
    juce::ReferenceCountedArray<Region> regions;
    bool hasReleaseRegions = false;
    int swLo = -1, swHi = -1, swDefault = -1;
    juce::String name;
    double memoryMB = 0.0;
    juce::StringArray warnings;

    // Charge un .sfz (thread de fond). Retourne nullptr + error en cas d'echec.
    static std::shared_ptr<Bank> load (const juce::File& sfz, std::atomic<float>& progress,
                                       const std::function<bool()>& cancelled, juce::String& error);
};
} // namespace humpire
