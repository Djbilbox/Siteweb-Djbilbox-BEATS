#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <memory>

#include "UI/RockpunkLookAndFeel.h"

namespace rk
{

class PluginProcessor;
struct Art;
class Panel;

//==============================================================================
// The editor is a fixed 1000 x 746 panel that the window scales: the faceplate is
// the mockup, the controls are drawn and hit-tested in the mockup's own pixels
// (see UI/Art.h), so nothing can sit beside the thing it is meant to sit on.
//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void rebuildPlate (int width, int height);

    PluginProcessor& proc;
    RockpunkLookAndFeel lnf;
    std::unique_ptr<Art> art;
    std::unique_ptr<Panel> panel;
    std::unique_ptr<juce::TooltipWindow> tooltips;

    // The faceplate resampled to the size it is shown at (physical pixels), so
    // that it is drawn 1:1 and not scaled again on every repaint.
    juce::Image scaledPlate;
    int pendingW = 0, pendingH = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};

} // namespace rk
