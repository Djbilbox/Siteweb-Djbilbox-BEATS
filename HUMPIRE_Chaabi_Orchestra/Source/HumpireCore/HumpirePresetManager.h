#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>

namespace humpire
{
// Factory preset: real parameter values (id -> value) + state properties (non-parameters).
struct FactoryPreset
{
    juce::String name;
    juce::String category;
    std::vector<std::pair<juce::String, float>> values;
    std::vector<std::pair<juce::String, juce::String>> props;
};

// Preset manager + A/B. Message thread only.
class PresetManager
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& apvts, juce::UndoManager& undo,
                   juce::File userPresetDir, std::vector<FactoryPreset> factory);

    int  getNumPresets() const noexcept { return (int) names.size(); }
    int  getCurrentIndex() const noexcept { return current; }
    juce::String getCurrentName() const;
    juce::String getName (int i) const { return names[(size_t) i]; }
    juce::String getCategory (int i) const { return categories[(size_t) i]; }

    void loadIndex (int i);
    void next();
    void prev();

    bool saveUser (const juce::String& name);
    void rescanUserPresets();

    // A/B: switches between two complete states.
    void toggleAB();
    bool isB() const noexcept { return currentIsB; }

private:
    void applyFactory (const FactoryPreset&);
    bool loadUserFile (const juce::File&);

    juce::AudioProcessorValueTreeState& state;
    juce::UndoManager& undo;
    juce::File userDir;
    std::vector<FactoryPreset> factoryPresets;

    std::vector<juce::String> names, categories;
    std::vector<juce::File> userFiles; // index offset by factoryPresets.size()
    int current = 0;

    juce::ValueTree ab[2];
    bool currentIsB = false;
};
} // namespace humpire
