#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <utility>
#include <vector>

#include "Palette.h"
#include "WarzoneLookAndFeel.h"

namespace wz
{

//==============================================================================
// Cycling switch for int / bool parameters (e.g. drive type, layer on/off).
// The graphic comes straight from the artwork: WarzoneLookAndFeel crops the
// real switch and rotates it into a filmstrip of N states. The state name is
// overlaid, and the whole thing glows neon while a note is triggering.
// No Attachment: it drives the raw AudioProcessorParameter directly and is
// re-synced from the host every UI tick (timerCallback in the editor).
//==============================================================================
class CyclingSwitch final : public juce::Component
{
public:
    CyclingSwitch (const juce::String& labelText, juce::AudioProcessorParameter& p,
                   std::vector<juce::String> names, WarzoneLookAndFeel& lfRef,
                   const juce::String& spriteKey)
        : label (labelText),
          param (p),
          stateNames (std::move (names)),
          artwork (lfRef),
          key (spriteKey)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void syncFromParam() noexcept { value01 = param.getValue(); }
    void timerTick() noexcept
    {
        glow += ((glowTarget - glow) * 0.18f);
        if (std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat();
        const bool lit = value01 > 0.5f;
        const float glowLevel = juce::jmax (glow, lit ? 0.35f : 0.0f);

        if (glowLevel > 0.02f)
        {
            g.setColour (Palette::neonRed().withAlpha (0.30f * glowLevel));
            g.fillRoundedRectangle (r.expanded (2.0f), 4.0f);
        }

        if (artwork.hasSprite (key))
            artwork.drawSwitchSprite (g, key, value01, getLocalBounds());
        else
            drawFallback (g, r, lit, glowLevel);

        const int idx = stateNames.size() > 1
            ? juce::jlimit (0, (int) stateNames.size() - 1,
                            (int) (value01 * (float) (stateNames.size() - 1) + 0.5f))
            : 0;
        const juce::String txt = idx < (int) stateNames.size() ? stateNames[(size_t) idx] : label;

        g.setColour (glowLevel > 0.02f ? Palette::neonRed() : (lit ? Palette::text() : Palette::textDim()));
        g.setFont (juce::Font (9.5f).boldened());
        g.drawText (txt, r, juce::Justification::centred);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        const int n = (int) stateNames.size();
        if (n < 2)
            return;

        float next = value01 + 1.0f / (float) (n - 1);
        if (next > 1.0f)
            next = 0.0f;

        param.beginChangeGesture();
        param.setValueNotifyingHost (next);
        param.endChangeGesture();
        value01 = next;
        repaint();
    }

private:
    void drawFallback (juce::Graphics& g, const juce::Rectangle<float>& r, bool lit, float glowLevel) const
    {
        g.setColour (lit ? Palette::red() : Palette::off());
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (glowLevel > 0.02f ? Palette::neonGreen() : Palette::chrome());
        g.drawRoundedRectangle (r, 4.0f, 1.2f);
    }

    juce::String label;
    juce::AudioProcessorParameter& param;
    std::vector<juce::String> stateNames;
    WarzoneLookAndFeel& artwork;
    juce::String key;
    float value01 = 0.0f;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace wz