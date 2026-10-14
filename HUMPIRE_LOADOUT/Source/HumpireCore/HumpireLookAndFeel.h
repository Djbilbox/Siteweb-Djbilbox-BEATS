#pragma once
#include "HumpireTheme.h"

namespace humpire
{
/** LookAndFeel skeuomorphique HUMPIRE (knob metal brosse, boutons pilule). Mode Clean = rendu plat. */
class HumpireLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HumpireLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;

    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
};
} // namespace humpire
