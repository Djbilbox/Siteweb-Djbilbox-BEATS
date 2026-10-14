#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Palette.h"
#include "WarzoneLookAndFeel.h"

namespace wz
{

//==============================================================================
// Stereo VU meter (one instance per channel). The meter body is a direct crop
// of the artwork; a translucent neon/red fill shows the live level. Reads the
// processor's smoothed output level and turns rose/pink in the top of the range
// or on note energy.
//==============================================================================
class VuMeter final : public juce::Component
{
public:
    VuMeter (bool left, WarzoneLookAndFeel& lfRef, const juce::String& spriteKey)
        : isLeft (left),
          artwork (lfRef),
          key (spriteKey)
    {
    }

    void setLevel (float l01) noexcept { level = juce::jlimit (0.0f, 1.0f, l01); }
    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();

        if (glow > 0.02f)
        {
            g.setColour (Palette::neonGreen().withAlpha (0.25f * glow));
            g.fillRoundedRectangle (r.expanded (2.0f), 4.0f);
        }

        if (artwork.hasSprite (key))
        {
            artwork.drawCropSprite (g, key, getLocalBounds());
            g.setColour (Palette::chrome());
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }
        else
        {
            g.setColour (Palette::off());
            g.fillRoundedRectangle (r, 4.0f);
        }

        const float h = r.getHeight() * level;
        const auto lit = juce::Rectangle<float> (r.getX(), r.getBottom() - h, r.getWidth(), h);

        g.setColour ((level > 0.85f ? Palette::neonRed()
                                    : (glow > 0.02f ? Palette::neonGreen() : Palette::red()))
                         .withAlpha (artwork.hasSprite (key) ? 0.85f : 1.0f));
        g.fillRoundedRectangle (lit, 4.0f);
    }

private:
    bool isLeft = true;
    WarzoneLookAndFeel& artwork;
    juce::String key;
    float level = 0.0f;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace wz