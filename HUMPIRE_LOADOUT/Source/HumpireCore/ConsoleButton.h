#pragma once
#include "HumpireTheme.h"

namespace humpire
{
/** Bouton 3D glossy : rond (A/B, face buttons) ou pilule (start/select). */
class ConsoleButton : public juce::Button
{
public:
    enum class Shape { round, pill };

    ConsoleButton (const juce::String& name, Shape s, juce::Colour baseColour);

    void setGlyph (const juce::String& g)  { glyph = g; repaint(); }
    /** LED d'etat au-dessus du bouton (visible si toggle ON). */
    void setShowLed (bool b)               { showLed = b; repaint(); }

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

private:
    Shape shape;
    juce::Colour base;
    juce::String glyph;
    bool showLed = false;
};

/** Croix directionnelle : 4 directions avec auto-repeat. onStep(dx, dy) : dx,dy in {-1,0,1}. */
class DPad : public juce::Component, private juce::Timer
{
public:
    DPad() = default;
    ~DPad() override { stopTimer(); }

    std::function<void (int dx, int dy)> onStep;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void fire();

    int dirX = 0, dirY = 0;
    bool repeating = false;
};
} // namespace humpire
