#include "HumpireLookAndFeel.h"

namespace humpire
{
HumpireLookAndFeel::HumpireLookAndFeel()
{
    using namespace juce;
    setColour (TextButton::buttonColourId,   Colour (0xff2a2a33));
    setColour (TextButton::buttonOnColourId, col::violet);
    setColour (TextButton::textColourOffId,  col::text);
    setColour (TextButton::textColourOnId,   Colours::white);
    setColour (PopupMenu::backgroundColourId,            col::panel);
    setColour (PopupMenu::textColourId,                  col::text);
    setColour (PopupMenu::highlightedBackgroundColourId, col::violet);
    setColour (PopupMenu::highlightedTextColourId,       Colours::white);
    setColour (AlertWindow::backgroundColourId,          col::panel);
    setColour (AlertWindow::textColourId,                col::text);
    setColour (TextEditor::backgroundColourId,           col::black);
    setColour (TextEditor::textColourId,                 col::amber);
    setColour (TextEditor::outlineColourId,              col::textDim);
    setColour (Label::textColourId,                      col::text);
}

juce::Font HumpireLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return Theme::uiFont (11.0f, true);
}

void HumpireLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                           float startAngle, float endAngle, juce::Slider&)
{
    using namespace juce;
    const auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float r  = jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto  c  = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const bool  clean = Theme::isClean();

    // --- anneau de valeur ambre -> orange
    const float arcR = r - 2.0f, thick = jmax (2.5f, r * 0.11f);
    Path track;  track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (col::black.brighter (0.18f));
    g.strokePath (track, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));

    if (pos > 0.001f)
    {
        Path val;  val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, angle, true);
        g.setGradientFill (ColourGradient (col::amber, c.x - arcR, c.y, col::orange, c.x + arcR, c.y, false));
        g.strokePath (val, PathStrokeType (thick, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // --- corps du knob
    const float kr = r * 0.72f;
    const Rectangle<float> kb (c.x - kr, c.y - kr, kr * 2.0f, kr * 2.0f);

    if (clean)
    {
        g.setColour (Colour (0xff22222b));  g.fillEllipse (kb);
        g.setColour (col::textDim);         g.drawEllipse (kb, 1.0f);
    }
    else
    {
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillEllipse (kb.translated (1.0f, 3.0f).expanded (1.0f));                   // ombre portee

        g.setGradientFill (ColourGradient (Colour (0xffe2e2e6), c.x - kr * 0.5f, c.y - kr * 0.8f,
                                           Colour (0xff3c3c44), c.x + kr * 0.7f, c.y + kr, true));
        g.fillEllipse (kb);                                                              // metal

        for (int i = 1; i < 12; ++i)                                                     // stries concentriques (brosse)
        {
            const float rr = kr * (float) i / 12.0f;
            g.setColour (Colours::white.withAlpha ((i % 2) ? 0.07f : 0.02f));
            g.drawEllipse (c.x - rr, c.y - rr, rr * 2.0f, rr * 2.0f, 0.7f);
        }
        g.setColour (Colours::black.withAlpha (0.7f));
        g.drawEllipse (kb, 1.2f);                                                        // liseré

        const auto hl = kb.reduced (kr * 0.25f).withHeight (kr * 0.9f).translated (0.0f, -kr * 0.05f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.38f), hl.getX(), hl.getY(),
                                           Colours::white.withAlpha (0.0f),  hl.getX(), hl.getBottom(), false));
        g.fillEllipse (hl);                                                              // reflet
    }

    // --- repere
    Path p;  p.addRoundedRectangle (-1.6f, -kr * 0.9f, 3.2f, kr * 0.42f, 1.5f);
    p.applyTransform (AffineTransform::rotation (angle).translated (c.x, c.y));
    g.setColour (col::amber);
    g.fillPath (p);
}

void HumpireLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& bg,
                                               bool highlighted, bool down)
{
    using namespace juce;
    const auto r  = b.getLocalBounds().toFloat().reduced (0.5f);
    const float cr = jmin (6.0f, r.getHeight() * 0.5f);
    auto base = bg;
    if (highlighted) base = base.brighter (0.12f);
    if (down)        base = base.darker (0.2f);

    if (Theme::isClean())
    {
        g.setColour (base);                        g.fillRoundedRectangle (r, cr);
        g.setColour (Colours::white.withAlpha (0.12f)); g.drawRoundedRectangle (r, cr, 1.0f);
        return;
    }
    g.setGradientFill (ColourGradient (base.brighter (0.35f), 0.0f, r.getY(),
                                       base.darker (0.35f),   0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, cr);
    g.setColour (Colours::black.withAlpha (0.7f));
    g.drawRoundedRectangle (r, cr, 1.0f);
    g.setColour (Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (r.reduced (1.0f), cr, 1.0f);
}
} // namespace humpire
