#include "ConsoleButton.h"

namespace humpire
{
ConsoleButton::ConsoleButton (const juce::String& n, Shape s, juce::Colour c)
    : juce::Button (n), shape (s), base (c) {}

void ConsoleButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    using namespace juce;
    auto b = getLocalBounds().toFloat().reduced (3.0f);
    if (shape == Shape::round) b = b.withSizeKeepingCentre (jmin (b.getWidth(), b.getHeight()), jmin (b.getWidth(), b.getHeight()));
    const float off = down ? 1.5f : 0.0f;
    auto body = b.translated (0.0f, off);
    const float cr = body.getHeight() * 0.5f;

    auto shapePath = [&] (Rectangle<float> r) { Path p; p.addRoundedRectangle (r, shape == Shape::round ? r.getHeight() * 0.5f : r.getHeight() * 0.5f); return p; };
    auto c = over ? base.brighter (0.1f) : base;

    if (Theme::isClean())
    {
        g.setColour (down ? c.darker (0.3f) : c);  g.fillPath (shapePath (body));
        g.setColour (Colours::white.withAlpha (0.2f)); g.strokePath (shapePath (body), PathStrokeType (1.0f));
    }
    else
    {
        if (! down) { g.setColour (Colours::black.withAlpha (0.55f)); g.fillPath (shapePath (b.translated (0.0f, 3.0f))); }
        g.setGradientFill (ColourGradient (c.brighter (0.5f), 0, body.getY(), c.darker (0.55f), 0, body.getBottom(), false));
        g.fillPath (shapePath (body));
        g.setColour (Colours::black.withAlpha (0.75f));  g.strokePath (shapePath (body), PathStrokeType (1.2f));
        // gloss
        const auto gl = Rectangle<float> (body.getX() + body.getWidth() * 0.14f, body.getY() + body.getHeight() * 0.07f,
                                          body.getWidth() * 0.72f, body.getHeight() * 0.46f);
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.55f), 0, gl.getY(), Colours::white.withAlpha (0.0f), 0, gl.getBottom(), false));
        g.fillPath (shapePath (gl));
    }

    if (glyph.isNotEmpty())
    {
        g.setColour (base.getPerceivedBrightness() > 0.55f ? Colour (0xff1a1a1f) : Colours::white.withAlpha (0.92f));
        g.setFont (Theme::uiFont (body.getHeight() * (shape == Shape::round ? 0.42f : 0.5f), true));
        g.drawText (glyph, body, Justification::centred, false);
    }
    juce::ignoreUnused (cr);

    if (showLed)
    {
        const bool on = getToggleState();
        const auto led = Rectangle<float> (7.0f, 7.0f).withCentre ({ b.getCentreX(), b.getY() - 1.0f });
        if (on) { g.setColour (col::amber.withAlpha (0.35f)); g.fillEllipse (led.expanded (3.0f)); }
        g.setColour (on ? col::amber : Colour (0xff2b2216));  g.fillEllipse (led);
        g.setColour (Colours::black.withAlpha (0.6f));        g.drawEllipse (led, 1.0f);
    }
}

// ============================================================ DPad
void DPad::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto b = getLocalBounds().toFloat().reduced (3.0f);
    const float arm = b.getWidth() / 3.0f;
    const auto c = b.getCentre();

    Path cross;
    cross.addRoundedRectangle (Rectangle<float> (arm, b.getHeight()).withCentre (c), 7.0f);
    cross.addRoundedRectangle (Rectangle<float> (b.getWidth(), arm).withCentre (c), 7.0f);
    cross.setUsingNonZeroWinding (true);

    if (! Theme::isClean())
    {
        g.setColour (Colours::black.withAlpha (0.55f));
        g.fillPath (cross, AffineTransform::translation (0.0f, 3.0f));
    }
    g.setGradientFill (ColourGradient (Colour (0xff4a4a52), b.getX(), b.getY(), Colour (0xff17171c), b.getRight(), b.getBottom(), false));
    g.fillPath (cross);
    g.setColour (Colours::black.withAlpha (0.8f));  g.strokePath (cross, PathStrokeType (1.2f));

    // direction pressee
    if (dirX != 0 || dirY != 0)
    {
        g.setColour (col::amber.withAlpha (0.28f));
        g.fillEllipse (Rectangle<float> (arm * 0.7f, arm * 0.7f).withCentre ({ c.x + (float) dirX * arm, c.y + (float) dirY * arm }));
    }
    // fleches
    g.setColour (Colours::white.withAlpha (0.35f));
    const float t = arm * 0.2f;
    for (auto [dx, dy] : { std::pair { 0, -1 }, std::pair { 0, 1 }, std::pair { -1, 0 }, std::pair { 1, 0 } })
    {
        Path tri;  tri.addTriangle (0.0f, -t, -t, t * 0.8f, t, t * 0.8f);
        const float ang = dx == 0 ? (dy < 0 ? 0.0f : MathConstants<float>::pi) : (dx < 0 ? -MathConstants<float>::halfPi : MathConstants<float>::halfPi);
        g.fillPath (tri, AffineTransform::rotation (ang).translated (c.x + (float) dx * arm, c.y + (float) dy * arm));
    }
    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillEllipse (Rectangle<float> (arm * 0.5f, arm * 0.5f).withCentre (c));                  // creux central
}

void DPad::mouseDown (const juce::MouseEvent& e)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    const float dx = e.position.x - c.x, dy = e.position.y - c.y;
    if (std::abs (dx) < getWidth() * 0.08f && std::abs (dy) < getHeight() * 0.08f) return;      // centre mort
    dirX = dirY = 0;
    if (std::abs (dx) > std::abs (dy)) dirX = dx < 0 ? -1 : 1;  else dirY = dy < 0 ? -1 : 1;
    repeating = false;
    fire();
    startTimer (350);
    repaint();
}

void DPad::mouseUp (const juce::MouseEvent&) { stopTimer(); dirX = dirY = 0; repaint(); }

void DPad::fire() { if (onStep) onStep (dirX, dirY); }

void DPad::timerCallback()
{
    if (! repeating) { repeating = true; startTimer (90); }
    fire();
}
} // namespace humpire
