#pragma once
#include <cmath>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Core/HumpireTheme.h"
#include "PluginProcessor.h"

/** Tableau de bord des doublures : une ligne par jumelle (pan, detune et retard courants, niveau). */
class TwinVoiceStrip : public juce::Component, private juce::Timer
{
public:
    explicit TwinVoiceStrip (TwinAudioProcessor& p) : proc (p) { startTimerHz (20); }

    void paint (juce::Graphics& g) override
    {
        using namespace humpire;
        auto r = getLocalBounds().toFloat();
        g.setColour (theme::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (theme::edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        auto inner = r.reduced (8.0f, 6.0f);
        const float rowH = inner.getHeight() / (float) twin::kMaxVoices;
        for (int k = 0; k < twin::kMaxVoices; ++k)
        {
            const auto& u = proc.voiceUi[(size_t) k];
            const float env = juce::jlimit (0.0f, 1.0f, u.env.load (std::memory_order_relaxed));
            const bool on = env > 0.01f;
            auto row = inner.removeFromTop (rowH);
            const float a = on ? 0.35f + 0.65f * env : 0.25f;

            const juce::Point<float> dot (row.getX() + 6.0f, row.getCentreY());
            g.setColour ((k % 2 == 0 ? theme::cyan : theme::violet).withAlpha (a));
            g.fillEllipse (dot.x - 4.0f, dot.y - 4.0f, 8.0f, 8.0f);

            g.setFont (theme::pixel (6.5f));
            g.setColour (theme::amber.withAlpha (a));
            g.drawText ("TWIN " + juce::String (k + 1), juce::Rectangle<float> (row.getX() + 16.0f, row.getY(), 64.0f, row.getHeight()), juce::Justification::centredLeft, false);

            g.setFont (theme::text (11.0f));
            g.setColour (theme::textHi.withAlpha (a));
            if (on)
            {
                const float pan = u.pan.load (std::memory_order_relaxed);
                const int pp = juce::roundToInt (std::abs (pan) * 100.0f);
                const juce::String panTxt = pp < 3 ? juce::String ("C") : (pan < 0.0f ? "L " : "R ") + juce::String (pp);
                const float ct = u.cents.load (std::memory_order_relaxed);
                g.drawText (panTxt, juce::Rectangle<float> (row.getX() + 82.0f, row.getY(), 44.0f, row.getHeight()), juce::Justification::centredLeft, false);
                g.drawText ((ct >= 0.0f ? "+" : "") + juce::String (ct, 1) + " ct", juce::Rectangle<float> (row.getX() + 128.0f, row.getY(), 62.0f, row.getHeight()), juce::Justification::centredLeft, false);
                g.drawText (juce::String (u.delayMs.load (std::memory_order_relaxed), 1) + " ms", juce::Rectangle<float> (row.getX() + 192.0f, row.getY(), 56.0f, row.getHeight()), juce::Justification::centredLeft, false);
            }
            else
                g.drawText ("en veille", juce::Rectangle<float> (row.getX() + 82.0f, row.getY(), 120.0f, row.getHeight()), juce::Justification::centredLeft, false);

            // niveau de la doublure
            auto bar = juce::Rectangle<float> (row.getRight() - 50.0f, row.getCentreY() - 2.5f, 50.0f, 5.0f);
            g.setColour (theme::bg);
            g.fillRoundedRectangle (bar, 2.5f);
            const float db = juce::Decibels::gainToDecibels (u.level.load (std::memory_order_relaxed), -48.0f);
            const float f = juce::jlimit (0.0f, 1.0f, (db + 48.0f) / 48.0f);
            g.setGradientFill (juce::ColourGradient (theme::violet, bar.getX(), 0, theme::amber, bar.getRight(), 0, false));
            g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * f), 2.5f);
        }
    }

private:
    void timerCallback() override { repaint(); }
    TwinAudioProcessor& proc;
};

/** Correlometre stereo "MONO COMPAT" : -1 (opposition de phase) .. +1 (mono). */
class CorrelationMeter : public juce::Component, private juce::Timer
{
public:
    explicit CorrelationMeter (const std::atomic<float>& src) : source (src) { startTimerHz (30); }

    void paint (juce::Graphics& g) override
    {
        using namespace humpire;
        auto r = getLocalBounds().toFloat();
        g.setColour (theme::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (theme::edge);
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        g.setFont (theme::pixel (7.0f));
        g.setColour (theme::amber);
        g.drawText ("MONO COMPAT", r.removeFromLeft (118.0f).reduced (10.0f, 0.0f), juce::Justification::centredLeft, false);
        auto txt = r.removeFromRight (64.0f);
        g.setColour (value < 0.0f ? theme::danger : value < 0.3f ? theme::amber : theme::good);
        g.setFont (theme::title (13.0f));
        g.drawText ((value >= 0.0f ? "+" : "") + juce::String (value, 2), txt, juce::Justification::centred, false);

        auto track = r.reduced (4.0f, r.getHeight() * 0.5f - 4.0f);
        g.setGradientFill (juce::ColourGradient (theme::danger.withAlpha (0.55f), track.getX(), 0, theme::good.withAlpha (0.55f), track.getRight(), 0, false));
        g.fillRoundedRectangle (track, 4.0f);
        g.setColour (theme::bg.withAlpha (0.55f));
        g.fillRoundedRectangle (track.reduced (1.0f), 3.0f);

        g.setFont (theme::text (9.0f));
        g.setColour (theme::textLo);
        g.drawText ("-1", juce::Rectangle<float> (track.getX(), track.getBottom() + 1.0f, 20.0f, 10.0f), juce::Justification::centredLeft, false);
        g.drawText ("0", juce::Rectangle<float> (track.getCentreX() - 10.0f, track.getBottom() + 1.0f, 20.0f, 10.0f), juce::Justification::centred, false);
        g.drawText ("+1", juce::Rectangle<float> (track.getRight() - 20.0f, track.getBottom() + 1.0f, 20.0f, 10.0f), juce::Justification::centredRight, false);
        g.fillRect (track.getCentreX() - 0.5f, track.getY() - 3.0f, 1.0f, track.getHeight() + 6.0f);

        const float x = track.getX() + track.getWidth() * (0.5f + 0.5f * juce::jlimit (-1.0f, 1.0f, value));
        LookAndFeelDot::draw (g, { x, track.getCentreY() }, value < 0.0f ? theme::danger : theme::amber);
    }

private:
    struct LookAndFeelDot
    {
        static void draw (juce::Graphics& g, juce::Point<float> c, juce::Colour col)
        {
            g.setColour (col.withAlpha (0.25f));
            g.fillEllipse (c.x - 9.0f, c.y - 9.0f, 18.0f, 18.0f);
            g.setColour (col);
            g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9.0f, 9.0f);
        }
    };

    void timerCallback() override
    {
        value += (source.load (std::memory_order_relaxed) - value) * 0.2f;
        repaint();
    }
    const std::atomic<float>& source;
    float value = 1.0f;
};
