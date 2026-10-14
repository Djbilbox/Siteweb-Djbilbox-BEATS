#pragma once
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "LevelMeter.h"
#include "PresetManager.h"

/** Barre superieure identique sur tous les plugins : logo, presets < >, A/B, Undo/Redo, Bypass, In/Out. */
class HumpireTopBar : public juce::Component, private juce::Timer
{
public:
    HumpireTopBar (juce::AudioProcessorValueTreeState&, PresetManager&, const juce::String& productName,
                   const std::atomic<float>& inLevel, const std::atomic<float>& outLevel);
    ~HumpireTopBar() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

    /** Affiche le badge "ORBIT COMPLETE" (preset sauvegarde). */
    void showOrbitComplete();
    void refreshPresetName();

private:
    void timerCallback() override;
    void askSaveName();

    juce::AudioProcessorValueTreeState& apvts;
    PresetManager& presets;
    juce::String product;

    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, nameBtn, saveBtn { "SAVE" },
                     abBtn { "A" }, undoBtn { "UNDO" }, redoBtn { "REDO" };
    juce::ToggleButton bypassBtn { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttach;
    LevelMeter inMeter, outMeter;

    juce::int64 badgeStart = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HumpireTopBar)
};
