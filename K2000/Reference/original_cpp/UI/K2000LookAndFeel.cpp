#include "K2000LookAndFeel.h"

namespace K2000UI
{
    const juce::Colour K2000LookAndFeel::neonCyan     = juce::Colour(0x00, 0xF0, 0xFF);
    const juce::Colour K2000LookAndFeel::neonCyanDim  = juce::Colour(0x00, 0x50, 0x60);
    const juce::Colour K2000LookAndFeel::chromeLight  = juce::Colour(0xEA, 0xF0, 0xF6);
    const juce::Colour K2000LookAndFeel::chromeMid    = juce::Colour(0x98, 0xA6, 0xB5);
    const juce::Colour K2000LookAndFeel::chromeDark   = juce::Colour(0x38, 0x42, 0x4D);
    const juce::Colour K2000LookAndFeel::chassisBg    = juce::Colour(0x16, 0x19, 0x1D);
    const juce::Colour K2000LookAndFeel::recessedBg   = juce::Colour(0x0E, 0x10, 0x13);

    K2000LookAndFeel::K2000LookAndFeel()
    {
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xCC, 0xDD, 0xEE));
        setColour(juce::ComboBox::textColourId, neonCyan);
        setColour(juce::PopupMenu::backgroundColourId, recessedBg);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0x00, 0x77, 0x99));
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    }

    void K2000LookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPosProportional, float rotaryStartAngle,
                                           float rotaryEndAngle, juce::Slider& /*slider*/)
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto centre = bounds.getCentre();

        // 1. Dark Recessed Bezel
        g.setColour(recessedBg);
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colour(0x28, 0x30, 0x3A));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);

        // 2. Glowing Neon Cyan Arc
        juce::Path trackPath;
        trackPath.addCentredArc(centre.x, centre.y, radius - 2.5f, radius - 2.5f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(neonCyanDim.withAlpha(0.4f));
        g.strokePath(trackPath, juce::PathStrokeType(2.5f));

        juce::Path valuePath;
        valuePath.addCentredArc(centre.x, centre.y, radius - 2.5f, radius - 2.5f, 0.0f, rotaryStartAngle, toAngle, true);
        g.setColour(neonCyan);
        g.strokePath(valuePath, juce::PathStrokeType(2.5f));

        // 3. Brushed Chrome Cap
        auto knobRadius = radius - 6.0f;
        juce::ColourGradient grad(chromeLight, centre.x - knobRadius, centre.y - knobRadius,
                                  chromeDark, centre.x + knobRadius, centre.y + knobRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

        // Inner knurled bevel ring
        g.setColour(chromeMid);
        g.drawEllipse(centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.2f);
        g.drawEllipse(centre.x - knobRadius + 2.5f, centre.y - knobRadius + 2.5f, (knobRadius - 2.5f) * 2.0f, (knobRadius - 2.5f) * 2.0f, 0.8f);

        // 4. White Indicator Notch
        juce::Path p;
        auto pointerLength = knobRadius * 0.75f;
        p.addRectangle(-1.2f, -knobRadius + 2.0f, 2.4f, pointerLength * 0.5f);
        p.applyTransform(juce::AffineTransform::rotation(toAngle).translated(centre.x, centre.y));
        g.setColour(juce::Colours::white);
        g.fillPath(p);
    }

    void K2000LookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                           const juce::Slider::SliderStyle /*style*/, juce::Slider& /*slider*/)
    {
        auto trackX = x + width / 2.0f;
        auto trackY = static_cast<float>(y + 4);
        auto trackH = static_cast<float>(height - 8);

        // 1. Recessed Fader Slot
        g.setColour(recessedBg);
        g.fillRoundedRectangle(trackX - 3.0f, trackY, 6.0f, trackH, 2.0f);
        g.setColour(juce::Colour(0x28, 0x30, 0x3A));
        g.drawRoundedRectangle(trackX - 3.0f, trackY, 6.0f, trackH, 2.0f, 1.0f);

        // 2. Dashed Cyan LED Markings on sides
        int numDashes = 9;
        for (int i = 0; i < numDashes; ++i)
        {
            float dashY = trackY + (trackH / static_cast<float>(numDashes - 1)) * static_cast<float>(i);
            bool isLit = dashY >= sliderPos;
            g.setColour(isLit ? neonCyan : neonCyanDim.withAlpha(0.4f));

            // Left tick & right tick
            g.drawLine(trackX - 8.0f, dashY, trackX - 5.0f, dashY, 1.0f);
            g.drawLine(trackX + 5.0f, dashY, trackX + 8.0f, dashY, 1.0f);
        }

        // 3. Metallic Slider Cap
        float capW = 22.0f;
        float capH = 9.0f;
        float capX = trackX - capW / 2.0f;
        float capY = sliderPos - capH / 2.0f;

        juce::ColourGradient capGrad(chromeLight, capX, capY, chromeDark, capX, capY + capH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(capX, capY, capW, capH, 2.0f);

        g.setColour(chromeMid);
        g.drawRoundedRectangle(capX, capY, capW, capH, 2.0f, 1.0f);

        // Glowing center line on cap
        g.setColour(neonCyan);
        g.drawLine(capX + 3.0f, sliderPos, capX + capW - 3.0f, sliderPos, 1.5f);
    }

    void K2000LookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                           bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
    {
        auto bounds = button.getLocalBounds().toFloat();
        auto centre = bounds.getCentre();
        bool isOn = button.getToggleState();

        // 1. Switch Bezel Ring
        g.setColour(recessedBg);
        g.fillEllipse(centre.x - 12.0f, centre.y - 12.0f, 24.0f, 24.0f);
        g.setColour(juce::Colour(0x38, 0x44, 0x50));
        g.drawEllipse(centre.x - 12.0f, centre.y - 12.0f, 24.0f, 24.0f, 1.5f);

        // 2. LED Indicator
        g.setColour(isOn ? neonCyan : juce::Colour(0x22, 0x30, 0x38));
        g.fillEllipse(centre.x - 3.0f, centre.y - 18.0f, 6.0f, 6.0f);
        if (isOn)
        {
            g.setColour(neonCyan.withAlpha(0.4f));
            g.drawEllipse(centre.x - 5.0f, centre.y - 20.0f, 10.0f, 10.0f, 1.5f);
        }

        // 3. Metallic Bat Handle
        float leverY = isOn ? centre.y - 7.0f : centre.y + 7.0f;
        g.setColour(chromeMid);
        g.drawLine(centre.x, centre.y, centre.x, leverY, 4.0f);

        juce::ColourGradient knobGrad(chromeLight, centre.x - 5.0f, leverY - 5.0f,
                                     chromeDark, centre.x + 5.0f, leverY + 5.0f, false);
        g.setGradientFill(knobGrad);
        g.fillEllipse(centre.x - 5.0f, leverY - 5.0f, 10.0f, 10.0f);
    }

    void K2000LookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                       int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                       juce::ComboBox& /*box*/)
    {
        auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

        // Recessed dark frame
        g.setColour(recessedBg);
        g.fillRoundedRectangle(bounds, 3.0f);

        // Glowing Neon Cyan Border
        g.setColour(neonCyan);
        g.drawRoundedRectangle(bounds, 3.0f, 1.2f);

        // Dropdown Arrow
        juce::Path arrow;
        float ax = width - 12.0f;
        float ay = height * 0.5f - 2.0f;
        arrow.addTriangle(ax - 4.0f, ay, ax + 4.0f, ay, ax, ay + 5.0f);
        g.setColour(neonCyan);
        g.fillPath(arrow);
    }

    void K2000LookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
    {
        g.setColour(recessedBg);
        g.fillRect(0, 0, width, height);

        g.setColour(neonCyan);
        g.drawRect(0, 0, width, height, 1);
    }

    void K2000LookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                            bool isHighlighted, bool /*isTicked*/, bool /*hasSubMenu*/,
                                            const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                            const juce::Drawable* /*icon*/, const juce::Colour* /*textColour*/)
    {
        if (isHighlighted)
        {
            g.setColour(juce::Colour(0x00, 0x66, 0x88));
            g.fillRect(area);
        }

        g.setColour(isHighlighted ? juce::Colours::white : juce::Colour(0xCC, 0xEE, 0xFF));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(text, area.reduced(8, 0), juce::Justification::centredLeft, true);
    }
}
