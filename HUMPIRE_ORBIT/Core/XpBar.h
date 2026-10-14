#pragma once
#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireTheme.h"

/** Barre "XP" de gain staging (touche gaming) : le niveau de sortie remplit la barre ;
    rester dans la zone ideale (-20..-6 dBFS) fait gagner de l'XP, 1 niveau toutes les 20 s. */
class XpBar : public juce::Component, private juce::Timer
{
public:
    explicit XpBar (const std::atomic<float>& out) : source (out) { startTimerHz (20); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (humpire::theme::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (humpire::theme::edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        g.setFont (humpire::theme::pixel (8.0f));
        g.setColour (humpire::theme::amber);
        g.drawText ("LVL " + juce::String (level), r.removeFromLeft (52.0f).reduced (6, 0), juce::Justification::centredLeft, false);

        auto bar = r.reduced (4, 9).withTrimmedRight (6);
        g.setColour (humpire::theme::bg);
        g.fillRoundedRectangle (bar, 4.0f);

        const auto toX = [&] (float db) { return bar.getX() + bar.getWidth() * juce::jlimit (0.0f, 1.0f, (db + 48.0f) / 48.0f); };
        g.setColour (humpire::theme::good.withAlpha (0.18f));
        g.fillRect (juce::Rectangle<float> (toX (-20.0f), bar.getY(), toX (-6.0f) - toX (-20.0f), bar.getHeight()));

        const float fill = toX (dbNow) - bar.getX();
        g.setGradientFill (juce::ColourGradient (humpire::theme::violet, bar.getX(), 0, dbNow > -6.0f ? humpire::theme::danger : humpire::theme::amber, bar.getRight(), 0, false));
        g.fillRoundedRectangle (bar.withWidth (juce::jmax (0.0f, fill)), 4.0f);

        // progression XP vers le niveau suivant (liseret bas)
        g.setColour (humpire::theme::good);
        g.fillRect (bar.getX(), bar.getBottom() - 2.0f, bar.getWidth() * (float) (xp / 20.0), 2.0f);
    }

private:
    void timerCallback() override
    {
        dbNow = juce::jmax (-48.0f, juce::Decibels::gainToDecibels (source.load (std::memory_order_relaxed), -48.0f));
        if (dbNow > -20.0f && dbNow < -6.0f)
        {
            xp += 0.05;
            if (xp >= 20.0) { xp = 0.0; ++level; }
        }
        repaint();
    }
    const std::atomic<float>& source;
    float dbNow = -48.0f;
    double xp = 0.0;
    int level = 1;
};
