#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "HumpireTheme.h"

/** Knob "orbite" : arc lumineux, valeur au survol, double-clic = reset,
    Shift (maintenu au clic) = reglage fin, clic droit = saisie numerique. */
class OrbitKnob : public juce::Slider
{
public:
    explicit OrbitKnob (const juce::String& name, juce::Colour accent = humpire::theme::amber)
    {
        setName (name);
        setSliderStyle (juce::Slider::RotaryVerticalDrag);
        setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        setColour (juce::Slider::rotarySliderFillColourId, accent);
        setMouseDragSensitivity (220);
        setScrollWheelEnabled (true);
        setPopupMenuEnabled (false);
    }

    /** Lie le knob a un parametre APVTS (le double-clic revient a la valeur par defaut du parametre). */
    void attach (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID)
    {
        param = apvts.getParameter (paramID);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramID, *this);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        entryClick = e.mods.isPopupMenu();
        if (entryClick) { showEntry(); return; }
        setMouseDragSensitivity (e.mods.isShiftDown() ? 2000 : 220);
        juce::Slider::mouseDown (e);
    }
    void mouseDrag (const juce::MouseEvent& e) override { if (! entryClick) juce::Slider::mouseDrag (e); }
    void mouseUp   (const juce::MouseEvent& e) override { if (! entryClick) juce::Slider::mouseUp (e); entryClick = false; }
    void mouseEnter (const juce::MouseEvent& e) override { juce::Slider::mouseEnter (e); repaint(); }
    void mouseExit  (const juce::MouseEvent& e) override { juce::Slider::mouseExit (e);  repaint(); }

private:
    void showEntry()
    {
        editor = std::make_unique<juce::TextEditor>();
        addAndMakeVisible (*editor);
        editor->setBounds (getLocalBounds().withSizeKeepingCentre (juce::jmax (48, getWidth() - 8), 22).translated (0, -6));
        editor->setJustification (juce::Justification::centred);
        editor->setFont (humpire::theme::title (13.0f));
        editor->setText (param != nullptr ? param->getCurrentValueAsText() : getTextFromValue (getValue()), false);
        editor->selectAll();
        editor->grabKeyboardFocus();
        editor->onReturnKey = [this] { commit(); };
        editor->onEscapeKey = [this] { closeEntry(); };
        editor->onFocusLost = [this] { closeEntry(); };
    }

    void commit()
    {
        const auto text = editor->getText();
        if (param != nullptr)
        {
            param->beginChangeGesture();
            param->setValueNotifyingHost (param->getValueForText (text));
            param->endChangeGesture();
        }
        else
            setValue (getValueFromText (text), juce::sendNotificationSync);
        closeEntry();
    }

    void closeEntry()
    {
        if (editor == nullptr) return;
        editor->setVisible (false);
        juce::Component::SafePointer<OrbitKnob> self (this);
        juce::MessageManager::callAsync ([self] { if (self != nullptr) self->editor.reset(); });
    }

    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    std::unique_ptr<juce::TextEditor> editor;
    bool entryClick = false;
};
