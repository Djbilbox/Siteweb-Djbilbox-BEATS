#pragma once
#include <cmath>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/HumpireTheme.h"
#include "TwinParams.h"

/** Live readout of the double stack: detune, micro-delay and stereo position of each voice. */
class TwinVoiceReadout : public juce::Component, private juce::Timer
{
public:
    explicit TwinVoiceReadout (const twin::UiState& s) : state (s) { startTimerHz (20); }

    void paint (juce::Graphics& g) override
    {
        using namespace humpire::theme;
        auto r = getLocalBounds().toFloat();
        g.setColour (panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        g.setFont (pixel (7.0f));
        g.setColour (amber);
        g.drawText ("DOUBLE STACK", r.reduced (12.0f, 7.0f).removeFromTop (12.0f), juce::Justification::centredLeft, false);

        const int voices = juce::jlimit (0, twin::kMaxVoices, state.voices.load (std::memory_order_relaxed));
        auto rows = r.reduced (12.0f, 0.0f).withTrimmedTop (24.0f).withTrimmedBottom (6.0f);
        const float rowH = rows.getHeight() / (float) twin::kMaxVoices;

        for (int k = 0; k < twin::kMaxVoices; ++k)
        {
            const auto i = (size_t) k;
            auto row = juce::Rectangle<float> (rows.getX(), rows.getY() + rowH * (float) k, rows.getWidth(), rowH)
                           .reduced (0.0f, 2.0f);

            g.setFont (pixel (7.0f));
            g.setColour (k < voices ? amber : textLo);
            g.drawText ("V" + juce::String (k + 1), row.removeFromLeft (24.0f), juce::Justification::centredLeft, false);

            if (k >= voices)
            {
                g.setFont (text (11.0f));
                g.setColour (textLo);
                g.drawText ("off", row, juce::Justification::centredLeft, false);
                continue;
            }

            const float ct  = state.detuneCt[i].load (std::memory_order_relaxed);
            const float ms  = state.delayMs[i].load (std::memory_order_relaxed);
            const float pan = juce::jlimit (-1.0f, 1.0f, state.pan[i].load (std::memory_order_relaxed));

            g.setFont (text (11.0f));
            g.setColour (textHi);
            g.drawText (juce::String (ct, 1) + " ct", row.removeFromLeft (62.0f), juce::Justification::centredLeft, false);
            g.drawText (juce::String (ms, 1) + " ms", row.removeFromLeft (62.0f), juce::Justification::centredLeft, false);

            auto panArea = row.removeFromRight (56.0f).reduced (4.0f, 5.0f);
            auto bar = row.reduced (4.0f, 6.0f);

            // micro-delay bar, 5..40 ms
            g.setColour (edge);
            g.fillRoundedRectangle (bar, 3.0f);
            const float fill = juce::jlimit (0.0f, 1.0f, (ms - 5.0f) / 35.0f);
            g.setGradientFill (juce::ColourGradient (violet, bar.getX(), bar.getY(), amber, bar.getRight(), bar.getY(), false));
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * fill), 3.0f);

            // stereo position marker
            g.setColour (edge);
            g.drawHorizontalLine ((int) panArea.getCentreY(), panArea.getX(), panArea.getRight());
            const float px = panArea.getX() + panArea.getWidth() * 0.5f * (pan + 1.0f);
            g.setColour (pan < -0.1f ? cyan : pan > 0.1f ? violet : amber);
            g.fillEllipse (px - 3.0f, panArea.getCentreY() - 3.0f, 6.0f, 6.0f);
        }
    }

private:
    void timerCallback() override { repaint(); }

    const twin::UiState& state;
};
