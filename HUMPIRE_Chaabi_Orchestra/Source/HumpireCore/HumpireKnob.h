#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace humpire
{
// Knob HUMPIRE :
//  - valeur affichee au survol (dessinee par HumpireLookAndFeel)
//  - double-clic = reset (fourni par SliderAttachment : valeur par defaut du parametre)
//  - Shift + glisser = reglage fin
//  - clic droit = saisie numerique
class HumpireKnob : public juce::Slider
{
public:
    explicit HumpireKnob (const juce::String& displayName)
    {
        setName (displayName);
        setSliderStyle (juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        setRotaryParameters (juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        setMouseDragSensitivity (220);
        // Shift bascule en mode "velocite" lent = reglage fin
        setVelocityBasedMode (false);
        setVelocityModeParameters (0.12, 1, 0.0, true, juce::ModifierKeys::shiftModifier);
        setScrollWheelEnabled (true);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu()) { askValue(); return; }
        juce::Slider::mouseDown (e);
    }
    void mouseEnter (const juce::MouseEvent& e) override { juce::Slider::mouseEnter (e); repaint(); }
    void mouseExit  (const juce::MouseEvent& e) override { juce::Slider::mouseExit (e);  repaint(); }

private:
    void askValue()
    {
        auto* w = new juce::AlertWindow (getName(), juce::String::fromUTF8 ("Saisir une valeur :"),
                                         juce::MessageBoxIconType::NoIcon);
        w->addTextEditor ("v", getTextFromValue (getValue()));
        w->addButton ("OK", 1, juce::KeyPress (juce::KeyPress::returnKey));
        w->addButton ("Annuler", 0, juce::KeyPress (juce::KeyPress::escapeKey));
        w->enterModalState (true,
            juce::ModalCallbackFunction::create ([safe = juce::Component::SafePointer<HumpireKnob> (this), w] (int r)
            {
                if (r == 1 && safe != nullptr)
                    safe->setValue (safe->getValueFromText (w->getTextEditorContents ("v")), juce::sendNotificationSync);
            }),
            true);
    }
};
} // namespace humpire
