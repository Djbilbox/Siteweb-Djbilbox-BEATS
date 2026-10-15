#pragma once
#include <array>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/HumpireTheme.h"
#include "PluginProcessor.h"
#include "SaturnOrbitView.h"

/** Strip under the scene: one cell per voice with its state (off, or its current interval). */
class SaturnReadout : public juce::Component, private juce::Timer
{
public:
    explicit SaturnReadout (SaturnAudioProcessor& p) : proc (p)
    {
        for (int v = 0; v < saturn::kNumVoices; ++v)
            onFlag[(size_t) v] = proc.apvts.getRawParameterValue (saturn::voiceId (v, saturn::vOn));
        startTimerHz (20);
    }

    void paint (juce::Graphics& g) override
    {
        using namespace humpire;
        auto r = getLocalBounds().toFloat();
        g.setColour (theme::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (theme::edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        const float cell = r.getWidth() / (float) saturn::kNumVoices;
        for (int v = 0; v < saturn::kNumVoices; ++v)
        {
            const bool on = onFlag[(size_t) v]->load() > 0.5f;
            const float shift = proc.uiVoiceShift[(size_t) v].load (std::memory_order_relaxed);
            const auto col = saturnVoiceColour (v);

            juce::String text = "V" + juce::String (v + 1) + "  ";
            text += on ? ((shift >= 0.0f ? "+" : "") + juce::String (shift, 1) + " st") : juce::String ("off");

            g.setFont (theme::title (13.0f));
            g.setColour (on ? col : theme::textLo);
            g.drawText (text, juce::Rectangle<float> (r.getX() + cell * (float) v, r.getY(), cell, r.getHeight()),
                        juce::Justification::centred, false);
        }
    }

private:
    void timerCallback() override { repaint(); }

    SaturnAudioProcessor& proc;
    std::array<std::atomic<float>*, saturn::kNumVoices> onFlag {};
};
