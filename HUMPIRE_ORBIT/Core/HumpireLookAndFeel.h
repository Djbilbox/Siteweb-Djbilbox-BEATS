#pragma once
#include "HumpireTheme.h"

/** LookAndFeel commun a tous les plugins HUMPIRE : knobs "orbite", boutons pilules, combos cosmiques. */
class HumpireLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HumpireLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool down, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int height) override;
    juce::Font getLabelFont (juce::Label&) override;

    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;

    /** Halo lumineux reutilisable. */
    static void drawGlowDot (juce::Graphics&, juce::Point<float> c, float radius, juce::Colour col);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HumpireLookAndFeel)
};
