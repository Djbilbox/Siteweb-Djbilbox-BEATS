#pragma once
#include <atomic>
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireTheme.h"

/** Vu-metre pic avec retenue. Source : atomic<float> lineaire (0..1+) ecrit par le thread audio. */
class LevelMeter : public juce::Component, private juce::Timer
{
public:
    LevelMeter (const std::atomic<float>& src, bool vert = false) : source (src), vertical (vert) { startTimerHz (30); }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour (humpire::theme::panel);
        g.fillRoundedRectangle (r, 3.0f);
        const float f = toFraction (level);
        auto fill = vertical ? r.withTrimmedTop (r.getHeight() * (1.0f - f)) : r.withWidth (r.getWidth() * f);
        g.setGradientFill (vertical
            ? juce::ColourGradient (humpire::theme::ember, r.getTopLeft(), humpire::theme::violet, r.getBottomLeft(), false)
            : juce::ColourGradient (humpire::theme::violet, r.getTopLeft(), humpire::theme::ember, r.getTopRight(), false));
        g.fillRoundedRectangle (fill.reduced (0.5f), 3.0f);
        // retenue du pic
        g.setColour (peakHold > 0.99f ? humpire::theme::danger : humpire::theme::amber);
        const float p = toFraction (peakHold);
        if (vertical) g.fillRect (r.getX(), r.getBottom() - r.getHeight() * p, r.getWidth(), 1.5f);
        else          g.fillRect (r.getX() + r.getWidth() * p - 1.5f, r.getY(), 1.5f, r.getHeight());
    }

private:
    static float toFraction (float lin)
    {
        const float db = juce::Decibels::gainToDecibels (lin, -60.0f);
        return juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    }
    void timerCallback() override
    {
        const float v = source.load (std::memory_order_relaxed);
        level = v > level ? v : level * 0.82f + v * 0.18f;
        if (level > peakHold) { peakHold = level; holdCount = 45; }
        else if (--holdCount < 0) peakHold *= 0.93f;
        repaint();
    }
    const std::atomic<float>& source;
    bool vertical;
    float level = 0.0f, peakHold = 0.0f;
    int holdCount = 0;
};
