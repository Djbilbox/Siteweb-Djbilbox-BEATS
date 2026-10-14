#pragma once
#include <functional>
#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>

/** Preset d'usine : liste (idParametre, valeur reelle). Les parametres absents reprennent leur valeur par defaut. */
struct HumpirePreset
{
    juce::String category, name;
    std::vector<std::pair<juce::String, float>> values;
};

/** Presets d'usine + utilisateur, navigation < >, A/B. Vit dans le processeur (survit a la fermeture de l'editeur). */
class PresetManager
{
public:
    PresetManager (juce::AudioProcessorValueTreeState& apvts, const juce::String& pluginName);

    void setFactoryPresets (std::vector<HumpirePreset> presets);
    /** Parametres jamais reinitialises par un preset (bypass, gains...). */
    void setPreservedParameters (juce::StringArray ids) { preserved = std::move (ids); }

    int  getNumPresets() const noexcept       { return (int) factory.size() + (int) userFiles.size(); }
    int  getCurrentIndex() const noexcept     { return current; }
    juce::String getCurrentName() const;
    void loadPreset (int index);
    void next();
    void prev();

    /** Sauve l'etat courant comme preset utilisateur -> declenche onOrbitComplete. */
    bool saveUserPreset (const juce::String& name);
    void showMenu (juce::Component* target);

    // A/B
    int  getABSlot() const noexcept { return abSlot; }
    void toggleAB();

    std::function<void()> onPresetChanged;    // UI : rafraichir le nom
    std::function<void()> onOrbitComplete;    // UI : badge "Orbit Complete"
    std::function<void()> onStateRestored;    // processeur : recharger les donnees hors APVTS (ex. courbe graphique)

private:
    void rescanUser();
    juce::File userDir() const;
    void applyFactory (const HumpirePreset&);

    juce::AudioProcessorValueTreeState& apvts;
    juce::String pluginName;
    std::vector<HumpirePreset> factory;
    juce::Array<juce::File> userFiles;
    juce::StringArray preserved;
    int current = 0;
    int abSlot = 0;
    juce::ValueTree abState[2];
};
