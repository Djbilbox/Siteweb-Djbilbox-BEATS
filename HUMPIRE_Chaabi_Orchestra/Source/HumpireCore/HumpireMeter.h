#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include "HumpireTheme.h"

namespace humpire
{
// Horizontal stereo meter (2 bars), reads atomics written by the audio thread.
class HumpireMeter : public juce::Component, private juce::Timer
{
public:
    HumpireMeter (const std::atomic<float>& l, const std::atomic<float>& r, juce::String label)
        : left (l), right (r), name (std::move (label))
    {
        startTimerHz (30);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace theme;
        auto b = getLocalBounds().toFloat();
        g.setColour (textDim);
        g.setFont (font (10.0f, true));
        g.drawText (name, b.removeFromLeft (26.0f).toNearestInt(), juce::Justification::centredLeft);

        auto drawBar = [&] (juce::Rectangle<float> r, float level, float peak)
        {
            g.setColour (bg.darker (0.3f));
            g.fillRoundedRectangle (r, 2.0f);
            auto fill = r.withWidth (r.getWidth() * level);
            g.setGradientFill (juce::ColourGradient (ok, r.getX(), 0.0f, terracotta, r.getRight(), 0.0f, false));
            g.fillRoundedRectangle (fill, 2.0f);
            g.setColour (gold);
            g.fillRect (juce::Rectangle<float> (r.getX() + r.getWidth() * peak - 1.0f, r.getY(), 2.0f, r.getHeight()));
        };

        const float h = b.getHeight() * 0.5f - 1.0f;
        drawBar (b.removeFromTop (h), lvl[0], pk[0]);
        b.removeFromTop (2.0f);
        drawBar (b, lvl[1], pk[1]);
    }

private:
    static float toUnit (float lin) { return juce::jmap (juce::Decibels::gainToDecibels (lin, -60.0f), -60.0f, 0.0f, 0.0f, 1.0f); }

    void timerCallback() override
    {
        const float in[2] { toUnit (left.load()), toUnit (right.load()) };
        for (int i = 0; i < 2; ++i)
        {
            lvl[i] = in[i] > lvl[i] ? in[i] : lvl[i] * 0.86f + in[i] * 0.14f;
            pk[i]  = in[i] > pk[i]  ? in[i] : juce::jmax (0.0f, pk[i] - 0.006f);
        }
        repaint();
    }

    const std::atomic<float>& left;
    const std::atomic<float>& right;
    juce::String name;
    float lvl[2] {}, pk[2] {};
};
} // namespace humpire
