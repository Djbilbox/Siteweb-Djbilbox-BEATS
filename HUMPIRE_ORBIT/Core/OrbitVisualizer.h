#pragma once
#include <vector>
#include <juce_gui_basics/juce_gui_basics.h>
#include "HumpireTheme.h"

/** Base de toutes les scenes cosmiques HUMPIRE : fond noir, nebuleuses statiques (mises en cache),
    etoiles scintillantes, boucle 60 fps. Les plugins derivent et implementent drawScene(). */
class OrbitVisualizer : public juce::Component, private juce::Timer
{
public:
    OrbitVisualizer()
    {
        setOpaque (true);
        juce::Random rng (0xC0FFEE);
        for (int i = 0; i < 140; ++i)
            stars.push_back ({ rng.nextFloat(), rng.nextFloat(), 0.4f + rng.nextFloat() * 1.3f,
                               rng.nextFloat() * 6.28f, 0.5f + rng.nextFloat() * 2.0f,
                               rng.nextInt (5) == 0 ? humpire::theme::amber : juce::Colours::white });
        startTimerHz (60);
    }

    void paint (juce::Graphics& g) override
    {
        const float t = (float) (juce::Time::getMillisecondCounterHiRes() * 0.001);
        g.drawImageAt (background, 0, 0);
        for (auto& s : stars)
        {
            const float a = 0.25f + 0.75f * (0.5f + 0.5f * std::sin (t * s.speed + s.phase));
            g.setColour (s.colour.withAlpha (a * 0.8f));
            g.fillEllipse (s.x * (float) getWidth() - s.size * 0.5f, s.y * (float) getHeight() - s.size * 0.5f, s.size, s.size);
        }
        drawScene (g, getLocalBounds().toFloat(), t);
        g.setColour (humpire::theme::edge);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), 10.0f, 1.0f);
    }

    void resized() override { rebuildBackground(); }

    //==========================================================================
    /** Anneau d'orbite (ellipse inclinee) avec trait doux. */
    static void drawOrbitRing (juce::Graphics& g, juce::Point<float> c, float rx, float ry, float rotation, juce::Colour col, float thickness = 1.0f)
    {
        juce::Path p;
        p.addEllipse (c.x - rx, c.y - ry, rx * 2.0f, ry * 2.0f);
        p.applyTransform (juce::AffineTransform::rotation (rotation, c.x, c.y));
        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (thickness));
    }

    /** Orbe lumineux (etoile / planete) avec halo radial. */
    static void drawGlowOrb (juce::Graphics& g, juce::Point<float> c, float radius, juce::Colour inner, juce::Colour outer)
    {
        juce::ColourGradient halo (inner, c, outer.withAlpha (0.0f), c.translated (radius * 3.0f, 0), true);
        halo.addColour (0.3, outer.withAlpha (0.4f));
        g.setGradientFill (halo);
        g.fillEllipse (c.x - radius * 3.0f, c.y - radius * 3.0f, radius * 6.0f, radius * 6.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white, c.translated (-radius * 0.3f, -radius * 0.3f), inner, c.translated (radius, radius), true));
        g.fillEllipse (c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);
    }

protected:
    /** Dessine la scene ; t = secondes (monotone). */
    virtual void drawScene (juce::Graphics& g, juce::Rectangle<float> area, float t) = 0;

private:
    struct Star { float x, y, size, phase, speed; juce::Colour colour; };

    void rebuildBackground()
    {
        const int w = juce::jmax (1, getWidth()), h = juce::jmax (1, getHeight());
        background = juce::Image (juce::Image::ARGB, w, h, true);
        juce::Graphics g (background);
        g.fillAll (humpire::theme::bg);
        auto nebula = [&] (float cx, float cy, float rad, juce::Colour col, float alpha)
        {
            juce::ColourGradient grad (col.withAlpha (alpha), cx * (float) w, cy * (float) h,
                                       col.withAlpha (0.0f), cx * (float) w + rad * (float) w, cy * (float) h, true);
            g.setGradientFill (grad);
            g.fillRect (0, 0, w, h);
        };
        nebula (0.18f, 0.22f, 0.65f, humpire::theme::violet, 0.20f);
        nebula (0.85f, 0.80f, 0.55f, humpire::theme::ember, 0.10f);
        nebula (0.60f, 0.10f, 0.40f, humpire::theme::cyan, 0.06f);
    }

    void timerCallback() override { repaint(); }

    juce::Image background;
    std::vector<Star> stars;
};
