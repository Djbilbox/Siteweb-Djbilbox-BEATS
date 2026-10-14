#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Palette.h"
#include "WarzoneLookAndFeel.h"

namespace wz
{

//==============================================================================
// Level fader (0..1), horizontal or vertical. The track is a direct crop of the
// artwork (WarzoneLookAndFeel::drawSliderSprite) with a translucent neon fill
// over the active portion. Same glow behaviour as the knobs while a note is
// triggering. In vertical mode no label text is drawn (the artwork carries its
// own labels) - it is also used for the pitch / mod wheels.
//==============================================================================
class LevelSlider final : public juce::Component
{
public:
    LevelSlider (const juce::String& labelText, WarzoneLookAndFeel& lfRef,
                 const juce::String& spriteKey, bool vertical = false)
        : levelLabel (labelText),
          artwork (lfRef),
          key (spriteKey),
          isVertical (vertical)
    {
        slider.setSliderStyle (isVertical ? juce::Slider::LinearVertical
                                          : juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (&sliderLnF);
        addAndMakeVisible (slider);
    }

    juce::Slider slider;

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void resized() override { slider.setBounds (getLocalBounds()); }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        const float v = (float) slider.getNormalisableRange().convertTo0to1 (slider.getValue());

        if (glow > 0.02f)
        {
            g.setColour (Palette::neonRed().withAlpha (0.30f * glow));
            g.fillRoundedRectangle (r.expanded (2.0f), 5.0f);
        }

        if (artwork.hasSprite (key))
            artwork.drawSliderSprite (g, key, v, getLocalBounds());
        else
            drawFallback (g, r, v);

        if (! isVertical)
        {
            g.setColour (glow > 0.02f ? Palette::neonGreen() : Palette::textDim());
            g.setFont (juce::Font (9.0f).boldened());
            g.drawText (levelLabel + "  " + juce::String ((int) (v * 100.0f)) + "%",
                        juce::Rectangle<float> (r.getX() + 4.0f, r.getY() + 1.0f,
                                                r.getWidth() - 8.0f, r.getHeight() - 2.0f),
                        juce::Justification::centred);
        }
    }

private:
    void drawFallback (juce::Graphics& g, const juce::Rectangle<float>& r, float v) const
    {
        g.setColour (Palette::off());
        g.fillRoundedRectangle (r, 4.0f);

        juce::ColourGradient fillGrad (Palette::red(), r.getX(), r.getY(),
                                       Palette::redDeep(), r.getRight(), r.getY(), false);
        g.setGradientFill (fillGrad);

        if (isVertical)
            g.fillRoundedRectangle (juce::Rectangle<float> (r.getX(), r.getBottom() - r.getHeight() * v,
                                                            r.getWidth(), r.getHeight() * v), 4.0f);
        else
            g.fillRoundedRectangle (juce::Rectangle<float> (r.getX(), r.getY(),
                                                            r.getWidth() * v, r.getHeight()), 4.0f);

        g.setColour (Palette::chrome());
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }

    BlankSliderLookAndFeel sliderLnF;
    WarzoneLookAndFeel& artwork;
    juce::String levelLabel;
    juce::String key;
    bool isVertical = false;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace wz