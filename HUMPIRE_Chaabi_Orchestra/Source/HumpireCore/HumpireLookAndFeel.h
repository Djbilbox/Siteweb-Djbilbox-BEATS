#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireTheme.h"

namespace humpire
{
// LookAndFeel premium : knobs a arc lumineux, boutons pilule or, combos sobres.
class HumpireLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HumpireLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool down, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
};
} // namespace humpire
