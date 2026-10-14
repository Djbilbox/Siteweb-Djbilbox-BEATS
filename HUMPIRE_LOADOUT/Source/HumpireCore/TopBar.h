#pragma once
#include "HumpireTheme.h"
#include "LedMeter.h"
#include "PresetManager.h"

namespace humpire
{
/** Barre fonctionnelle commune aux 8 plugins : logo, presets < >, SAVE, A/B, UNDO/REDO, BYPASS, meters In/Out.
    Largeur de reference 760 px, hauteur 40 px (le parent est mis a l'echelle par ScaledEditor).
    Requiert un parametre bool "bypass" dans l'APVTS. */
class TopBar : public juce::Component, private juce::ChangeListener, private juce::Timer
{
public:
    static constexpr int height = 40;

    TopBar (juce::AudioProcessorValueTreeState&, PresetManager&, juce::UndoManager&,
            std::atomic<float>& inPeak, std::atomic<float>& outPeak, const juce::String& pluginTitle);
    ~TopBar() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void changeListenerCallback (juce::ChangeBroadcaster*) override { refresh(); }
    void timerCallback() override;
    void refresh();
    void showPresetMenu();
    void promptSave();

    juce::AudioProcessorValueTreeState& apvts;
    PresetManager& presets;
    juce::UndoManager& undo;
    juce::String title;

    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, nameBtn, saveBtn { "SAVE" },
                     abBtn { "A" }, undoBtn { "UNDO" }, redoBtn { "REDO" }, bypassBtn { "BYPASS" };
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassAttach;
    LedMeter inMeter, outMeter;
    juce::Rectangle<int> inLabel, outLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopBar)
};
} // namespace humpire
