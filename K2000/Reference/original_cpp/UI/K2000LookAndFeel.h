#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace K2000UI
{
    class K2000LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        K2000LookAndFeel();
        ~K2000LookAndFeel() override = default;

        // Custom Rotary Slider Drawing (Matches K2000 Brushed Chrome & Neon Cyan Arc)
        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider& slider) override;

        // Custom Linear Fader Drawing (Matches K2000 Metallic Cap & Dashed Cyan Track)
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override;

        // Custom Toggle Switch (Matches K2000 PITCH 3D toggle & illuminated cyan LED)
        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        // Custom Dropdown / ComboBox
        void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                          int buttonX, int buttonY, int buttonW, int buttonH,
                          juce::ComboBox& box) override;

        void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;
        void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                               bool isHighlighted, bool isTicked, bool hasSubMenu,
                               const juce::String& text, const juce::String& shortcutKeyText,
                               const juce::Drawable* icon, const juce::Colour* textColour) override;

        // Color Constants
        static const juce::Colour neonCyan;
        static const juce::Colour neonCyanDim;
        static const juce::Colour chromeLight;
        static const juce::Colour chromeMid;
        static const juce::Colour chromeDark;
        static const juce::Colour chassisBg;
        static const juce::Colour recessedBg;
    };
}
