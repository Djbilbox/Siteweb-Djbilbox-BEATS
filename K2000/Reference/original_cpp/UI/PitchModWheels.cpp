#include "PitchModWheels.h"
#include "K2000LookAndFeel.h"

namespace K2000UI
{
    PitchModWheels::PitchModWheels()
    {
    }

    void PitchModWheels::resized()
    {
    }

    void PitchModWheels::setModWheelValue(float val)
    {
        modValue = std::max(0.0f, std::min(val, 1.0f));
        repaint();
    }

    void PitchModWheels::drawWheel(juce::Graphics& g, const juce::Rectangle<float>& bounds, float value, bool isPitch, const juce::String& label)
    {
        auto wheelBounds = bounds.withTrimmedBottom(18.0f).reduced(4.0f, 2.0f);

        // 1. Recessed Wheel Well
        g.setColour(K2000LookAndFeel::recessedBg);
        g.fillRoundedRectangle(wheelBounds, 4.0f);
        g.setColour(juce::Colour(0x2A, 0x34, 0x40));
        g.drawRoundedRectangle(wheelBounds, 4.0f, 1.5f);

        // 2. Ribbed Wheel Cylinder
        auto innerBounds = wheelBounds.reduced(3.0f, 4.0f);
        juce::ColourGradient wheelGrad(juce::Colour(0x35, 0x3E, 0x48), innerBounds.getX(), innerBounds.getY(),
                                    juce::Colour(0x10, 0x14, 0x18), innerBounds.getX(), innerBounds.getBottom(), false);
        g.setGradientFill(wheelGrad);
        g.fillRoundedRectangle(innerBounds, 3.0f);

        // Rib grooves
        int numRibs = 14;
        float offset = (isPitch ? value : (value * 2.0f - 1.0f)) * 8.0f;
        for (int i = 0; i < numRibs; ++i)
        {
            float y = innerBounds.getY() + (innerBounds.getHeight() / numRibs) * i + std::fmod(offset, innerBounds.getHeight() / numRibs);
            if (y >= innerBounds.getY() && y <= innerBounds.getBottom())
            {
                g.setColour(juce::Colour(0x15, 0x1A, 0x20));
                g.drawLine(innerBounds.getX() + 2.0f, y, innerBounds.getRight() - 2.0f, y, 1.5f);
                g.setColour(juce::Colour(0x55, 0x62, 0x70));
                g.drawLine(innerBounds.getX() + 2.0f, y + 1.0f, innerBounds.getRight() - 2.0f, y + 1.0f, 0.8f);
            }
        }

        // 3. Glowing Neon Cyan Center Index Line
        float centerY = isPitch
            ? innerBounds.getCentreY() - value * (innerBounds.getHeight() * 0.4f)
            : innerBounds.getBottom() - value * innerBounds.getHeight();

        centerY = std::max(innerBounds.getY() + 2.0f, std::min(centerY, innerBounds.getBottom() - 2.0f));

        g.setColour(K2000LookAndFeel::neonCyan);
        g.drawLine(innerBounds.getX() + 4.0f, centerY, innerBounds.getRight() - 4.0f, centerY, 2.5f);

        // Lateral cyan glow strips
        g.setColour(K2000LookAndFeel::neonCyan.withAlpha(0.6f));
        g.drawLine(wheelBounds.getX(), wheelBounds.getY() + 4.0f, wheelBounds.getX(), wheelBounds.getBottom() - 4.0f, 1.5f);
        g.drawLine(wheelBounds.getRight(), wheelBounds.getY() + 4.0f, wheelBounds.getRight(), wheelBounds.getBottom() - 4.0f, 1.5f);

        // 4. Label below wheel
        g.setColour(juce::Colour(0xAA, 0xBB, 0xCC));
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.drawText(label, bounds.removeFromBottom(16.0f), juce::Justification::centred, false);
    }

    void PitchModWheels::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        auto wheelW = bounds.getWidth() * 0.5f;

        drawWheel(g, bounds.removeFromLeft(wheelW), pitchValue, true, "PITCH BEND");
        drawWheel(g, bounds, modValue, false, "MODULATION");
    }

    void PitchModWheels::mouseDown(const juce::MouseEvent& event)
    {
        float halfW = getWidth() * 0.5f;
        dragStartY = static_cast<float>(event.y);

        if (event.x < halfW)
        {
            isDraggingPitch = true;
            initialVal = pitchValue;
        }
        else
        {
            isDraggingMod = true;
            initialVal = modValue;
        }
    }

    void PitchModWheels::mouseDrag(const juce::MouseEvent& event)
    {
        float deltaY = dragStartY - static_cast<float>(event.y);
        float sensitivity = 0.015f;

        if (isDraggingPitch)
        {
            pitchValue = std::max(-1.0f, std::min(initialVal + deltaY * sensitivity, 1.0f));
            if (onPitchBendChanged) onPitchBendChanged(pitchValue);
            repaint();
        }
        else if (isDraggingMod)
        {
            modValue = std::max(0.0f, std::min(initialVal + deltaY * sensitivity, 1.0f));
            if (onModWheelChanged) onModWheelChanged(modValue);
            repaint();
        }
    }

    void PitchModWheels::mouseUp(const juce::MouseEvent& /*event*/)
    {
        if (isDraggingPitch)
        {
            isDraggingPitch = false;
            // Spring-Back to center (0.0)
            pitchValue = 0.0f;
            if (onPitchBendChanged) onPitchBendChanged(0.0f);
            repaint();
        }
        isDraggingMod = false;
    }
}
