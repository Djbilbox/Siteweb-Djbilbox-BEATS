#pragma once

#include <JuceHeader.h>
#include <vector>

// Runtime equivalent of PresetDefinition (Source/Params/PresetData.h) — same
// fields, but String instead of const char* so packs can be parsed from disk
// at any time instead of only existing as compile-time data.
struct RuntimePreset
{
    juce::String name, category;

    int oscWaveform = 0;
    double filterCutoffHz = 8000.0, filterResonance = 1.0;
    int filterType = 0;
    double filterEnvDepthOct = 0.0;

    double attackMs = 10.0, decayMs = 200.0, sustainLevel = 0.7, releaseMs = 300.0;
    double glideMs = 0.0;

    double lfoRateHz = 2.0, lfoAmount = 0.0;
    int lfoDestination = 0;

    double distortionDriveDb = 0.0, distortionToneHz = 5000.0, distortionMix = 0.0;
    double chorusRateHz = 1.5, chorusAmount = 0.0, chorusMix = 0.0;
    double delayTimeMs = 250.0, delayFeedback = 0.0, delayMix = 0.0;
    double reverbRoom = 0.3, reverbDamping = 0.5, reverbWidth = 1.0, reverbMix = 0.2;

    double masterVolume = 1.0;

    // optional (absent = derived from the amp envelope)
    double filterAttackMs = 1.0, filterDecayMs = -1.0, filterSustain = -1.0, velocity = 0.4;

    int unisonVoices = 1;
    double unisonDetune = 0.0;
    double unisonWidth = 0.0;
};

// One .wz808exp file's worth of presets — shown on the cassette label as a
// group and grouped under a section heading in the preset dropdown.
struct ExpansionPack
{
    juce::String name;   // e.g. "Expansion Vol.1" — drawn on the cassette label
    juce::String author;
    std::vector<RuntimePreset> presets;
};

// Parses a .wz808exp JSON file (see Assets/EXPANSION_WZ_VOL1.wz808exp for the
// canonical example). Returns false and leaves outPack untouched on any
// parse/schema failure, so a malformed file can't half-load or crash the plugin.
bool loadExpansionPackFromFile(const juce::File& file, ExpansionPack& outPack);

bool loadExpansionPackFromString(const juce::String& json, ExpansionPack& outPack);
