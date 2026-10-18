#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <functional>
#include <memory>
#include "SfzParser.h"
#include "Model/ModelLibrary.h"

namespace humpire
{
// Audio data shared between regions (one file / one rendered zone = one SampleData).
struct SampleData
{
    juce::AudioBuffer<float> audio;     // 1 or 2 channels
    double sampleRate = 44100.0;
    int numFrames = 0;
};

// A playable SFZ-style region.  Derives from SynthesiserSound so that juce::Synthesiser::startVoice accepts it.
class Region : public juce::SynthesiserSound
{
public:
    Region (SfzRegionDef d, std::shared_ptr<SampleData> s) : def (std::move (d)), data (std::move (s)) {}

    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }

    // Region selection: key, velocity, random, round robin, keyswitch, attack/release trigger.
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

// A bank = the content of one instrument: either a loaded SFZ file or the built-in modelled library.
// Immutable once built (may be shared by several plug-in instances).
class Bank
{
public:
    juce::ReferenceCountedArray<Region> regions;
    bool hasReleaseRegions = false;
    bool builtIn = false;               // true: synthesised by the built-in modelled library
    int swLo = -1, swHi = -1, swDefault = -1;
    juce::String name;
    double memoryMB = 0.0;
    juce::StringArray warnings;

    // Loads an .sfz (background thread).  Returns nullptr + error on failure.
    static std::shared_ptr<Bank> load (const juce::File& sfz, std::atomic<float>& progress,
                                       const std::function<bool()>& cancelled, juce::String& error);

    // Renders the built-in modelled library of `instrumentIndex` (background thread, cancellable) and
    // converts it into a bank.  Identical requests are served from a process-wide cache while some plug-in
    // instance still holds the bank.  Returns nullptr + error on failure / cancellation.
    static std::shared_ptr<Bank> buildFromModel (int instrumentIndex, double sampleRate, std::atomic<float>& progress,
                                                 const std::function<bool()>& cancelled, juce::String& error);

    // Converts an already rendered library (same Region / SampleData semantics as an SFZ file).
    static std::shared_ptr<Bank> fromLibrary (const model::Library&, juce::String& error);
};
} // namespace humpire
