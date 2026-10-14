#include "CassetteSlot.h"
#include "K2000LookAndFeel.h"

namespace K2000UI
{
    CassetteSlot::CassetteSlot()
    {
    }

    void CassetteSlot::mouseEnter(const juce::MouseEvent& /*event*/)
    {
        isHovered = true;
        repaint();
    }

    void CassetteSlot::mouseExit(const juce::MouseEvent& /*event*/)
    {
        isHovered = false;
        repaint();
    }

    void CassetteSlot::mouseDown(const juce::MouseEvent& /*event*/)
    {
        if (onCassetteClicked)
            onCassetteClicked();
    }

    void CassetteSlot::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        auto bayBounds = bounds.withTrimmedBottom(18.0f);

        // 1. Recessed Bay Frame with Cyan Glow
        g.setColour(K2000LookAndFeel::recessedBg);
        g.fillRoundedRectangle(bayBounds, 6.0f);

        g.setColour(isHovered ? K2000LookAndFeel::neonCyan : K2000LookAndFeel::neonCyan.withAlpha(0.65f));
        g.drawRoundedRectangle(bayBounds, 6.0f, 1.8f);

        // 2. Vintage Cassette Body
        auto casBounds = bayBounds.reduced(6.0f, 6.0f);
        juce::ColourGradient casGrad(juce::Colour(0x32, 0x38, 0x40), casBounds.getX(), casBounds.getY(),
                                   juce::Colour(0x18, 0x1C, 0x22), casBounds.getX(), casBounds.getBottom(), false);
        g.setGradientFill(casGrad);
        g.fillRoundedRectangle(casBounds, 4.0f);

        // 3. Vintage Label Paper (Beige / Cream)
        auto labelBounds = casBounds.reduced(8.0f, 12.0f);
        g.setColour(juce::Colour(0xD4, 0xC8, 0xB4));
        g.fillRoundedRectangle(labelBounds, 2.0f);

        // Label Text & Branding
        g.setColour(juce::Colour(0x15, 0x18, 0x20));
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.drawText("A", labelBounds.removeFromLeft(14.0f), juce::Justification::centred, false);

        g.setFont(juce::Font(11.0f, juce::Font::bold));
        g.drawText("VOL.1", labelBounds.removeFromRight(36.0f), juce::Justification::centred, false);

        // 4. Central Tape Window & Reels
        auto winBounds = labelBounds.reduced(4.0f, 2.0f);
        g.setColour(juce::Colour(0x1A, 0x1D, 0x24));
        g.fillRoundedRectangle(winBounds, 2.0f);

        // Two mini tape spools
        float spoolR = 5.0f;
        g.setColour(juce::Colour(0x8A, 0x96, 0xA2));
        g.fillEllipse(winBounds.getCentreX() - 14.0f - spoolR, winBounds.getCentreY() - spoolR, spoolR * 2.0f, spoolR * 2.0f);
        g.fillEllipse(winBounds.getCentreX() + 14.0f - spoolR, winBounds.getCentreY() - spoolR, spoolR * 2.0f, spoolR * 2.0f);

        // 5. Chrome Latch Hinge on the right
        auto hingeRect = juce::Rectangle<float>(bayBounds.getRight() - 7.0f, bayBounds.getCentreY() - 12.0f, 7.0f, 24.0f);
        g.setColour(K2000LookAndFeel::chromeMid);
        g.fillRoundedRectangle(hingeRect, 2.0f);
        g.setColour(K2000LookAndFeel::chromeLight);
        g.drawRoundedRectangle(hingeRect, 2.0f, 1.0f);

        // 6. Label Underneath
        g.setColour(juce::Colour(0xAA, 0xBB, 0xCC));
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.drawText("EXPANSION", bounds.removeFromBottom(16.0f), juce::Justification::centred, false);
    }
}
