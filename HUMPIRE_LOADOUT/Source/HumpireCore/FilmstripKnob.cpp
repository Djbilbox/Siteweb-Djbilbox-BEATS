#include "FilmstripKnob.h"

namespace humpire
{
FilmstripKnob::FilmstripKnob()
{
    setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
    setMouseDragSensitivity (220);
    setPopupMenuEnabled (false);
}

void FilmstripKnob::setFilmstrip (const juce::Image& s, int n, bool v)
{
    strip = s;  frames = s.isValid() ? juce::jmax (0, n) : 0;  vertical = v;
    repaint();
}

void FilmstripKnob::paint (juce::Graphics& g)
{
    if (frames > 1)
    {
        const int idx = juce::jlimit (0, frames - 1,
                            juce::roundToInt ((float) valueToProportionOfLength (getValue()) * (float) (frames - 1)));
        const int fw = vertical ? strip.getWidth()  : strip.getWidth() / frames;
        const int fh = vertical ? strip.getHeight() / frames : strip.getHeight();
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (strip, 0, 0, getWidth(), getHeight(),
                     vertical ? 0 : idx * fw, vertical ? idx * fh : 0, fw, fh);
    }
    else
        juce::Slider::paint (g);
}

void FilmstripKnob::paintOverChildren (juce::Graphics& g)
{
    if (! (hover || isMouseButtonDown())) return;
    const auto txt = getTextFromValue (getValue()) + getTextValueSuffix();
    const auto f = Theme::uiFont (11.0f, true);
    const int  w = juce::roundToInt (juce::GlyphArrangement::getStringWidth (f, txt)) + 12;
    const auto r = juce::Rectangle<int> (w, 16).withCentre (getLocalBounds().getCentre());
    g.setColour (col::black.withAlpha (0.88f));  g.fillRoundedRectangle (r.toFloat(), 4.0f);
    g.setColour (col::amber);                    g.drawRoundedRectangle (r.toFloat(), 4.0f, 1.0f);
    g.setFont (f);  g.drawText (txt, r, juce::Justification::centred);
}

void FilmstripKnob::mouseDown (const juce::MouseEvent& e)
{
    rightClickHandled = e.mods.isPopupMenu();
    if (rightClickHandled) { promptForValue(); return; }
    setMouseDragSensitivity (e.mods.isShiftDown() ? 1800 : 220);   // Shift = fin
    juce::Slider::mouseDown (e);
}

void FilmstripKnob::mouseUp (const juce::MouseEvent& e)
{
    if (rightClickHandled) { rightClickHandled = false; return; }
    juce::Slider::mouseUp (e);
}

void FilmstripKnob::mouseEnter (const juce::MouseEvent& e) { hover = true;  repaint(); juce::Slider::mouseEnter (e); }
void FilmstripKnob::mouseExit  (const juce::MouseEvent& e) { hover = false; repaint(); juce::Slider::mouseExit (e); }

void FilmstripKnob::promptForValue()
{
    auto* w = new juce::AlertWindow ("Value", getName(), juce::MessageBoxIconType::NoIcon);
    w->addTextEditor ("v", getTextFromValue (getValue()));
    w->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<FilmstripKnob> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int result)
    {
        if (result == 1 && safe != nullptr)
            safe->setValue (safe->getValueFromText (w->getTextEditorContents ("v")), juce::sendNotificationSync);
    }), true);
}
} // namespace humpire
