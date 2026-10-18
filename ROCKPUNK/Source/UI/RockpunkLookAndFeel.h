#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "Palette.h"

namespace rk
{

//==============================================================================
// Menus and tooltips in the colours of the faceplate. Everything else on the
// editor is artwork, so this is the only look-and-feel there is.
//==============================================================================
class RockpunkLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RockpunkLookAndFeel()
    {
        setColour (juce::PopupMenu::backgroundColourId, Palette::popupBack());
        setColour (juce::PopupMenu::textColourId, Palette::popupText());
        setColour (juce::PopupMenu::headerTextColourId, Palette::pink());
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xFFC03470));
        setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);

        setColour (juce::TooltipWindow::backgroundColourId, juce::Colour (0xF0150F14));
        setColour (juce::TooltipWindow::textColourId, Palette::popupText());
        setColour (juce::TooltipWindow::outlineColourId, Palette::pink().withAlpha (0.8f));
    }

    juce::Font getPopupMenuFont() override { return juce::Font (juce::FontOptions (16.0f)); }

    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                           juce::Rectangle<int> parentArea) override
    {
        const auto text = juce::AttributedString (tipText);
        juce::TextLayout layout;
        juce::AttributedString s;
        s.setJustification (juce::Justification::centredLeft);
        s.append (tipText, juce::Font (juce::FontOptions (14.0f)), Palette::popupText());
        layout.createLayoutWithBalancedLineLengths (s, 360.0f);
        const int w = (int) std::ceil (layout.getWidth()) + 18;
        const int h = (int) std::ceil (layout.getHeight()) + 12;
        juce::ignoreUnused (text);
        return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                     screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 18,
                                     w, h).constrainedWithin (parentArea);
    }

    void drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        const auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
        g.setColour (findColour (juce::TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle (r.reduced (0.5f), 6.0f);
        g.setColour (findColour (juce::TooltipWindow::outlineColourId));
        g.drawRoundedRectangle (r.reduced (1.0f), 6.0f, 1.2f);

        juce::AttributedString s;
        s.setJustification (juce::Justification::centredLeft);
        s.append (text, juce::Font (juce::FontOptions (14.0f)), findColour (juce::TooltipWindow::textColourId));
        juce::TextLayout layout;
        layout.createLayoutWithBalancedLineLengths (s, 360.0f);
        layout.draw (g, r.reduced (9.0f, 6.0f));
    }
};

} // namespace rk
