#include "HumpireLookAndFeel.h"

using namespace humpire;

HumpireLookAndFeel::HumpireLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, theme::bg);
    setColour (juce::Slider::rotarySliderFillColourId, theme::amber);
    setColour (juce::Slider::rotarySliderOutlineColourId, theme::edge);
    setColour (juce::Slider::thumbColourId, theme::textHi);
    setColour (juce::Label::textColourId, theme::textHi);
    setColour (juce::ComboBox::backgroundColourId, theme::panel);
    setColour (juce::ComboBox::outlineColourId, theme::edge);
    setColour (juce::ComboBox::textColourId, theme::textHi);
    setColour (juce::ComboBox::arrowColourId, theme::amber);
    setColour (juce::PopupMenu::backgroundColourId, theme::panel);
    setColour (juce::PopupMenu::textColourId, theme::textHi);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, theme::violet.withAlpha (0.55f));
    setColour (juce::PopupMenu::highlightedTextColourId, theme::textHi);
    setColour (juce::ToggleButton::tickColourId, theme::amber);
    setColour (juce::TextButton::buttonColourId, theme::panelHi);
    setColour (juce::TextButton::buttonOnColourId, theme::amber);
    setColour (juce::TextButton::textColourOffId, theme::textHi);
    setColour (juce::TextButton::textColourOnId, theme::bg);
    setColour (juce::TextEditor::backgroundColourId, theme::panel);
    setColour (juce::TextEditor::textColourId, theme::textHi);
    setColour (juce::TextEditor::outlineColourId, theme::edge);
    setColour (juce::TextEditor::focusedOutlineColourId, theme::amber);
    setColour (juce::AlertWindow::backgroundColourId, theme::panel);
    setColour (juce::AlertWindow::textColourId, theme::textHi);
    setColour (juce::AlertWindow::outlineColourId, theme::edge);
    setDefaultSansSerifTypefaceName ("Inter");
}

void HumpireLookAndFeel::drawGlowDot (juce::Graphics& g, juce::Point<float> c, float r, juce::Colour col)
{
    juce::ColourGradient halo (col.withAlpha (0.85f), c, col.withAlpha (0.0f), c.translated (r * 3.2f, 0), true);
    halo.addColour (0.25, col.withAlpha (0.35f));
    g.setGradientFill (halo);
    g.fillEllipse (c.x - r * 3.2f, c.y - r * 3.2f, r * 6.4f, r * 6.4f);
    g.setColour (juce::Colours::white.interpolatedWith (col, 0.25f));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);
}

//==============================================================================
void HumpireLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                           float startAngle, float endAngle, juce::Slider& s)
{
    const float labelH = 14.0f;
    auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    auto labelArea = area.removeFromBottom (labelH);
    const float d = juce::jmin (area.getWidth(), area.getHeight()) - 6.0f;
    auto c = area.getCentre();
    const float r = d * 0.5f;
    const auto accent = s.findColour (juce::Slider::rotarySliderFillColourId);
    const bool active = s.isMouseOverOrDragging() && s.isEnabled();

    // Disque central (planete sombre)
    juce::ColourGradient body (theme::panelHi.brighter (0.15f), c.translated (-r * 0.3f, -r * 0.4f),
                               theme::bg, c.translated (r * 0.7f, r * 0.8f), true);
    g.setGradientFill (body);
    g.fillEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f);
    g.setColour (theme::edge);
    g.drawEllipse (c.x - r * 0.72f, c.y - r * 0.72f, r * 1.44f, r * 1.44f, 1.0f);

    // Orbite (piste)
    const float arcR = r - 2.0f;
    juce::Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (theme::edge);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Arc lumineux (valeur)
    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path val;
    val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, angle, true);
    for (int i = 3; i >= 1; --i)   // halo
    {
        g.setColour (accent.withAlpha (0.07f * (float) (4 - i) * (active ? 1.6f : 1.0f)));
        g.strokePath (val, juce::PathStrokeType (3.0f + (float) i * 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setGradientFill (juce::ColourGradient (accent.brighter (0.2f), c.x - r, c.y, accent.interpolatedWith (theme::ember, 0.7f), c.x + r, c.y, false));
    g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Planete en bout d'orbite
    const juce::Point<float> tip (c.x + arcR * std::sin (angle), c.y - arcR * std::cos (angle));
    drawGlowDot (g, tip, 2.6f, accent);

    // Texte : valeur au survol, nom sinon
    g.setFont (theme::text (11.0f));
    if (active)
    {
        g.setColour (theme::textHi);
        g.setFont (theme::title (12.0f));
        g.drawText (s.getTextFromValue (s.getValue()), area.withSizeKeepingCentre (r * 1.4f, 16.0f), juce::Justification::centred, false);
    }
    g.setColour (active ? accent : theme::textLo);
    g.setFont (theme::text (11.0f));
    g.drawText (s.getName().toUpperCase(), labelArea, juce::Justification::centred, false);
}

//==============================================================================
void HumpireLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const float cr = juce::jmin (r.getHeight() * 0.5f, 10.0f);
    const bool on = b.getToggleState();
    if (on)
    {
        g.setGradientFill (theme::starGradient (r.getTopLeft(), r.getBottomRight()));
        g.fillRoundedRectangle (r, cr);
    }
    else
    {
        g.setColour (down ? theme::panelHi.brighter (0.2f) : over ? theme::panelHi.brighter (0.08f) : theme::panelHi);
        g.fillRoundedRectangle (r, cr);
        g.setColour (over ? theme::amber.withAlpha (0.6f) : theme::edge);
        g.drawRoundedRectangle (r, cr, 1.0f);
    }
}

void HumpireLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.setColour (b.getToggleState() ? theme::bg : theme::textHi);
    g.drawText (b.getButtonText(), b.getLocalBounds().reduced (4, 0), juce::Justification::centred, false);
}

void HumpireLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = b.getToggleState();
    const auto accent = b.findColour (juce::ToggleButton::tickColourId, true);
    const juce::Colour col = accent.isTransparent() ? theme::amber : accent;
    if (on)
    {
        g.setColour (col.withAlpha (0.18f));
        g.fillRoundedRectangle (r.expanded (1.5f), r.getHeight() * 0.5f);
        g.setGradientFill (juce::ColourGradient (col.brighter (0.15f), r.getTopLeft(), col.interpolatedWith (theme::ember, 0.6f), r.getBottomRight(), false));
        g.fillRoundedRectangle (r, juce::jmin (10.0f, r.getHeight() * 0.5f));
    }
    else
    {
        g.setColour (over ? theme::panelHi.brighter (0.1f) : theme::panelHi);
        g.fillRoundedRectangle (r, juce::jmin (10.0f, r.getHeight() * 0.5f));
        g.setColour (over ? col.withAlpha (0.6f) : theme::edge);
        g.drawRoundedRectangle (r, juce::jmin (10.0f, r.getHeight() * 0.5f), 1.0f);
    }
    g.setFont (theme::title (juce::jmin (13.0f, r.getHeight() * 0.55f)));
    g.setColour (on ? theme::bg : theme::textHi);
    g.drawText (b.getButtonText(), r, juce::Justification::centred, false);
}

//==============================================================================
void HumpireLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool down, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (0.5f);
    g.setColour (down ? theme::panelHi : theme::panel);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (box.isMouseOver() ? theme::amber.withAlpha (0.7f) : theme::edge);
    g.drawRoundedRectangle (r, 8.0f, 1.0f);
    juce::Path arrow;
    const float ax = (float) w - 16.0f, ay = (float) h * 0.5f;
    arrow.addTriangle (ax - 4, ay - 2, ax + 4, ay - 2, ax, ay + 3);
    g.setColour (theme::amber);
    g.fillPath (arrow);
}

void HumpireLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& l)
{
    l.setBounds (8, 1, box.getWidth() - 30, box.getHeight() - 2);
    l.setFont (getComboBoxFont (box));
}

juce::Font HumpireLookAndFeel::getComboBoxFont (juce::ComboBox&)        { return theme::title (13.0f); }
juce::Font HumpireLookAndFeel::getPopupMenuFont()                       { return theme::text (14.0f); }
juce::Font HumpireLookAndFeel::getTextButtonFont (juce::TextButton&, int h) { return theme::title (juce::jmin (13.0f, (float) h * 0.5f)); }
juce::Font HumpireLookAndFeel::getLabelFont (juce::Label& l)           { return l.getFont(); }

void HumpireLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (theme::panel);
    g.setColour (theme::edge);
    g.drawRect (0, 0, w, h, 1);
}
