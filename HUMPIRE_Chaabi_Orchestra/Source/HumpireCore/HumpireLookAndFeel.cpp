#include "HumpireLookAndFeel.h"

namespace humpire
{
using namespace theme;

HumpireLookAndFeel::HumpireLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, bg);
    setColour (juce::TextButton::buttonColourId, panelHi);
    setColour (juce::TextButton::buttonOnColourId, gold);
    setColour (juce::TextButton::textColourOffId, text);
    setColour (juce::TextButton::textColourOnId, bg);
    setColour (juce::ToggleButton::textColourId, text);
    setColour (juce::ComboBox::backgroundColourId, panelHi);
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::arrowColourId, gold);
    setColour (juce::PopupMenu::backgroundColourId, panel);
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, terracotta);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::AlertWindow::backgroundColourId, panel);
    setColour (juce::AlertWindow::textColourId, text);
    setColour (juce::AlertWindow::outlineColourId, gold);
    setColour (juce::TextEditor::backgroundColourId, bg);
    setColour (juce::TextEditor::textColourId, text);
    setColour (juce::TextEditor::outlineColourId, panelHi);
    setColour (juce::TextEditor::focusedOutlineColourId, gold);
    setColour (juce::ProgressBar::foregroundColourId, gold);
    setColour (juce::ProgressBar::backgroundColourId, panelHi);
    setColour (juce::Label::textColourId, text);
}

void HumpireLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                           float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto labelArea = b.removeFromBottom (15.0f);
    auto dial = b.reduced (3.0f);
    const float d = juce::jmin (dial.getWidth(), dial.getHeight());
    auto c = dial.withSizeKeepingCentre (d, d);
    const float cx = c.getCentreX(), cy = c.getCentreY();
    const float arcR = d * 0.5f - 3.0f;
    const float thick = juce::jmax (2.5f, d * 0.055f);
    const bool hot = s.isMouseOverOrDragging() && s.isEnabled();

    // Piste
    juce::Path track;
    track.addCentredArc (cx, cy, arcR, arcR, 0.0f, a0, a1, true);
    g.setColour (panelHi.brighter (0.15f));
    g.strokePath (track, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Arc de valeur (bipolaire si le range contient 0)
    const float aVal = a0 + pos * (a1 - a0);
    float aFrom = a0;
    if (s.getMinimum() < 0.0 && s.getMaximum() > 0.0)
        aFrom = a0 + (float) s.valueToProportionOfLength (0.0) * (a1 - a0);

    juce::Path val;
    val.addCentredArc (cx, cy, arcR, arcR, 0.0f, juce::jmin (aFrom, aVal), juce::jmax (aFrom, aVal), true);
    g.setColour (gold.withAlpha (hot ? 0.38f : 0.22f));
    g.strokePath (val, juce::PathStrokeType (thick + 5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (s.isEnabled() ? gold : textDim);
    g.strokePath (val, juce::PathStrokeType (thick, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Corps
    auto body = c.reduced (d * 0.17f);
    g.setGradientFill (juce::ColourGradient (panelHi.brighter (0.25f), body.getX(), body.getY(),
                                             bg.darker (0.5f), body.getRight(), body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.drawEllipse (body, 1.0f);

    // Aiguille terre cuite
    juce::Path ptr;
    const float r = body.getHeight() * 0.5f;
    ptr.addRoundedRectangle (-1.7f, -r + 2.5f, 3.4f, r * 0.55f, 1.5f);
    g.setColour (terracotta.brighter (0.25f));
    g.fillPath (ptr, juce::AffineTransform::rotation (aVal).translated (cx, cy));

    // Libelle : nom au repos, valeur au survol
    g.setColour (hot ? gold : textDim);
    g.setFont (font (11.5f, hot));
    g.drawText (hot ? s.getTextFromValue (s.getValue()) : s.getName(),
                labelArea.toNearestInt(), juce::Justification::centred, true);
}

void HumpireLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&,
                                               bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    auto base = b.getToggleState() ? gold : panelHi;
    if (highlighted) base = base.brighter (0.12f);
    if (down) base = base.darker (0.15f);
    g.setColour (base);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (b.getToggleState() ? gold.brighter (0.3f) : juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);
}

void HumpireLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();
    g.setColour (on ? gold : panelHi);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (on ? gold.brighter (0.3f) : juce::Colours::white.withAlpha (highlighted ? 0.2f : 0.08f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);
    g.setColour (on ? bg : text);
    g.setFont (font (13.0f, true));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

juce::Font HumpireLookAndFeel::getTextButtonFont (juce::TextButton&, int h)
{
    return font (juce::jmin (14.0f, (float) h * 0.55f), true);
}

void HumpireLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& cb)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (0.5f);
    g.setColour (panelHi);
    g.fillRoundedRectangle (r, 6.0f);
    juce::Path arrow;
    const float cx = (float) w - 14.0f, cy = (float) h * 0.5f;
    arrow.addTriangle (cx - 4.0f, cy - 2.0f, cx + 4.0f, cy - 2.0f, cx, cy + 3.0f);
    g.setColour (cb.findColour (juce::ComboBox::arrowColourId));
    g.fillPath (arrow);
}

juce::Font HumpireLookAndFeel::getComboBoxFont (juce::ComboBox&) { return font (13.0f); }

} // namespace humpire
