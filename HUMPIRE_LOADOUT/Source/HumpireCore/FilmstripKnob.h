#pragma once
#include "HumpireTheme.h"

namespace humpire
{
/** Knob HUMPIRE : filmstrip optionnel (sinon rendu vectoriel du LookAndFeel).
    - valeur au survol
    - double-clic = reset (fourni par SliderAttachment)
    - Shift au debut du drag = reglage fin
    - clic droit = saisie numerique */
class FilmstripKnob : public juce::Slider
{
public:
    FilmstripKnob();

    /** strip : images empilees verticalement (vertical=true) ou cote a cote. */
    void setFilmstrip (const juce::Image& strip, int numFrames, bool vertical = true);

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    void promptForValue();

    juce::Image strip;
    int  frames = 0;
    bool vertical = true, hover = false, rightClickHandled = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilmstripKnob)
};
} // namespace humpire
