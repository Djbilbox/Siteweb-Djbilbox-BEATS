#pragma once
#include "HumpireTheme.h"
#include <vector>

namespace humpire
{
/** Preset d'usine : valeurs en unites reelles (ex. bits = 8.0, rate = 12000). */
struct FactoryPreset
{
    juce::String name, category;
    std::vector<std::pair<juce::String, float>> values;
};

/** Presets (usine + utilisateur XML), navigation < >, A/B. Thread message uniquement. */
class PresetManager : public juce::ChangeBroadcaster
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& apvts, const juce::String& pluginName,
                   std::vector<FactoryPreset> factory,
                   juce::StringArray preservedIds = { "bypass", "simple", "clean" });

    int  getNumPresets() const noexcept      { return (int) entries.size(); }
    int  getCurrentIndex() const noexcept    { return current; }
    juce::String getCurrentName() const;

    void load (int index);
    void next()  { if (! entries.empty()) load ((current + 1) % getNumPresets()); }
    void prev()  { if (! entries.empty()) load ((current - 1 + getNumPresets()) % getNumPresets()); }

    bool saveUserPreset (const juce::String& name);
    void rescanUser();

    /** Menu groupe par categorie ; item id = baseId + index. */
    juce::PopupMenu buildMenu (int baseId) const;

    // ---- comparaison A/B
    void toggleAB();
    bool isB() const noexcept { return onB; }

private:
    struct Entry { juce::String name, category; std::vector<std::pair<juce::String, float>> values; juce::File file; };

    juce::File userDir() const;
    void applyFactory (const Entry&);
    void applyUser (const juce::File&);

    juce::AudioProcessorValueTreeState& apvts;
    juce::String pluginName;
    juce::StringArray preserved;
    std::vector<Entry> entries;
    size_t numFactory = 0;
    int current = 0;
    bool onB = false;
    juce::ValueTree slotA, slotB;
};
} // namespace humpire
