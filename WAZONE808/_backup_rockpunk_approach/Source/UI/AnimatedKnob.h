#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <functional>

#include "Palette.h"
#include "WarzoneLookAndFeel.h"

namespace wz
{

// Hides the default slider drawing so the knob is fully sprite-painted.
class BlankSliderLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int, int, int, int, float, const float,
                           const float, juce::Slider&) override {}
    void drawLinearSlider (juce::Graphics&, int, int, int, int, float, float, float,
                           const juce::Slider::SliderStyle, juce::Slider&) override {}
};

//==============================================================================
// Rotary knob rendered from the background artwork: WarzoneLookAndFeel crops
// the real knob out of the mockup and rotates it into a filmstrip, so turning
// the control spins the actual artwork. A neon halo overlays the sprite while a
// note is triggering, and hover/press subtly brightens the glow.
//==============================================================================
class AnimatedKnob final : public juce::Component,
                           public juce::Timer,
                           public juce::Slider::Listener
{
public:
    AnimatedKnob (const juce::String& labelText, WarzoneLookAndFeel& lfRef,
                  const juce::String& spriteKey)
        : knobLabel (labelText),
          artwork (lfRef),
          key (spriteKey)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setLookAndFeel (&sliderLnF);
        slider.getProperties().set ("sprite", juce::var (key));
        slider.addListener (this);
        addAndMakeVisible (slider);
        startTimerHz (30);
        addMouseListener (this, true);
    }

    ~AnimatedKnob() override
    {
        slider.removeListener (this);
    }

    juce::Slider slider;

    void setGlow01 (float g) noexcept { glowTarget = juce::jlimit (0.0f, 1.0f, g); }
    void setLabel (const juce::String& t) noexcept { knobLabel = t; repaint(); }
    void setTextFormatter (std::function<juce::String (float)> f) noexcept { fmt = std::move (f); }

    void resized() override { slider.setBounds (getLocalBounds()); }

    void timerCallback() override
    {
        hover += ((hoverTarget - hover) * 0.25f);
        press += ((pressTarget - press) * 0.2f);
        glow  += ((glowTarget - glow) * 0.18f);

        if (std::fabs (hover - hoverTarget) > 0.01f
            || std::fabs (press - pressTarget) > 0.01f
            || std::fabs (glow - glowTarget) > 0.01f)
            repaint();
    }

    void sliderValueChanged (juce::Slider*) override { repaint(); }

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();
        const float labelH = 15.0f;
        const float knobH = std::max (8.0f, h - labelH - 2.0f);
        const juce::Point<float> c (w * 0.5f, labelH + knobH * 0.5f + 1.0f);
        const float radius = std::min (w * 0.5f - 3.0f, knobH * 0.5f - 3.0f);

        if (radius <= 6.0f)
        {
            g.setColour (Palette::textDim());
            g.drawText (knobLabel, getLocalBounds(), juce::Justification::centred);
            return;
        }

        const float v = (float) slider.getNormalisableRange().convertTo0to1 (slider.getValue());
        const float glowLevel = juce::jmax (glow, hover * 0.3f);
        const juce::Rectangle<int> knobRect ((int) (c.x - radius), (int) (c.y - radius),
                                             (int) (radius * 2.0f), (int) (radius * 2.0f));

        // real artwork, rotating filmstrip
        if (artwork.hasSprite (key))
            artwork.drawKnobSprite (g, key, v, knobRect);
        else
            drawFallbackKnob (g, c, radius, v);

        // neon halo (only when a note is triggering / hovering)
        if (glowLevel > 0.02f)
        {
            g.setColour (Palette::neonRed().withAlpha (0.30f * glowLevel));
            g.drawEllipse (c.x - radius - 4.0f, c.y - radius - 4.0f,
                           (radius + 4.0f) * 2.0f, (radius + 4.0f) * 2.0f, 4.0f);
            g.setColour (Palette::neonGreen().withAlpha (0.45f * glowLevel));
            g.drawEllipse (c.x - radius - 2.0f, c.y - radius - 2.0f,
                           (radius + 2.0f) * 2.0f, (radius + 2.0f) * 2.0f, 2.0f);
        }

        // label
        g.setColour (Palette::textDim());
        g.setFont (juce::Font (10.0f).boldened());
        g.drawText (knobLabel, juce::Rectangle<float> (0, labelH, w, 13.0f), juce::Justification::centred);

        // value
        g.setColour (glowLevel > 0.02f ? Palette::neonGreen() : Palette::text());
        g.setFont (juce::Font (9.0f));
        g.drawText (formatValue (slider.getValue()),
                    juce::Rectangle<float> (0, h - 13.0f, w, 13.0f), juce::Justification::centred);
    }

private:
    void drawFallbackKnob (juce::Graphics& g, juce::Point<float> c, float radius, float v) const
    {
        const float startA = 2.35619f; // 135 deg
        const float endA   = 5.49779f; // 315 deg
        const float a = startA + (endA - startA) * v;

        juce::ColourGradient bodyGrad (Palette::chromeHi(), c.x - radius, c.y - radius,
                                       Palette::ink(), c.x + radius, c.y + radius, false);
        g.setGradientFill (bodyGrad);
        g.fillEllipse (c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);

        g.setColour (Palette::ink());
        g.fillEllipse (c.x - radius * 0.82f, c.y - radius * 0.82f, radius * 1.64f, radius * 1.64f);

        juce::Path arc;
        arc.addCentredArc (c.x, c.y, radius * 0.90f, radius * 0.90f, 0.0f, startA, a, true);
        g.setColour (Palette::red());
        g.strokePath (arc, juce::PathStrokeType (2.4f));

        g.setColour (Palette::text());
        g.drawLine (c.x, c.y, c.x + std::cos (a) * radius * 0.62f, c.y + std::sin (a) * radius * 0.62f, 2.0f);
    }

    juce::String formatValue (float v) const
    {
        if (fmt)
            return fmt (v);
        const float n = std::fabs (v);
        if (n >= 100.0f) return juce::String (std::round (v));
        if (n >= 1.0f)   return juce::String (v, 1);
        return juce::String (v, 2);
    }

    void mouseEnter (const juce::MouseEvent&) override { hoverTarget = 1.0f; }
    void mouseExit (const juce::MouseEvent&) override { hoverTarget = 0.0f; }
    void mouseDown (const juce::MouseEvent&) override { pressTarget = 1.0f; }
    void mouseUp (const juce::MouseEvent&) override { pressTarget = 0.0f; }

    BlankSliderLookAndFeel sliderLnF;
    WarzoneLookAndFeel& artwork;
    juce::String knobLabel;
    juce::String key;
    std::function<juce::String (float)> fmt;
    float hover = 0.0f, hoverTarget = 0.0f;
    float press = 0.0f, pressTarget = 0.0f;
    float glow = 0.0f, glowTarget = 0.0f;
};

} // namespace wz