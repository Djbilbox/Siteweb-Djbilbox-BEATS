#include "XYPad.h"

namespace humpire
{
XYPad::XYPad (juce::AudioProcessorValueTreeState& s, const juce::String& xId, const juce::String& yId,
              bool round, bool springBack)
    : px (s.getParameter (xId)), py (s.getParameter (yId)),
      ax (*px, [this] (float v) { nx = px->convertTo0to1 (v); repaint(); }, s.undoManager),
      ay (*py, [this] (float v) { ny = py->convertTo0to1 (v); repaint(); }, s.undoManager),
      isRound (round), spring (springBack)
{
    ax.sendInitialUpdate();
    ay.sendInitialUpdate();
}

void XYPad::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto b = getLocalBounds().toFloat().reduced (2.0f);
    const bool clean = Theme::isClean();

    // puits
    g.setGradientFill (ColourGradient (Colour (0xff050507), b.getCentreX(), b.getY(),
                                       Colour (0xff1d1d25), b.getCentreX(), b.getBottom(), false));
    if (isRound) g.fillEllipse (b); else g.fillRoundedRectangle (b, 8.0f);
    g.setColour (Colours::white.withAlpha (0.12f));
    if (isRound) g.drawEllipse (b, 1.5f); else g.drawRoundedRectangle (b, 8.0f, 1.5f);

    // reticule
    g.setColour (Colours::white.withAlpha (0.07f));
    g.drawLine (b.getCentreX(), b.getY() + 4, b.getCentreX(), b.getBottom() - 4);
    g.drawLine (b.getX() + 4, b.getCentreY(), b.getRight() - 4, b.getCentreY());

    // tete du stick (anneau lumineux ambre)
    const float tr = jmin (b.getWidth(), b.getHeight()) * 0.17f;
    const float tx = b.getX() + tr + nx * (b.getWidth()  - 2 * tr);
    const float ty = b.getY() + tr + (1.0f - ny) * (b.getHeight() - 2 * tr);
    if (! clean)
    {
        g.setColour (col::amber.withAlpha (0.25f));  g.fillEllipse (tx - tr * 1.35f, ty - tr * 1.35f, tr * 2.7f, tr * 2.7f);
        g.setColour (Colours::black.withAlpha (0.6f)); g.fillEllipse (tx - tr + 1, ty - tr + 3, tr * 2, tr * 2);
    }
    g.setGradientFill (ColourGradient (Colour (0xff6a6a72), tx - tr * 0.5f, ty - tr * 0.7f,
                                       Colour (0xff17171c), tx + tr * 0.6f, ty + tr, true));
    g.fillEllipse (tx - tr, ty - tr, tr * 2, tr * 2);
    g.setColour (col::amber);  g.drawEllipse (tx - tr, ty - tr, tr * 2, tr * 2, 2.5f);
    g.setColour (Colours::white.withAlpha (0.25f));
    g.fillEllipse (tx - tr * 0.45f, ty - tr * 0.75f, tr * 0.9f, tr * 0.5f);
}

void XYPad::setFromMouse (const juce::MouseEvent& e)
{
    const auto b = getLocalBounds().toFloat().reduced (2.0f);
    const float tr = juce::jmin (b.getWidth(), b.getHeight()) * 0.17f;
    nx = juce::jlimit (0.0f, 1.0f, (e.position.x - b.getX() - tr) / (b.getWidth()  - 2 * tr));
    ny = juce::jlimit (0.0f, 1.0f, 1.0f - (e.position.y - b.getY() - tr) / (b.getHeight() - 2 * tr));
    ax.setValueAsPartOfGesture (px->convertFrom0to1 (nx));
    ay.setValueAsPartOfGesture (py->convertFrom0to1 (ny));
    repaint();
}

void XYPad::mouseDown (const juce::MouseEvent& e)  { ax.beginGesture(); ay.beginGesture(); setFromMouse (e); }
void XYPad::mouseDrag (const juce::MouseEvent& e)  { setFromMouse (e); }

void XYPad::mouseUp (const juce::MouseEvent&)
{
    if (spring) resetToDefault();
    ax.endGesture();  ay.endGesture();
}

void XYPad::resetToDefault()
{
    ax.setValueAsPartOfGesture (px->convertFrom0to1 (px->getDefaultValue()));
    ay.setValueAsPartOfGesture (py->convertFrom0to1 (py->getDefaultValue()));
}

void XYPad::mouseDoubleClick (const juce::MouseEvent&)
{
    ax.beginGesture(); ay.beginGesture();
    resetToDefault();
    ax.endGesture();   ay.endGesture();
}
} // namespace humpire
